#include <sgos.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

SimpleConsole::SimpleConsole()
{
	video = buffer = 0;
	ifront = irear = 0;
	charWidth = 2;
	width = 80;
	height = 25;
	cursorX = cursorY = 0;
	charStyle = 0;
}

int SimpleConsole::Initialize( int dev, int screen )
{
}

SimpleConsole::~SimpleConsole(){
	if( buffer ){
		free( buffer );
	}
}

void SimpleConsole::ScrollUp()
{
	memcpyw( buffer, buffer+width, width*(height-1) );
	memsetw( buffer + width *( height - 1 ), charStyle , width );
	memcpyw( video, buffer, width * height );
	cursorY --;
}

void SimpleConsole::NextLine()
{
	cursorX = 0;
	cursorY ++;
	if(cursorY >= height)	//是否需要滚屏？
		ScrollUp();
}

void SimpleConsole::MoveCursor()
{
	MoveCursorTo( cursorX, cursorY );
	memcpyw( video + cursorY * width, buffer+ cursorY * width , width);	//刷新光标所在项
}

void SimpleConsole::MoveCursorTo( int x, int y )
{
	int offset = y * width + x;
	outbyte( 0x3D4, 14 );
	outbyte( 0x3D5, offset >> 8  );
	outbyte( 0x3D4, 15 );
	outbyte( 0x3D5, offset&0xFF );
}

void SimpleConsole::ClearScreen()
{
	memsetw( buffer, charStyle, width * height );
	RefreshBuffer();
	cursorX = 0;
	cursorY = 0;
	MoveCursor();
}

void SimpleConsole::OutputChar( int ch, int x, int y )
{
	buffer[ y * width + x ]=ch;	//到缓冲区
}

void SimpleConsole::OutputChar( int ch )
{
	outbyte(0xE9,ch);	//for bochs
	if(ch=='\t')
	{
		do{
			OutputChar(' ');
		}while( (cursorX-1)%4 );
		return;
	}
	if(ch=='\n')
	{
		NextLine();
	}
	else if( ch == 8 )
	{
		OutputChar( charStyle, cursorY, --cursorX );
	}
	else if ( isprint((unsigned char)ch) )
	{
		OutputChar( charStyle | ch, cursorY, cursorX++ );
	}
	if(cursorX >= width )
	{
		NextLine();
	}
	MoveCursor();
}

int SimpleConsole::InputChar()
{
}

void SimpleConsole::PutKeyChar( int ch )
{
}

void SimpleConsole::ClearInput()
{
	ifront = irear = 0;
}

void SimpleConsole::RefreshBuffer()
{
	memcpyw( video, buffer, width * height );
}
