/*
 *  alloc.c
 *
 *  Memory allocation management
 *
 *  Huang Guan  2009-6-18 Created.
 *  gdxxhg@gmail.com
 *
 *  Description: This file mainly includes the functions about malloc, realloc, calloc, free 
 *  Warning: The code cannot be run in mult-threading environment!!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <api.h>

#define FREE_NODE 0x00000001

typedef struct node{
	//邻接链表
	struct node* pre;
	struct node* next;
	//Hash table
	struct node* hash_pre;
	struct node* hash_next;
	//分配描述
	size_t attribute;
	size_t size; //不包括分配描述符大小
}node_t;

#define IS_FREE_NODE(nod) (nod->attribute&FREE_NODE )
#define MAKE_FREE(nod) (nod->attribute |= FREE_NODE)
#define MAKE_OCCUPIED(nod) (nod->attribute &= ~FREE_NODE )

/* 
 * 计算最大块链表 max = 2^(5+MAX_HASH_ENTRY)
 */
#define MAX_HASH_ENTRY 19 // max = 16MB
#define HASH_APPEND( i, l ) { \
	i->hash_next = l;	i->hash_pre = NULL;	if(l) l->hash_pre = i; l=i;	}
#define HASH_DELETE( i, l ) { \
	if( i->hash_pre ){ i->hash_pre->hash_next = i->hash_next; }else{ l = i->hash_next; }	\
	if( i->hash_next ){ i->hash_next->hash_pre = i->hash_pre; } \
	}

static node_t* free_table[MAX_HASH_ENTRY] = {NULL,};
static node_t* first_node = NULL;	//used for debugging.
static int	mutex;
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
...
free_table[MAX_HASH_ENTRY-1]	超大空闲块
*/

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

// 模拟实现lock和nlock功能
static void	lock()
{
	while(--mutex){
		mutex++;
		//让出cpu
		SysSleepThread(1);
	}
}

static void	unlock()
{
	mutex++;
}

void	__allocation_init();
//基本分配函数
//接近常数的时间复杂度
void*	malloc(size_t siz)
{
	size_t m, k, i, j;
	node_t* nod;
	if(!siz) return NULL;
	if(!first_node) __allocation_init();
	if(siz<8) siz=8;
	m = siz + sizeof(node_t);
	i = calc_hash_index( m );
	//进入临界区
	lock();
	//在空闲块散列表中搜索合适的块
	for( j=i; j<MAX_HASH_ENTRY; j++ ){
		nod = free_table[j];
		//patched by Huang Guan. added "&& nod->size!=siz"
		while( nod && nod->size<siz )
			nod = nod->hash_next;
		if( nod ){	//找到可用块
			size_t rest = nod->size + sizeof(node_t) - m;	//rest大小不包括分配描述符大小
			//从空闲散列表中删除
			HASH_DELETE( nod, free_table[j] );
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
				HASH_APPEND( nod2, free_table[k] );
				MAKE_FREE( nod2 );
			}
			//离开临界区
			unlock();
			return (void*)((size_t)nod + sizeof(node_t));
		}
		//没有可用块
	}
	//离开临界区
	unlock();
	return NULL;
}

//合并a和b，c为a、b中不是当前释放的一个。
static node_t*	merge( node_t* a, node_t* b, node_t* c )
{
	int k;
	//调整散列表
	k = calc_hash_index( c->size+sizeof(node_t) );
	HASH_DELETE(c, free_table[k]);
	//调整前一块大小
	a->size += b->size + sizeof(node_t);
	//调整邻接链表
	a->next = b->next;
	if(a->next)
		a->next->pre = a;
	return a;
}

//基本释放函数。
//常数的时间复杂度
void	free(void* p)
{
	int k;
	node_t* nod;
	if( !p ) return;
	nod = (node_t*)((size_t)p - sizeof(node_t));
	if( IS_FREE_NODE(nod) ){
		printf("Error: cannot free free node.\n");
		return;
	}
	MAKE_FREE(nod);
	//进入临界区
	lock();
	if( nod->pre && IS_FREE_NODE(nod->pre) ){
		//和前面一块空闲块合并，
		nod = merge(nod->pre, nod, nod->pre);
	}
	if( nod->next && IS_FREE_NODE(nod->next) ){
		//和后面一块合并。调整大小
		nod = merge(nod, nod->next, nod->next);
	}
	k = calc_hash_index( nod->size+sizeof(node_t) );
	HASH_APPEND(nod, free_table[k]);
	//离开临界区
	unlock();
}

void*	calloc(size_t c, size_t n)
{
	return malloc( c*n );
}

// 未真正实现realloc
void* 	realloc(void* p, size_t siz)
{
	free(p);
	return malloc(siz);
}

//获取可用内存空间
void __allocation_init()
{
	size_t size = 256*1024*1024;
	memset( free_table, 0, sizeof(free_table ) );
	void* p = SysAllocateMemory( SysGetCurrentSpaceId(), size, MEMORY_ATTR_WRITE, ALLOC_LAZY );
	if( p == NULL ){
		printf("Memory is not enough.\n");
		//should do something here....
		SysExitSpace(-ERR_NOMEM);
	}
	//make a node
	node_t* nod = (node_t*)p;
	nod->pre = nod->next = NULL;
	nod->size = size - sizeof(node_t);
	//attach it to free table
	HASH_APPEND( nod, free_table[MAX_HASH_ENTRY-1] );
	MAKE_FREE( nod );
	first_node = nod;
	//init mutex
	mutex = 1;
}

/***************** 以下为测试用代码 ******************/
void debug_print()
{
	int i;
	size_t used_size, free_size;
	node_t * nod;
	printf("遍历邻接链表:\n");
	nod = first_node;
	used_size=free_size=0;
	while( nod ){
		if( IS_FREE_NODE(nod) ){
			printf("[空闲:%u]", nod->size+sizeof(node_t) );
			free_size += nod->size+sizeof(node_t);
		}else{
			printf("{占用:%u}", nod->size+sizeof(node_t) );
			used_size += nod->size+sizeof(node_t);
		}
		nod = nod->next;
	}
	printf("\n一共占用%u字节，剩余%u字节，内存总量%u字节\n", used_size, free_size, used_size+free_size);
	printf("遍历空闲散列表:\n");
	for(i=0;i<MAX_HASH_ENTRY;i++){
		printf("%u字节\t", 1<<(i+5) );
		nod = free_table[i];
		while( nod ){
			printf("[%u]", nod->size+sizeof(node_t) );
			nod = nod->hash_next;
		}
		printf("\n");
	}
}
