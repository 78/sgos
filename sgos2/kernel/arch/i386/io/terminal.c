// Ported from SGOS1

#include <sgos.h>
#include <stdarg.h>
#include <rtl.h>
#include <ctype.h>
#include <kd.h>
#include <arch.h>
#include <terminal.h>

static int _w=80,_h=25;
static int _x=0, _y=0;	//光标位置
static short *video = (short*)0xC00B8000;	//显存地址
static short buffer[80*25*2];	//字符缓冲
static unsigned short style = 0x0700;

static void scroll_up(); //向上卷屏
static void next_line();	//下一行
static void move_cursor(); //移动光标

// 下一行
static void next_line()
{
	_x=0;
	_y++;
	if(_y>=_h)	//是否需要滚屏？
		scroll_up();
}

// move the cursor as you input a character
static void move_cursor()
{
	//设置光标位置
	int offset = _y * _w + _x;
	//video[offset*2]=buffer[offset*2];
	memcpy16( video+_y * _w, buffer+_y * _w , 80);	//刷新光标所在项
	ArOutByte( 0x3D4, 14 );//指定访问14号寄存器
	ArOutByte( 0x3D5, offset >> 8  );
	ArOutByte( 0x3D4, 15 );
	ArOutByte( 0x3D5, offset&0xFF );
}

// clear the screen
void ArClearScreen()
{
	memset16((char*)buffer, style, _w*_h );
	memcpy16( video, buffer, _w*_h);
	_x = 0;
	_y = 0;
	move_cursor();
}

// print a character
void ArPrintChar(char ch)
{
	ArOutByte(0xE9,ch);	//for bochs
	uint eflags;
	if(ch=='\t')
	{
		do{
			ArPrintChar(' ');
		}while( (_x-1)%4 );
		return;
	}
	//特殊字符处理
	switch( ch ){
	case '#':
		style = 0x0C00;
		break;
	case '[':
		style = 0x0200;
		break;
	}
	ArLocalSaveIrq( eflags );
	//
	if(ch=='\n')
	{
		next_line();
	}
	else if( ch == 8 )
	{
			buffer[_y*_w+ --_x]=style;	//到缓冲区
	}
	else if ( isprint((unsigned char)ch) )
	{
			buffer[_y*_w+_x++] = style | ch;	//到缓冲区
	}
	//恢复正常字体颜色
	if( ch== ']'||ch=='\n' )	
		style = 0x0700;
		
	if(_x>=_w)
	{
		next_line();
	}
	//移动光标
	move_cursor();
	ArLocalRestoreIrq( eflags );
}

// scroll up the screen
static void scroll_up()
{
	memcpy16((char*)buffer,(char*)(buffer+_w),_w*(_h-1) );
	memset16((char*)(buffer+_w*(_h-1)), style ,_w);
	memcpy16( video, buffer, _w*_h);
	_y--;
}
