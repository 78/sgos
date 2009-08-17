/*
 *  userspace.c
 *
 *  Memory allocation management (The second version)
 *
 *  Huang Guan  2009-8-16 Created.
 *  gdxxhg@gmail.com
 *
 *  Description: 这个文件写得很烂，是从allocator改过来的。因为用户空间大块内存分配的比较少，频繁的小块分配
 *  是在用户态进行的，所以这里不会太注重效率。
 *
 */

#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <string.h>
#include <mutex.h>
#include <bigblock.h>

#define HASH_APPEND( i, l ) { \
	i->hash_next = l;	i->hash_pre = NULL;	if(l) l->hash_pre = i; l=i;	}
#define HASH_DELETE( i, l ) { \
	if( i->hash_pre ){ i->hash_pre->hash_next = i->hash_next; }else{ l = i->hash_next; }	\
	if( i->hash_next ){ i->hash_next->hash_pre = i->hash_pre; } \
	}

#define IS_FREE_NODE(nod) (nod->attribute&FREE_NODE )
#define MAKE_FREE(nod) (nod->attribute |= FREE_NODE)
#define MAKE_OCCUPIED(nod) (nod->attribute &= ~FREE_NODE )

/* 
free_table[0]	4096字节空闲块
free_table[1]	8192字节空闲块
free_table[2]	16K字节空闲块
free_table[3]	32K字节空闲块
free_table[4]	64K字节空闲块
free_table[5]	128K字节空闲块
free_table[6]	256K字节空闲块
free_table[7]	512K字节空闲块
free_table[8]	1M字节空闲块
free_table[9]	2M字节空闲块
free_table[10]	4M字节空闲块
free_table[11]	8M字节空闲块
free_table[12]	16M字节空闲块
free_table[13]	32M字节空闲块
free_table[14]	64M字节空闲块
free_table[15]	128M字节空闲块
*/

//从大小得到散列表索引号
static int calc_hash_index(size_t k)
{
	int i;
	k = k>>PAGE_SIZE_BITS;
	i = 0;
	while( k>1 ){
		k = k>>1;
		i ++;
	}
	if( i>=MAX_HASH_ENTRY )
		i = MAX_HASH_ENTRY - 1;
	return i;
}

//基本分配函数
//时间复杂度分析：最坏情况W(n)  一般情况O(1)
//从allocator改过来，已经改得不像样了。。。
void*	bb_alloc(bigblock_t* who, size_t siz)
{
	size_t k, i, j;
	bnode_t* nod;
	uint eflags;
	if(!siz) return NULL;
	i = calc_hash_index( siz );
	//进入临界区
	local_irq_save( eflags );
	//在空闲块散列表中搜索合适的块
	for( j=i; j<MAX_HASH_ENTRY; j++ ){
		nod = who->free_table[j];
		//patched by Huang Guan. added "&& nod->size!=siz"
		while( nod && nod->size < siz )
			nod = nod->hash_next;
		if( nod ){	//找到可用块
			size_t rest = nod->size - siz;	//rest大小不包括分配描述符大小
			//从空闲散列表中删除
			HASH_DELETE( nod, who->free_table[j] );
			MAKE_OCCUPIED(nod);	//占用
			if( rest>=PAGE_SIZE ){ //如果有余下空间，则添加到空闲散列表中。rest == 0 为理想状态
				bnode_t* nod2 = (bnode_t*)kmalloc( sizeof(bnode_t) );
				nod2->size = rest;
				nod2->addr = nod->addr + siz;
				nod->size = siz;
				//调整邻接链表
				nod2->next = nod->next;
				if( nod2->next )
					nod2->next->pre = nod2;
				nod->next = nod2;
				nod2->pre = nod;
				//调整散列表
				k = calc_hash_index( rest );
				HASH_APPEND( nod2, who->free_table[k] );
				MAKE_FREE( nod2 );
			}
			//离开临界区
			local_irq_restore(eflags);
			return (void*)(nod->addr);
		}
		//没有可用块
	}
	//离开临界区
	local_irq_restore(eflags);
	return NULL;
}

//指定分配地址分配，时间复杂度O(n)，希望不要存放太多块了，不过这个函数估计只在开始
//时候使用一两次，这是完全没问题的 ！！
void*	bb_alloc_ex(bigblock_t* who, size_t addr, size_t siz )
{
	size_t k, j;
	bnode_t* nod;
	uint eflags;
	if(!siz) return NULL;
	//进入临界区
	local_irq_save( eflags );
	//在空闲块散列表中搜索合适的块
	for( nod = who->first_node; nod; nod = nod->next ){
		if( IS_FREE_NODE(nod) && nod->addr <= addr && nod->addr+nod->size >= addr+siz ){
			//good luck!!	//找到可用块，必须剪出来
			//一般剪中间的一块占用，两边的是可用空闲
			size_t left, right;
			bnode_t* nl, *nr, *opre, *onext;
			//从空闲散列表中移除
			j = calc_hash_index( nod->size );
			HASH_DELETE( nod, who->free_table[j] );
			//记下原来的左右节点
			opre = nod->pre;
			onext = nod->next;
			//开始设置新的节点
			right = (nod->addr+nod->size) - (addr+siz);
			left = addr - nod->addr;//左边剩余大小
			nod->size = siz;
			if( left>=PAGE_SIZE ){ //左边有空闲足够添加到散列表中 
				nl = (bnode_t*)kmalloc(sizeof(bnode_t));
				nl->size = left; //新大小
				nl->addr = nod->addr;
				nl->next = nod; //右节点变了
				nl->pre = opre;
				if( opre )
					opre->next = nl;
				else
					who->first_node = nl;
				nod->pre = nl; //调整中间的节点
				nod->addr = addr;
				//调整散列表
				k = calc_hash_index( left );
				HASH_APPEND( nl, who->free_table[k] );
				MAKE_FREE(nl);
				//不用MAKEFREE了，本来就是free的
			}else{	//如果不足够，则加入到中间
				nod->size += left;
			}
			if( right>=PAGE_SIZE ){ //右边有空闲足够添加到散列表中 
				nr = (bnode_t*)kmalloc(sizeof(bnode_t));
				nr->size = right;
				nr->addr = addr + siz;
				nr->pre = nod;
				nr->next = onext;
				if( onext )
					onext->pre = nr;
				nod->next = nr;
				//调整散列表
				k = calc_hash_index( right );
				HASH_APPEND( nr, who->free_table[k] );
				MAKE_FREE(nr);
			}else{
				nod->next = onext;
				nod->size += right;
			}
			MAKE_OCCUPIED(nod);
			//离开临界区
			local_irq_restore(eflags);
			return (void*)(nod->addr);
		}
	}
	//没有合适块
	//离开临界区
	local_irq_restore(eflags);
	PERROR("##failed to allocate memory at 0x%x(0x%x).", addr, siz );
	return NULL;
}

//检查地址为addr的空间是否已被分配。
int	bb_check_allocated(bigblock_t* who, size_t addr )
{
	bnode_t* nod;
	uint eflags;
	uint nod_addr;
	local_irq_save( eflags );
	nod = who->first_node;
	while( nod ){
		//
		if( nod->addr <= addr && nod->addr+nod->size > addr  ){
			local_irq_restore(eflags);
			if( IS_FREE_NODE(nod) ){
				return 0;
			}else{
				return 1;
			}
		}
		nod = nod->next;
	}
	local_irq_restore(eflags);
	return 0;
}

//合并a和b，c为a、b中不是当前释放的一个。
static bnode_t*	merge( bigblock_t* who, bnode_t* a, bnode_t* b, bnode_t* c )
{
	int k;
	//调整散列表
	k = calc_hash_index( c->size );
	HASH_DELETE(c, who->free_table[k]);
	//调整前一块大小
	a->size += b->size;
	//调整邻接链表
	a->next = b->next;
	if(a->next)
		a->next->pre = a;
	kfree(b);
	return a;
}

//基本释放函数。
//时间复杂度分析：O(n)
size_t	bb_free(bigblock_t* who, void* p)
{
	int k;
	bnode_t* nod;
	uint eflags;
	size_t siz;
	if( !p ) return;
	for( nod=who->first_node; nod; nod=nod->next ){
		if(nod->addr == (size_t)p )
			break;
	}
	if( !nod ){
		PERROR("trying to free free memory at 0x%X", p);
	}
	//进入临界区
	local_irq_save(eflags);
	if( IS_FREE_NODE(nod) ){
		PERROR("## cannot free free node.\n");
		local_irq_restore(eflags);
		return 0;
	}
	MAKE_FREE(nod);
	siz = nod->size;
	if( nod->pre && IS_FREE_NODE(nod->pre) ){
		//和前面一块空闲块合并，
		nod = merge(who, nod->pre, nod, nod->pre);
	}
	if( nod->next && IS_FREE_NODE(nod->next) ){
		//和后面一块合并。调整大小
		nod = merge(who, nod, nod->next, nod->next);
	}
	k = calc_hash_index( nod->size );
	HASH_APPEND(nod, who->free_table[k]);
	//离开临界区
	local_irq_restore(eflags);
	return siz;
}

void*	bb_calloc(bigblock_t* who, size_t c, size_t n)
{
	return bb_alloc( who, c*n );
}

void* 	bb_realloc(bigblock_t* who, void* p, size_t siz)
{
	bb_free(who, p);
	return bb_alloc(who, siz);
}

//获取可用内存空间
void	bb_init_block(bigblock_t* who, size_t addr, size_t size)
{
	//make it clear
	memset( (void*)who, 0, sizeof(bigblock_t) );
	//make a node
	bnode_t* nod = (bnode_t*)kmalloc(sizeof(bnode_t));
	nod->pre = nod->next = NULL;
	nod->size = size;
	nod->addr = addr;
	//attach it to free table
	HASH_APPEND( nod, who->free_table[MAX_HASH_ENTRY-1] );
	MAKE_FREE( nod );
	who->first_node = nod;
}


/***************** 以下为测试用代码 ******************/
void	bb_print_block(bigblock_t* who)
{
	int i;
	size_t used_size, free_size;
	bnode_t * nod;
	uint eflags;
	kprintf("Search the link table:\n");
	nod = who->first_node;
	used_size=free_size=0;
	local_irq_save(eflags);
	while( nod ){
		if( IS_FREE_NODE(nod) ){
			kprintf("[Free:%u]", nod->size );
			free_size += nod->size;
		}else{
			kprintf("{Used:%u}", nod->size );
			used_size += nod->size;
		}
		nod = nod->next;
	}
	kprintf("\nTotal used size: %u bytes, remaing %u bytes，total is %u bytes\n", used_size, free_size, used_size+free_size);
	kprintf("Search the free table:\n");
	for(i=0;i<MAX_HASH_ENTRY;i++){
		kprintf("%u bytes\t", 1<<(i+5) );
		nod = who->free_table[i];
		while( nod ){
			kprintf("[%u]", nod->size );
			nod = nod->hash_next;
		}
		kprintf("\n");
	}
	local_irq_restore(eflags);
}

void	bb_free_all(bigblock_t * who)
{
	//
	PERROR("not implemented.");
}
