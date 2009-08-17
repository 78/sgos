// Warning: 此版本不支持多线程环境下操作同一个BXML数据
// utf8 version

#ifndef _BXML_
#define _BXML_


#define BXML_MAGIC 0x4C4D5842 //'BXML'
#define BXML_HEAD_SIZE 16

enum BXML_NODE_TYPE{
	BTYPE_ATTRIBUTE,
	BTYPE_NODE,
	BTYPE_TEXT
};

struct BXML_HEADER{
	unsigned int		bxml_magic;
	unsigned short		bxml_version;
	unsigned char		bxml_flag;
	unsigned char		reserved[9];
};

#define BXML_BUFF_TOO_SMALL 0x102

struct BXML_NODE{
	struct BXML_NODE*	node_attr;	//attributes
	struct BXML_NODE*	node_next;	//next sibling
	struct BXML_NODE*	node_pre;	//previous sibling
	struct BXML_NODE*	node_child; //children
	struct BXML_NODE*	node_parent;	//parent
	unsigned char		name_len;	//length of name, not including the '\0'
	unsigned int		value_len;	//length of value, not including the '\0'
	char*			name;
	char*			value;
};

typedef struct BXML_DATA{
	struct BXML_NODE*	node_root;		//Document Root
	struct BXML_NODE*	node_cur;		//Current Node
	unsigned short		version;		//XML Version
	unsigned char		flag;
	unsigned char		error;
//	mutex_t			mutex;			//wo don't need it in kernel mode.
}BXMLData;

//XML
struct BXML_DATA*		bxml_load( const char* filename );	//load xml data from a file
struct BXML_DATA*		bxml_parse( const char* mem );	//load xml data from memory buffer
int 				bxml_savexml( struct BXML_DATA* bxml, const char* filename );	//save xml data to a file
int 				bxml_buildxml( struct BXML_DATA* bxml, char* mem, int mem_size );	//build xml into memory
int 				bxml_redirect( struct BXML_DATA* bxml, const char* path, int create_if_not_exist );	//change the current path
char* 				bxml_readstr( struct BXML_DATA* bxml, const char* path );	//read a string
void 				bxml_writestr( struct BXML_DATA* bxml, const char* path, const char* str );	//write a stringcurrent path
int 				bxml_movenext( struct BXML_DATA* bxml );	//读取下一个项目
//BXML
//加载接口
int 				bxml_save( struct BXML_DATA* bxml, const char* filename );	//save bxml data to a file
int 				bxml_build( struct BXML_DATA* bxml, char* mem, int mem_size );	//build xml into memory
void 				bxml_free( struct BXML_DATA* bxml );	//free bxml data
int 				bxml_read( struct BXML_DATA* bxml, const char* path, void* buf, unsigned int buf_size );	//read 
void 				bxml_write( struct BXML_DATA* bxml, const char* path, const void* buf, unsigned int buf_size );	//write 
//操作接口
//
const char*			bxml_lasterr();

#endif	//_BXML_PARSER_

