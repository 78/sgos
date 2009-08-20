/*
 *  alloc.c
 *
 *  Memory allocation management
 *
 *  Huang Guan  2009-6-18 Created.
 *  gdxxhg@gmail.com
 *
 *  Description: This file mainly includes the functions about malloc, realloc, calloc, free 
 *  Warning: The code cannot be run in mult-threading environment currently!!
 *
 */

#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <string.h>
#include <mutex.h>
#include <allocator.h>
#include <process.h>

#define HASH_APPEND( i, l ) { \
	i->hash_next = l;	i->hash_pre = NULL;	if(l) l->hash_pre = i; l=i;	}
#define HASH_DELETE( i, l ) { \
	if( i->hash_pre ){ i->hash_pre->hash_next = i->hash_next; }else{ l = i->hash_next; }	\
	if( i->hash_next ){ i->hash_next->hash_pre = i->hash_pre; } \
	}

/*  sizeof(node_t)=20  最小分配单元为12字节
free_table[0]	32<=m<64
free_table[1]	64<=m<s128
free_table[2]	128字节空闲块
free_table[3]	256字节空闲块
free_table[4]	512字节空闲块
free_table[5]	1024字节空闲块
free_table[6]	2048字节空闲块
free_table[7]	4096字节空闲块
free_table[8]	8192字节空闲块
free_table[9]	超大空闲块
*/
//标识当前是否运行在mm_alloc中
static int in_allocator = 0;

//从大小得到散列表索引号
static int calc_hash_index(size_t k)
{
	int i;
	k = k>>5;
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
void*	mm_alloc(allocator_t* who, size_t siz)
{
	size_t m, k, i, j;
	node_t* nod;
	uint eflags;
	if(!siz) return NULL;
	m = siz + sizeof(node_t);	//m是包括节点大小计算的。
	i = calc_hash_index( m );
	//进入临界区
	local_irq_save( eflags );
	in_allocator = 1;	//告诉内核，进入了分配器
	//在空闲块散列表中搜索合适的块
	for( j=i; j<MAX_HASH_ENTRY; j++ ){
		nod = who->free_table[j];
		//patched by Huang Guan. added "&& nod->size!=siz"
		while( nod && nod->size < siz )
			nod = nod->hash_next;
		if( nod ){	//找到可用块
			int rest = nod->size - siz;	//rest大小不包括分配描述符大小
			//从空闲散列表中删除
			HASH_DELETE( nod, who->free_table[j] );
			MAKE_OCCUPIED(nod);	//占用
			if( rest>=32 ){ //如果有余下空间，则添加到空闲散列表中。rest == 0 为理想状态
				node_t* nod2 = (node_t*)((size_t)nod + m);
				nod2->size = rest - sizeof(node_t);
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
			//离开分配器
			in_allocator = 0;
			//离开临界区
			local_irq_restore(eflags);
			return (void*)((size_t)nod + sizeof(node_t));
		}
		//没有可用块
	}
	//离开分配器
	in_allocator = 0;	
	//离开临界区
	local_irq_restore(eflags);
	return NULL;
}


//检查地址为addr的空间是否已被分配。
int	mm_check_allocated(allocator_t* who, size_t addr )
{
	node_t* nod;
	uint eflags;
	local_irq_save( eflags );
	//这是例外情况。
	if( in_allocator ){
		local_irq_restore(eflags);
		return 1;
	}
	nod = who->first_node;
	while( nod ){
		//
		if( (uint)nod <= addr && (uint)nod+sizeof(node_t)+nod->size > addr  ){
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
static node_t*	merge( allocator_t* who, node_t* a, node_t* b, node_t* c )
{
	int k;
	//调整散列表
	k = calc_hash_index( c->size+sizeof(node_t) );
	HASH_DELETE(c, who->free_table[k]);
	//调整前一块大小
	a->size += b->size + sizeof(node_t);
	//调整邻接链表
	a->next = b->next;
	if(a->next)
		a->next->pre = a;
	return a;
}

//基本释放函数。
//时间复杂度分析：最坏情况W(1)  一般O(1)
size_t	mm_free(allocator_t* who, void* p)
{
	int k;
	node_t* nod;
	uint eflags;
	size_t siz;
	if( !p ) return 0;
	nod = (node_t*)((size_t)p - sizeof(node_t));
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
	k = calc_hash_index( nod->size+sizeof(node_t) );
	HASH_APPEND(nod, who->free_table[k]);
	//离开临界区
	local_irq_restore(eflags);
	return siz;
}

void*	mm_calloc(allocator_t* who, size_t c, size_t n)
{
	return mm_alloc( who, c*n );
}

void* 	mm_realloc(allocator_t* who, void* p, size_t siz)
{
	mm_free(who, p);
	return mm_alloc(who, siz);
}

//获取可用内存空间
void	mm_init_block(allocator_t* who, size_t addr, size_t size)
{
	//make it clear
	memset( (void*)who, 0, sizeof(allocator_t) );
	//make a node
	node_t* nod = (node_t*)addr;
	nod->pre = nod->next = NULL;
	nod->size = size - sizeof(node_t);
	//attach it to free table
	HASH_APPEND( nod, who->free_table[MAX_HASH_ENTRY-1] );
	MAKE_FREE( nod );
	who->first_node = nod;
}


/***************** 以下为测试用代码 ******************/
void	mm_print_block(allocator_t* who)
{
	int i;
	size_t used_size, free_size;
	node_t * nod;
	uint eflags;
	kprintf("Search the link table:\n");
	nod = who->first_node;
	used_size=free_size=0;
	local_irq_save(eflags);
	while( nod ){
		if( IS_FREE_NODE(nod) ){
		//	kprintf("[Free:%u]", nod->size+sizeof(node_t) );
			free_size += nod->size+sizeof(node_t);
		}else{
		//	kprintf("{Used:%u}", nod->size+sizeof(node_t) );
				used_size += nod->size+sizeof(node_t);
		}
		nod = nod->next;
	}
	kprintf("\nTotal used size: %u bytes, remaing %u bytes，total is %u bytes\n", used_size, free_size, used_size+free_size);
	kprintf("Search the free table:\n");
	for(i=0;i<MAX_HASH_ENTRY;i++){
		kprintf("%u bytes\t", 1<<(i+5) );
		nod = who->free_table[i];
		while( nod ){
			kprintf("[%u]", nod->size+sizeof(node_t) );
			nod = nod->hash_next;
		}
		kprintf("\n");
	}
	local_irq_restore(eflags);
}

void	mm_free_all(allocator_t * who)
{
	//
	PERROR("not implemented.");
}
