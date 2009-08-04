#include <stdio.h>
#include <stdlib.h>
#include "fat32.h"
#include "unicode.h"

#define TRUE 1
#define FALSE 0

typedef int(*fat32_enumerator) ( char* fname, int size, TIME* ctime, TIME* mtime, 
	int attr, int cluster, FILE_DESC* fNew, char* comp_name );

static t_32 fat32_getclu( FILE_DESC* f );
static t_32 fat32_nextclu( FILE_DESC* f, t_32 clu );
static int fat32_get_clu_by_order( FILE_DESC* f, int clu, int no );
int fat32_open( FILE_DESC* f, char *name );

static FAT32DEV fatdev, *fat32; 
FILE_DESC rootfile, *root;

int fat32_init( t_16 dev )
{
	FAT32_BOOTSEC* boot;
	void* buf;
	fat32 = &fatdev;
	root = &rootfile;
	memset( fat32, 0, sizeof(FAT32DEV) );

	buf = buffer_read( dev, 0 );	//读取第一块 1KB
	boot = (FAT32_BOOTSEC*)buf;
	if( boot->EBR != 0x28 && boot->EBR !=0x29 )
	{
		//printf("Not fat32 system\n");
		buffer_release( buf );
		return -1;
	}
	printf("fat32 System OEM: %s\n", boot-> OEM );
	fat32->fats = boot->Fats;//
	fat32->secPerClu = boot->SecPerClu;//
	fat32->secPerFat = boot->SecPerFat;	//每fat扇区数
	fat32->bytPerSec = boot->BytPerSec;	//每扇区字节数 512
	fat32->secReserved = boot->SecReserved;	//保留扇区数
	fat32->rootClu = boot->RootCluster;	//根目录簇号
	
	buffer_release( buf );
	//caculate some information
	fat32->fatAddr = fat32->bytPerSec * fat32->secReserved;	//fat基址
	//printf("fat addr: 0x%X\n", fat32->FatAddr);
	fat32->dataAddr = fat32->fatAddr + fat32->fats * fat32->secPerFat *fat32->bytPerSec;	//数据区基址
	fat32->bytPerClu = fat32->bytPerSec * fat32->secPerClu;
	printf("fat32 RootClu: 0x%X  data_addr: 0x%X  bytPerClu:%x  fatAddr:%x\n",
        fat32->rootClu, fat32->dataAddr, fat32->bytPerClu, fat32->fatAddr );
	//设置根目录
	memset( root, 0, sizeof(FILE_DESC) );
	root->dev = dev;
	root->data = fat32->rootClu;	//根目录的簇
	root->attribute = FILE_ATTR_ISDIR;
	root->dev_info = fat32;
	return 0;
}


//时间转换 移植自好友孙亮的ulios
/*取得目录项时间,op:0取得创建时间1取得修改时间,出错1成功0*/
static int fat32_gettime(FAT32_ENTRY* ff, TIME *tm, int op)
{
    if(op==0)
	{	tm->year=(ff->cdate>>9)+1980;
		tm->mon=(ff->cdate>>5)&0x0f;
		tm->mday=ff->cdate&0x1f;
		tm->hour=ff->ctime>>11;
		tm->min=(ff->ctime>>5)&0x1f;
		tm->sec=(ff->ctime<<1)&0x1f;
	}
	else
	{	tm->year=(ff->mdate>>9)+1980;
		tm->mon=(ff->mdate>>5)&0x0f;
		tm->mday=ff->mdate&0x1f;
		tm->hour=ff->mtime>>11;
		tm->min=(ff->mtime>>5)&0x1f;
		tm->sec=(ff->mtime<<1)&0x1f;
	}
	if(tm->mon==0 || tm->mon>12) return -1;
	if(tm->mday==0 || tm->mday>31) return -1;
	if(tm->hour>23 || tm->min>59 || tm->sec>59) return -1;
	return 0;
}


/*设置目录项时间,op:0设置创建时间1设置修改时间2都设置,出错1成功0*/
static int fat32_settime(FAT32_ENTRY* ff, TIME *tm, int op)
{
    if(tm->year<1980) return 1;
	if(tm->mon==0 || tm->mon>12) return 1;
	if(tm->mday==0 || tm->mday>31) return 1;
	if(tm->hour>23 || tm->min>59 || tm->sec>59) return 1;
	if(op==0)
	{
	    ff->cdate=((tm->year-1980)<<9)+(tm->mon<<5)+tm->mday;	/*时间格式与FAT32相同*/
		ff->ctime=(tm->hour<<11)+(tm->min<<5)+(tm->sec>>1);
	}
	else
	{
	    ff->mdate=((tm->year-1980)<<9)+(tm->mon<<5)+tm->mday;
		ff->mtime=(tm->hour<<11)+(tm->min<<5)+(tm->sec>>1);
		if(op==2)
		{
		    ff->cdate=ff->mdate;
			ff->ctime=ff->mtime;
		}
	}
	return 0;
}

//释放簇，后续簇也将被释放
static int fat32_freeclu( FILE_DESC* f, t_32 clu )
{
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
	t_32 block, pos, lastone;
	t_32 read_addr;
	t_32* next;
	void* buf = NULL;
NEXT:
	read_addr = fat->fatAddr;
	read_addr += clu*4;
	block = read_addr/BLOCK_SIZE;	//读哪个块
	pos = read_addr%BLOCK_SIZE;	//偏移
	if( pos > 1020 )
	{
		printf("warning: Boundary Out!");
		return -1;
	}
	if( buf==NULL || lastone!=block ){	//|| buf->block!=block
	    if( buf )
            buffer_release(buf);
        buf = buffer_read( f->dev, block );
		lastone = block;
	}
	if(buf==NULL)
	{
		printf("Read Error.");
		return -1;
	}
	//ok get the next
	next = (t_32*)((unsigned char*)buf+pos);
	if( IS_FAT32CLU_INUSE( *next ) )
	{
	    clu = *next;
	    *next = 0;  //free it
	    goto NEXT;
	}
	buffer_release(buf);
	return 0;
}


//获取新簇
static t_32 fat32_allocclu(FILE_DESC* f)
{
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
	t_32 tmp = f->data;
	t_32 clu;
	do{
		clu = tmp;
		tmp=fat32_nextclu(f,tmp);
	}while( IS_FAT32CLU_INUSE(tmp) );

	t_32 read_addr = fat->fatAddr;
	read_addr+= clu*4;
	t_32 block, pos;
	block = read_addr/BLOCK_SIZE;	//读哪个块
	pos = read_addr%BLOCK_SIZE;	//偏移
	if( pos > BLOCK_SIZE-4 )
	{
		printf("warning: Boundary Out!\n");
		return 0;
	}
	void* buf = buffer_read( f->dev, block );
	if(buf==NULL)
	{
		printf("Read Error.");
		return 0;
	}
	t_32* next;
	next = (t_32*)((unsigned char*)buf+pos);
	if( IS_FAT32CLU_END( *next ) )
	{
	    *next = fat32_getclu(f);
	}
//	buf->dirty = 1;
	buffer_write( f->dev, block, buf );
	buffer_release(buf);
	return *next;
}

static t_32 fat32_getclu( FILE_DESC* f )
{
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
    void* buf;
	t_32 read_addr = fat->fatAddr;
	read_addr += fat->newCluStart;
	t_32 block, pos, count = fat->secPerFat * fat->bytPerSec;
	while( count>0 ){
        block = read_addr/BLOCK_SIZE;	//读哪个块
        pos = read_addr%BLOCK_SIZE;	//偏移
        if( pos > 1020 )
        {
            printf("FatGetNextClu warning: Boundary Out!\n");
        }
        buf = buffer_read( f->dev, block );
        if(buf==NULL)
        {
            printf("Read Error.\n");
            return 0;
        }
        t_32* lookup, i, lookup_no = (BLOCK_SIZE-pos)>>2;
        lookup = (t_32*)((unsigned char*)buf+pos);
        for( i=0; i<lookup_no; i++, read_addr+=4, count-=4 )
        {
            if( IS_FAT32CLU_FREE(lookup[i]) )
            {
                int clu = ( read_addr - fat->fatAddr ) >> 2;
                lookup[i] = 0xffffffff;
 //               buf->dirty = 1;
				buffer_write( f->dev, block, buf );
                buffer_release(buf);
                //printf("New cluster: %d  %x addr:%x", clu, fat->fatAddr, read_addr);
                return clu;
            }
        }
        buffer_release(buf);
	}
	printf("No enough cluster for allocating.");
	return 0; //error clu
}

//获得短文件名
static int fat32_get_shortname( char* name, FAT32_ENTRY* ff )
{
	int i, j=0;
	for( i=0; i<8 && ff->name[i] && ff->name[i]!=' '; i++ )
		name[j++]=ff->name[i];
	if( !FAT32_IS_SUBDIR(ff->attribute) ){
		name[j++]='.';
		for( i=0; i<3 && ff->extention[i] && ff->extention[i]!=' '; i++ )
			name[j++]=ff->extention[i];
	}
	name[j]=0;
	if( j )
		return TRUE;
	return FALSE;
}

//构造短文件名
static int fat32_build_shortname( FILE_DESC* f, char* name, FAT32_ENTRY* ff )
{
	int i, j;
	char* ext;
    char trychar = '1';
    char longSignChar = '~';
TRY:
	ext = strrchr( name, '.' );
	memset( ff->name, ' ', 11 );    //用空格填充
	if( (int)(ext-name) > 8 )
	{
        for( i=0; i<6 && *name && name!=ext; i++, name++ )
            ff->name[i]=toupper(*name);
        ff->name[i++] = longSignChar;
        ff->name[i] = trychar;
	}else{
        for( i=0; i<8 && *name && name!=ext; i++, name++ )
            ff->name[i]=toupper(*name);
	}
	if( ext )
	{
	    ext++;
        for( i=0; i<3 && *ext; i++, ext++ )
            ff->name[8+i]=toupper(*ext);
	}
	//尝试这个文件名是否已用。
	{
	    char fname[16];
        FILE_DESC tmp;
        fat32_get_shortname( fname, ff );
        memcpy( &tmp, f, sizeof(FILE_DESC) );
        tmp.flag = 0;
        if( fat32_open( &tmp, fname )==0 )
        {
            if( trychar == '6' )
            {
                trychar = '1';
                longSignChar++;
                goto TRY;
            }else{
                trychar++;
                goto TRY;
            }
        }
	}
	//printf("fat32BuildShortFileName %s, %s\n", name, ff->name );
	return 0;
}


//读入下一个簇号
static t_32 fat32_nextclu( FILE_DESC* f, t_32 clu )
{
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;

	t_32 read_addr = fat->fatAddr;
	read_addr+= clu*4;
	t_32 block, pos;
	block = read_addr/BLOCK_SIZE;	//读哪个块
	pos = read_addr%BLOCK_SIZE;	//偏移
	if( pos > BLOCK_SIZE-4 )
	{
		printf("FatGetNextClu warning: Boundary Out!\n");
	}
	void* buf = buffer_read( f->dev, block );
	if(buf==NULL)
	{
		printf("Read Error.\n");
		return -1;
	}
	t_32 next;
	next = * (t_32*)( (t_32)buf + pos );
	buffer_release(buf);
	return next;
}

//写一个簇的数据
static int fat32_writeclu( FILE_DESC* f, t_32 clu, char* clu_data )
{
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
	char* data = clu_data;	//数据缓冲区
	t_32 block, pos, write=0, write_size;
	block = (fat->dataAddr / fat->bytPerSec + (clu - 2) * fat->secPerClu ) / 2;	//fat数据区偏移
	write_size = fat->bytPerClu;	//读的大小
	//下面这个干什么了？？？090804 HG :-(
	pos = ( (fat->dataAddr % BLOCK_SIZE) +
	  ( (( clu - 2 ) % BLOCK_SIZE ) * fat->bytPerClu ) % BLOCK_SIZE
		) % BLOCK_SIZE;	//偏移
	while( write_size>0 )
	{
		write = BLOCK_SIZE-pos;
		if( write_size<write ) write=write_size;
		void* buf = buffer_read( f->dev, block );
		if(buf==NULL)
		{
			printf("write clu Error.\n");
			return -1;
		}
		memcpy( (unsigned char*)buf+pos, data, write );
//		buf->dirty = 1;
		buffer_write( f->dev, block, buf );
		buffer_release( buf );
		data += write;
		write_size -= write;
		block++;
		pos = 0;
	}
	return 0;
}

//读入一个簇的数据进缓冲区
static char* fat32_readclu( FILE_DESC* f, t_32 clu )
{
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
	char* clu_data = (char*)malloc( fat->bytPerClu );
	char* data = clu_data;	//数据缓冲区
	t_32 block, pos, read=0, read_size;
	block = (fat->dataAddr/512 + (clu-2)*fat->secPerClu)/2;	//fat数据区偏移
	read_size = fat->bytPerClu;	//读的大小
	pos = ( (fat->dataAddr%BLOCK_SIZE) +
	  (((clu-2)%BLOCK_SIZE)*fat->bytPerClu)%BLOCK_SIZE
		)%BLOCK_SIZE;	//偏移
	//printf("block: %d  pos: %d\n", block, pos );
	while( read_size>0 )
	{
		read = BLOCK_SIZE - pos;
		if( read_size < read ) 
			read = read_size;
		void* buf = buffer_read( f->dev, block );
		if(buf==NULL)
		{
			printf("Read clu Error.\n");
			return NULL;
		}
		memcpy( data, (unsigned char*)buf+pos, read );
		buffer_release(buf);
		data += read;
		read_size -= read;
		block++;
		pos = 0;
	}
	return clu_data;
}

//组合长文件名 注意，宽字符
static int fat32_build_filename( char* fname, int name_len, char (*long_name)[26], int name_entries )
{
    int i;
    char tmp[FILENAME_LEN];
    fname[0]='\0';
    for( i=0; i<name_entries; i++ )
    {
        if( name_len<=(i+1)*26 ) {
            printf("Buffer is too small.");
            return -1;
        }
        memcpy( fname + i*26, long_name[i], 26 );
    }
    unicode_decode( (wchar_t*)fname, name_len, tmp, FILENAME_LEN );
    strncpy( fname, tmp, FILENAME_LEN );
    return 0;
}

//枚举一个目录下的所有目录项
static int fat32_enum_ent( FILE_DESC* f, fat32_enumerator enumerator, FILE_DESC* f_new, char* comp_name )
{
    #define MAX_LNAME_ENTRIS FILENAME_LEN/26
	FAT32_ENTRY* ff;
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
	//读入parent的簇，搜索
	t_32 clu = fat32_get_clu_by_order( f, f->data, f->pos / fat->bytPerClu );
	t_32 pos = f->pos % fat->bytPerClu;
	//printf("clu:%d\n", clu);
	char fname[FILENAME_LEN];
	char long_name[MAX_LNAME_ENTRIS][26];
	char* clu_data;
	int i, max_entries = fat->bytPerClu/sizeof(FAT32_ENTRY);
	int checksum, id, name_entries;   //长文件名校验码
	name_entries = 0;
read_clu:
	clu_data = fat32_readclu( f, clu );
	for( i=0, ff=(FAT32_ENTRY*)(clu_data+pos); i<max_entries; i++,ff++ )
	{
	    //检查第一个字符
	    if( ff->name[0]==0xE5 || ff->name[0]==0 )
            continue;
	    //判断是否是长文件名项
	    if( FAT32_IS_LNAME( ff->attribute ) )
	    {
	        FAT32_NAME_ENTRY* lne = (FAT32_NAME_ENTRY*)ff;
	        if( lne->id&0x40 ) //最后一个项
	        {
	            checksum = lne->checksum;
	            id = lne->id&0x1F;
	            name_entries = id;
                if( name_entries>MAX_LNAME_ENTRIS )
                {
                    printf("id is too big.");
                    name_entries = 0;
                    continue;
                }
	            id --;
	        }else{
	            if( lne->checksum!=checksum || lne->id != id ){
                    printf("checksum or id is invalid.");
                    continue;
	            }
                id--;
	        }
	        memcpy( long_name[id], lne->name1, 10 );
	        memcpy( long_name[id]+10, lne->name2, 12 );
	        memcpy( long_name[id]+22, lne->name3, 4 );
	    }else{
	        //非长文件名项
	        if( name_entries ) //使用长文件名
	        {
	            fat32_build_filename( fname, FILENAME_LEN, long_name, name_entries );
	        }else{
	            fat32_get_shortname( fname, ff );
	        }
            int cluster = (ff->cluster_high<<16) | ff->cluster;
            int size = ff->size, attr = 0;
            TIME ctime, mtime;
            int ret;
            fat32_gettime( ff, &ctime, 0 );
            fat32_gettime( ff, &mtime, 1 );
            //设置文件属性
            if( FAT32_IS_READONLY( ff->attribute ) )
                attr |= FILE_ATTR_RDONLY;
            if( FAT32_IS_SYSTEM( ff->attribute ) )
                attr |= FILE_ATTR_SYSTEM;
            if( FAT32_IS_SUBDIR( ff->attribute ) )
                attr |= FILE_ATTR_ISDIR;
            if( FAT32_IS_HIDDEN( ff->attribute ) )
                attr |= FILE_ATTR_HIDDEN;
            //( char* fname, int* size, TIME* ctime, TIME* mtime, int* attr, int* cluster )
            ret = enumerator( fname, size, &ctime, &mtime, attr, cluster, f_new, comp_name );
            switch( ret ){
                case 0:
                    //continue
                    break;
                case 1:
                    //return
                    free( clu_data );
                    return ret;
                case 2:
                    //delete it!!
                    for( id=-name_entries; id<=0; id++ )
                    {
                        ff[id].name[0] = 0xE5;
                    }
                    fat32_writeclu( f, clu, clu_data );
                    free ( clu_data );
                    return ret;
                case 3:
                    //save changes except name
                    ff->size = f_new->size;
                    fat32_settime( ff, &f_new->ctime, 0 );
                    fat32_settime( ff, &f_new->mtime, 1 );
                    if( f_new->attribute & FILE_ATTR_RDONLY )
                        ff->attribute |= RDONLY;
                    if( f_new->attribute & FILE_ATTR_SYSTEM )
                        ff->attribute |= SYSTEM;
                    if( f_new->attribute & FILE_ATTR_ISDIR )
                        ff->attribute |= SUBDIR;
                    if( f_new->attribute & FILE_ATTR_HIDDEN )
                        ff->attribute |= HIDDEN;
                    ff->cluster_high = f_new->data>>16;
                    ff->cluster = (t_16)(f_new->data&0xffff);
                    fat32_writeclu( f, clu, clu_data );
                    free( clu_data );
                    return ret;
                default:
                    printf("Unknown ret value.");
                    break;
            };
            //next file
            name_entries = 0;
	    }
	}
	pos = 0;
	free( clu_data );
	//在这个簇没有找到
	clu = fat32_nextclu(f, clu);
	if( IS_FAT32CLU_INUSE(clu) )
	{
		goto read_clu;
	}
	//printf("Couldnot find file %s\n", name );
	return -1;
}


static int remove_enumerator( char* fname, int size, TIME* ctime,
    TIME* mtime, int attr, int cluster, FILE_DESC* f_new, char* comp_name )
{
    //printf("Comparing %s & %s\n", fname, comp_name );
    if( stricmp( fname, comp_name )==0 )
    {
        return 2; //delete this file
    }
    return 0; //continue enumerating.
}

//remove an entry from the directory, but not clean the fat table
static int fat32_remove_ent( FILE_DESC* f )
{
	FILE_DESC* parent = f->parent;
	int ret;
	parent -> pos = 0; // find from first
    ret = fat32_enum_ent( parent, remove_enumerator, f, f->name );
    if( ret==2 )
    {
        return 0;
    }
    //printf("%s: file not found.\n", name );
	return -1;
}

//
static int fat32_create_ent( FILE_DESC* fsave )
{
    #define MAX_LNAME_ENTRIS FILENAME_LEN/26
	FAT32_ENTRY* ff;
	FILE_DESC* f = fsave->parent;
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
	//读入parent的簇，搜索
	t_32 clu = fat32_get_clu_by_order( f, f->data, f->pos / fat->bytPerClu );
	t_32 pos = f->pos % fat->bytPerClu;
	char uni_name[FILENAME_LEN];
	char* clu_data;
	int i, max_entries = fat->bytPerClu/sizeof(FAT32_ENTRY);
	int name_entries = 1, found;   //长文件名
	int uni_len = 0;
	if( strlen( fsave->name ) > 11 )    //使用了长文件名
	{
	    memset( uni_name, 0xff, FILENAME_LEN );
        uni_len = unicode_encode( (unsigned char*)fsave->name, FILENAME_LEN, (wchar_t*)uni_name, FILENAME_LEN );
        name_entries = uni_len/13+1+1;  //计算使用多少个项
	}
	clu_data = fat32_readclu( f, clu );
read_clu:
    found=0; //从新找，长文件名不希望跨越簇
	for( i=0, ff=(FAT32_ENTRY*)(clu_data+pos); i<max_entries; i++,ff++ )
	{
	    //检查第一个字符
	    if( ff->name[0]==0xE5 || ff->name[0]==0 )
        {
            found ++;
            if( found == name_entries ) //足够数目写下该目录项
            {
                //构造短文件名
                memset( ff, 0, sizeof( FAT32_ENTRY ) );
                fat32_build_shortname( fsave, fsave->name, ff );
                fat32_settime( ff, &fsave->mtime, 2 );	//set create time
                ff->size = fsave->size;
                if( fsave->attribute & FILE_ATTR_RDONLY )
                    ff->attribute |= RDONLY;
                if( fsave->attribute & FILE_ATTR_SYSTEM )
                    ff->attribute |= SYSTEM;
                if( fsave->attribute & FILE_ATTR_ISDIR )
                    ff->attribute |= SUBDIR;
                if( fsave->attribute & FILE_ATTR_HIDDEN )
                    ff->attribute |= HIDDEN;
                ff->cluster_high = fsave->data>>16;
                ff->cluster = (t_16)(fsave->data&0xffff);
                if( uni_len ) //长文件名
                {
                    unsigned char checksum = 0;
                    int j, k;
                    FAT32_NAME_ENTRY * lne = (FAT32_NAME_ENTRY*)ff;
                    for (i=0; i<11; i++)
                        checksum = ((checksum<<7) | (checksum>>1)) + ff->name[i];

                    //for( j=11, k=0; j>0; j--, k++ )
                    //{
                    //    checksum = ( (checksum & 1) ? 0x80: 0 ) + ( checksum >> 1 ) +
                    //        ff->name[k];
                    //}
                    //printf("checksum: %x", checksum );

                    for( j=-1, k=0; j>-name_entries; j--, k+=26 )
                    {
                        memset( &lne[j], 0, sizeof( FAT32_ENTRY ) );
                        lne[j].id = -j;
                        lne[j].attr = 0xf;
                        lne[j].checksum = checksum;
                        memcpy( lne[j].name1, uni_name+k, 10 );
                        memcpy( lne[j].name2, uni_name+k+10, 12 );
                        memcpy( lne[j].name3, uni_name+k+22, 4 );
                        if( j== +1-name_entries ) //最后一个
                        {
                            lne[j].id |= 0x40;
                        }
                    }
                }
                fat32_writeclu( f, clu, clu_data );
                free( clu_data );
                return 0;
            }
        }else{
            found = 0;
        }
	}
	pos = 0;
	free( clu_data );
	//在这个簇没有找到
	clu = fat32_nextclu( f, clu );
	if( IS_FAT32CLU_INUSE(clu) )
	{
        clu_data = fat32_readclu( f, clu );
		goto read_clu;
	}else if( IS_FAT32CLU_END( clu ) ) //增大文件
    {
        clu = fat32_allocclu( f );
        if( !IS_FAT32CLU_INUSE(clu) )
            return -1; //error
        clu_data = (char*)malloc( fat->bytPerClu );
        memset( clu_data, 0, fat->bytPerClu );
		goto read_clu;
    }
	//printf("Couldnot find file %s\n", name );
	return -1;
}

static int open_enumerator( char* fname, int size, TIME* ctime,
    TIME* mtime, int attr, int cluster, FILE_DESC* f_new, char* comp_name )
{
    //printf("Comparing %s & %s\n", fname, comp_name );
    if( stricmp( fname, comp_name )==0 )
    {
        f_new->pos = 0;
        f_new->size = size;
        f_new->ctime = *ctime;
        f_new->mtime = *mtime;
        f_new->data = cluster;
        f_new->attribute = attr;
        return 1; //stop enumerating.
    }
    return 0; //continue enumerating.
}

//读磁盘上的文件信息
int fat32_open( FILE_DESC* f, char *name )
{
	FAT32_ENTRY* ff;
	FILE_DESC* parent = f->parent;
	int ret;
	parent -> pos = 0; // find from first
    ret = fat32_enum_ent( parent, open_enumerator, f, name );
    if( ret>0 )
    {
        strncpy( f->name, name, FILENAME_LEN );
        return 0;
    }
    //找不到文件
    if( f->flag & FILE_FLAG_CREATE ) //创建
    {
        strncpy( f->name, name, FILENAME_LEN );
        f->size = 0;
        f->data = fat32_getclu( f );   //获取一个空簇
        ret = fat32_create_ent( f );
        if( ret==0 )
        {
            return 0; //create OK!
        }
    }
    //printf("%s: file not found.\n", name );
	return -1;
}

//获得第几个簇号
static int fat32_get_clu_by_order( FILE_DESC* f, int clu, int no )
{
    int i;
	for( i=0; i<no; i++ )
	{
		clu = fat32_nextclu( f, clu );
		if( ! IS_FAT32CLU_INUSE(clu) )
		{
			printf("Skip Clusters Error\n");
			return 0; //errror
		}
	}
	return clu;
}

//读文件数据
int fat32_read( FILE_DESC* f, char* buf, t_32 count )
{
	//printf("FatReadFile( %d, 0x%X, %d);\n", f-file, buf, count );
	if( count>f->size-f->pos )
		count = f->size-f->pos;
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
	t_32 *fpos = &(f->pos), clu = fat32_get_clu_by_order( f, f->data, *fpos / fat->bytPerClu );
    char* clu_data;
	t_32 pos = *fpos%fat->bytPerClu, read;
	t_32 read_no = 0;
	//一个簇一个簇地读，直到读完
	while( count>0 )
	{
		read = fat->bytPerClu-pos;
		if( count<read) read=count;
		if( ! IS_FAT32CLU_INUSE(clu) )
		{
			printf("EOF: read_no=%d\n", read_no);
			return 0;
		}
		clu_data = fat32_readclu( f, clu );
		memcpy( buf, clu_data+pos, read );
		free( clu_data );
		buf+=read;
		(*fpos)+=read;
		read_no+=read;
		pos=0;
		count-=read;
		if( count>0 )
			clu = fat32_nextclu( f, clu );
	}
	return read_no;
}

//Enumerator for readDir
static int fat32_readdir_enumerator( char* fname, int size, TIME* ctime,
    TIME* mtime, int attr, int cluster, FILE_DESC* f_new, char* other )
{
    DIRENTRY** buf = (DIRENTRY**)other;
    DIRENTRY* dir = *buf;
    int* remain = (int*)f_new;
    if( *remain < sizeof( DIRENTRY ) )
        return 1;
    strncpy( dir->name, fname, FILENAME_LEN );
    dir->size = size;
    dir->attr = attr;
    dir->ctime = *ctime;
    dir->mtime = *mtime;
    (*buf)++;
    *remain -= sizeof( DIRENTRY );
    return 0; //continue enumerating.
}

//读目录
int fat32_readdir( FILE_DESC* f, void* buf, t_32 size )
{
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
	int ret, remain = size;
    ret = fat32_enum_ent( f, fat32_readdir_enumerator, (FILE_DESC*)&remain, (char*)&buf );
	return (size - remain);
}

//写文件数据
int fat32_write( FILE_DESC* f, const char* buf, t_32 count )
{
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
	t_32 *fpos = &(f->pos), clu = fat32_get_clu_by_order( f, f->data, *fpos / fat->bytPerClu );
    char* clu_data;
	t_32 pos = *fpos % fat->bytPerClu, write;
	t_32 written = 0;
	//一个簇一个簇地写，直到写完
	while( count>0 )
	{
		write = fat->bytPerClu-pos;
		if( count<write) write=count;
		if( !IS_FAT32CLU_INUSE(clu) )
		{
		    if( IS_FAT32CLU_END( clu ) ) //增大文件
		    {
		        t_32 tmp = fat32_allocclu( f );
                if( !IS_FAT32CLU_INUSE(tmp) )
                    return -1; //error
				clu = tmp;
		    }else{
                printf("EOF: written=%d clu:%x\n", written, clu );
                return 0;
		    }
		}
		if( pos ) //pos不为0
		{
		    clu_data = fat32_readclu( f, clu );
		}else{
		    clu_data = (char*) malloc( fat->bytPerClu );
		}
		memcpy( clu_data+pos, buf, write );
        fat32_writeclu( f, clu, clu_data );
        free( clu_data );
		buf+=write;
		written+=write;
		pos=0;
		count-=write;
		if( count>0 ){
			clu = fat32_nextclu( f, clu );
		}
	}
	if( written )
	{
        (*fpos)+=written;
        if( *fpos > f->size )
        {
            f->size = *fpos;
            f->dirty = 1;
        }
	}
	return written;
}

//更名
int fat32_rename( FILE_DESC* f, char* name )
{
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
	int ret;
	//尝试这个文件名是否已用。
	{
        FILE_DESC tmp;
        memcpy( &tmp, f, sizeof(FILE_DESC) );
        tmp.flag = 0;
        if( fat32_open( &tmp, name )==0 )
        {
            return -1;
        }
	}
	ret = fat32_remove_ent( f );
	if( ret<0 )
	{
	    printf("Remove entry: %s failed.", f->name );
	    return -1;
	}
	strncpy( f->name, name, FILENAME_LEN );
	ret = fat32_create_ent( f );
	if( ret<0 )
	{
	    printf("AddNewEntry %s failed.", name );
	    return -1;
	}
	return 0;
}

//删除文件
int fat32_remove( FILE_DESC* f )
{
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
	//释放簇
	fat32_freeclu( f, f->data );
	//释放目录项
	fat32_remove_ent( f );
}

static int saveinfo_enumerator( char* fname, int size, TIME* ctime,
    TIME* mtime, int attr, int cluster, FILE_DESC* f_new, char* comp_name )
{
    //printf("Comparing %s & %s\n", fname, comp_name );
    if( stricmp( fname, comp_name )==0 )
    {
        return 3; //save this file information
    }
    return 0; //continue enumerating.
}

//保存文件信息
static int fat32_saveinfo( FILE_DESC* f )
{
	FILE_DESC* parent = f->parent;
	int ret;
	parent -> pos = 0; // find from first
	printf("save info: %s size:%d  clu:%x\n", f->name, f->size, f->data );
    ret = fat32_enum_ent( parent, saveinfo_enumerator, f, f->name );
    if( ret==3 ) //save ok
    {
        return 0;
    }else if ( ret==-1 ){ //couldn't find the file, maybe its name changed
        printf("%s: file not found. f->data: %x\n", f->name, f->data );
    }
	return -1;
}

//关闭文件, 保存目录项
int fat32_close( FILE_DESC* f )
{
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
	if( f->flag & FILE_FLAG_DELETE )
	{
	    fat32_remove( f );
	    printf("Delete %s", f->name );
	    return 0;
	}
	if( f->dirty )
	{
	    int ret = fat32_saveinfo( f );
	    f->dirty = 0;
	    if( ret < 0 )
	    {
	        printf("Save file info failed.");
	        return ret;
	    }
	}
	return 0;
}

//截断或增长文件
static int fat32_setsize( FILE_DESC* f, t_32 new_size )
{
	FAT32DEV* fat = (FAT32DEV*)f->dev_info;
    int old_no, new_no, clu;
    new_no = new_size / fat->bytPerClu + 1;
    old_no = f->size / fat->bytPerClu + 1;
    if( new_no < old_no ) //截断
    {
        //算算使用多少个块
        if( new_no == old_no ){
            f->size = new_size;
            return 0;
        }
        clu = fat32_get_clu_by_order( f, f->data, new_no );
        if( clu )
        {
            fat32_freeclu( f, clu );
        }else{
            //failed ?
            printf( "FAILED ?? " );
        }
    }else if( new_no > old_no ) //增长
    {
        while( new_no>old_no )
        {
            if( fat32_allocclu( f ) )
            {
                old_no++;
            }else{
                printf("FAILED!!!!!");
                return -1;
            }
        }
    }
    f->size = new_size;
    return 0;
}
