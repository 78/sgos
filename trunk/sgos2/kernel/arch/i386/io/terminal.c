// Ported from SGOS1

#include <sgos.h>
#include <ctype.h>
#include <debug.h>
#include <string.h>
#include <stdarg.h>
#include <arch.h>
#include <terminal.h>

static int _w=80,_h=25;
static int _x=0, _y=0;	//光标位置
static short *video = (short*)0xC00B8000;	//显存地址
static short buffer[80*25*2];	//字符缓冲
static const unsigned short style = 0x0700;

static void scroll_up(); //向上卷屏
static void next_line();	//下一行
static void move_cursor(); //移动光标

static void next_line()
{
	_x=0;
	_y++;
	if(_y>=_h){
		scroll_up();
	}
}

static int gotoxy( int x, int y)
{
	if( x * _w + y > _w*_h)
		return -1;
	_x = x;
	_y = y;
	return 0;
}

static void move_cursor()
{
	//设置光标位置
	int offset = _y * _w + _x;
	//video[offset*2]=buffer[offset*2];
	memcpyw( video+_y * _w, buffer+_y * _w , 80);	//刷新光标所在项
	out_byte( 0x3D4, 14 );//指定访问14号寄存器
	out_byte( 0x3D5, offset >> 8  );
	out_byte( 0x3D4, 15 );
	out_byte( 0x3D5, offset&0xFF );
}


void clrscr()
{
	memsetw((char*)buffer, style, _w*_h );
	memcpyw( video, buffer, _w*_h);
	_x = 0;
	_y = 0;
	move_cursor();
}


void putchar(char ch)
{
	out_byte(0xE9,ch);	//for bochs
	if(ch=='\t')
	{
		putchar(' ');
		return;
	}
	if(ch=='\n')
	{
		next_line();
	}
	else if( ch == 8 )
	{
			buffer[_y*_w+ --_x]=style;
	}
	else if ( isprint((unsigned char)ch) )
	{
			buffer[_y*_w+_x++] = style | ch;
	}

	if(_x>=_w)
	{
		next_line();
	}
	move_cursor();
}

static void scroll_up()
{
	memcpy((char*)buffer,(char*)(buffer+_w),_w*(_h-1)*2);
	memsetw((char*)(buffer+_w*(_h-1)), style ,_w);
	memcpyw( video, buffer, _w*_h);
	_y--;
}
