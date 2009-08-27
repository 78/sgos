
#include <system.h>
#include <stdio.h>
#include <string.h> //std string
#include <stdlib.h> //sprintf

namespace System{
	string::string()
	{
		//临时分配这么多空间了
		init();
		mksize(16);
	}
	
	string::string( const char *src )
	{
		init();
		if(src)
			copy(src);
	}
	
	//拷贝函数
	string::string( const string &src )
	{
		init();
		copy(src.buf);
	}
	
	string::~string()
	{
		if( buf )
			free(buf);
	}
	
	//初始化私有成员
	void string::init()
	{
		len = 0;
		buf_size = 0;
		buf = (char*)0;
	}
	
	//改变缓冲区大小
	void string::mksize(int siz)
	{
		char* newstr;
		if( siz > buf_size ){
			buf_size = siz;
			newstr = (char*)malloc(siz);
			if(len>0){ //原来有数据，则复制过去
				strcpy( newstr, buf );
			}
			//设置结束符，便于使用strcat等
			newstr[len] = '\0';
			if( buf )
				free(buf);
			buf = newstr;
		}
	}
	
	void string::copy ( const char *src )
	{
		int newlen;
		newlen = strlen(src);
		len = 0; 
		mksize(newlen+1);
		if(!buf)return;
		strcpy( buf, src );
		len = newlen;
	}
	
	//操作符重载
	char& string::operator [] ( int index )
	{
		//如果不做检查，会溢出。。。
		return buf[index];
	}
	
	string& string::operator = ( const char *src )
	{
		copy(src);
		return *this;
	}
	
	string& string::operator = ( const string& s )
	{
		copy(s.buf);
		return *this;
	}
	
	bool string::operator == ( const char *src )
	{
		if( !buf || !src )
			return false;
		return ( strcmp( buf, src )==0 );
	}
	
	bool string::operator == ( const string& s )
	{
		if( !buf || !s.buf )
			return false;
		return ( strcmp( buf, s.buf )==0 );
	}
	
	string& string::operator + ( const string& add2 )
	{
		len += add2.len;
		mksize( len+1 );
		strcat( buf, add2.buf );
		return *this;
	}
	
	string& string::operator + ( const char* src )
	{
		len += strlen(src);
		mksize( len+1 );
		strcat( buf, src );
		return *this;
	}
	
	string& string::operator += ( const string& add2 )
	{
		*this = *this + add2;
		return *this;
	}
	
	string& string::operator += ( const char* src )
	{
		*this = *this + src;
		return *this;
	}
	
	string::operator char *()
	{
		return buf;
	}
	
	// get integer value
	string::operator int()
	{
		int num = 0;
		bool neg = false;
		char* str = buf;
		if((*str>='0'&&*str<='9')||*str=='-'||*str=='+'){
			if(*str=='-'||*str=='+'){
				if(*str=='-')
					neg = true;
				str++;
			} 
		}else
			return 0;
		while(*str>='0'&&*str<='9')
			num = num*10 + (*str++ -'0');
		return neg?-num:num;
	}
	
	//成员函数
	const char * string::cstr()
	{
		return buf;
	}
	
	char string::charAt( int index )
	{
		return buf[index];
	}
	
	string& string::fromInteger( int number )
	{
		mksize(16);
		len = sprintf(buf, "%ld", number);
		return *this;
	}
	
	int string::length()
	{
		return len;
	}
	
	int string::find( string substr , int start )
	{
		char* found;
		if( start>=len )
			return -1;
		found = strstr( &buf[start], substr.buf );
		if(found)
			return (int)(found-buf);
		return -1;
	}
	
	int string::rfind( string substr )
	{
		//not implemented.
		return -1;
	}
	
	string string::trim( char part )
	{
		int beg, end;
		string tmp;
		for(beg=0; buf[beg]==part; beg++);
		for(end=len-1; buf[end]==part; end--);
		if(end-beg>=0){
			tmp.mksize(end-beg+1);
			memcpy(tmp.buf, &buf[beg], end-beg);
			tmp.len = end-beg;
			tmp.buf[tmp.len]='\0';
		}
		return tmp;
	}
	
	string string::left( int sublen )
	{
		string tmp;
		if(sublen>len)
			sublen=len;
		if(sublen>=0){
			tmp.mksize(sublen+1);
			memcpy(tmp.buf, &buf[0], sublen);
			tmp.len = sublen;
			tmp.buf[sublen]='\0';
		}
		return tmp;
	}
	
	string string::right( int sublen )
	{
		string tmp;
		if(sublen>len)
			sublen=len;
		if(sublen>=0){
			tmp.mksize(sublen+1);
			memcpy(tmp.buf, &buf[len-sublen-1], sublen);
			tmp.len = sublen;
			tmp.buf[sublen]='\0';
		}
		return tmp;
	}
	
	string string::mid( int start , int sublen )
	{
		string tmp;
		if(start+sublen>len){
			sublen=len-start;
		}
		if(start>=0&&sublen>0){
			tmp.mksize(sublen+1);
			memcpy(tmp.buf, &buf[start], sublen);
			tmp.len = sublen;
			tmp.buf[sublen]='\0';
		}
		return tmp;
	}
	
	// 下面的代码已经可以工作
	string string::replace( string src , string dest )
	{
		string tmp;
		int i, j;
		for(j=0,i=find(src.buf,0); i>=0; i=find(src.buf,j) ){
			tmp += mid(j,i-j) + dest;
			j = i + src.len;
		}
		tmp += mid(j, len-j);
		return tmp;
	}
}

