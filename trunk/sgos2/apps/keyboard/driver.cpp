#include <sgos.h>
#include <api.h>
#include <stdio.h>
#include <stdlib.h>
#include "port.h"
#include "keyboard.h"
#include "keymap_us.h"



// 硬件相关的定义
#define KB_DATA			0x60
#define KB_STATUS		0x64
#define KB_CONTROL		0x64
#define KB_IRQ			1
#define KB_COMMAND_ACK		0xFA	//键盘接受到命令的回应

#define KB_DOWN			1	//按下键
#define KB_UP			0	//释放键

// 键盘状态
#define KB_STATUS_ERROR		0x80
#define KB_STATUS_NOT_INTERRUPT	0x40
#define KB_STATUS_NOT_SEND	0x20
#define KB_STATUS_LOCKED	0x10
#define KB_STATUS_IS_COMMAND	0x8
#define KB_STATUS_OK		0x4
#define KB_STATUS_INPUT_BUFFER_FULL	0x2
#define KB_STATUS_OUTPUT_BUFFER_FULL	0x1

#define KB_EXT1	0xE0	//有一个附加键
#define KB_EXT2	0xE1	//有两个附加键

static int WaitForReply(uchar ackdata);
static char capsLock, numLock, scrollLock;
static char shiftFlag, altFlag, ctrlFlag;

static KEYMAP* currentKeymap;

static int GetStatus()
{
	return inbyte( KB_STATUS );
}

static void ResetKeyboard()
{
	outbyte( KB_DATA, 0xED );
	WaitForReply(KB_COMMAND_ACK);
	WaitForReply(0xAA);
}

static int WaitForBuffer()
{
	int times;
	for( times=1000000; times>0; times-- )
		if( !(GetStatus()&KB_STATUS_INPUT_BUFFER_FULL) )
			return 0;
	//出错。重启
	printf("[kb]##error: Buffer is full\n");
	ResetKeyboard();
}


static int WaitForReply(uchar ackdata)
{
	int times;
	for( times=1000000; times>0; times-- )
		if( inbyte(KB_DATA)==ackdata)
			return 0;
	printf("[kb]##error: keyboard no reply.\n");
	return -1;
}


static void SetKeyboardLeds()
{
	uchar data;
	data = 0;
	if( capsLock ) data |= 0x4;
	if( numLock ) data |= 0x2;
	if( scrollLock ) data |= 0x1;
	WaitForBuffer();
	outbyte( KB_DATA, 0xED );	//发送命令到0xED
	WaitForReply(KB_COMMAND_ACK);
	outbyte( KB_DATA, data );
	WaitForReply(KB_COMMAND_ACK);
}

static void TranslateKeyCode( uint code, uint keyDown, uint ext )
{
	uint ascii;
	if( keyDown )
	{
		switch( currentKeymap[code].style ){
		case KEYMAP_NORMAL:
			ascii = currentKeymap[code].nkey;
			break;
		case KEYMAP_SHIFT:
			if (shiftFlag)
				ascii = currentKeymap[code].mkey;
			else
				ascii = currentKeymap[code].nkey;
			break;
		case KEYMAP_LETTER:
			if (shiftFlag ^ capsLock)
				ascii = currentKeymap[code].mkey;
			else
				ascii = currentKeymap[code].nkey;
			break;
		case KEYMAP_NUMBER:
			if (numLock)
				ascii = currentKeymap[code].mkey;
			else
				ascii = currentKeymap[code].nkey;
			break;
		case KEYMAP_EXTRA:
			ascii = currentKeymap[code].nkey;
			switch (ascii) {
			case KEY_LCTRL:
			case KEY_RCTRL:
				ctrlFlag = 1;
				break;
			case KEY_LSHIFT:
			case KEY_RSHIFT:
				shiftFlag = 1;
				break;
			case KEY_LALT:
			case KEY_RALT:
				altFlag = 1;
				break;
			case KEY_CAPS:
				capsLock = capsLock ? 0 : 1;
				SetKeyboardLeds();
				break;
			case KEY_NUMBER:
				numLock = numLock ? 0 : 1;
				SetKeyboardLeds();
				break;
			case KEY_SCROLL:
				scrollLock = scrollLock ? 0 : 1;
				SetKeyboardLeds();
				break;
			}
			ascii=0;
		}
		if(ext)ascii=0;
	}else{
		ascii = currentKeymap[code].nkey;
		if (currentKeymap[code].style ==  KEYMAP_EXTRA) {
			switch (ascii) {
			case KEY_LCTRL:
				ctrlFlag = 0;
				ascii = currentKeymap[code].mkey;
				break;
			case KEY_RCTRL:
				ctrlFlag = 0;
				ascii = currentKeymap[code].mkey;
				break;
			case KEY_LSHIFT:
				shiftFlag = 0;
				ascii = currentKeymap[code].mkey;
				break;
			case KEY_RSHIFT:
				shiftFlag = 0;
				ascii = currentKeymap[code].mkey;
				break;
			case KEY_LALT:
				altFlag = 0;
				ascii = currentKeymap[code].mkey;
				break;
			case KEY_RALT:
				altFlag = 0;
				ascii = currentKeymap[code].mkey;
				break;
			}
		}
		ascii=0;
	}
	uint flag = 	(scrollLock<<0)|(numLock<<1)|(capsLock<<2)|
			(ctrlFlag<<4)|(shiftFlag<<5)|(altFlag<<6)|
			(keyDown<<7);
	printf("%c", ascii);
	DispatchKeyboardMessage( currentKeymap[code].nkey, ascii, flag );
}


void ReadKey()
{
	uint code, make, ext1, ext2;
	code = inbyte( KB_DATA );
	switch( code ){
	case 0xFA:
		break;
	case KB_EXT1:	//带一个附加码
		while( !(GetStatus()&KB_STATUS_OUTPUT_BUFFER_FULL) );
		code = inbyte( KB_DATA );
		if( code < 0x80 ){
			code += 0x80;
			TranslateKeyCode( code, KB_DOWN, KB_EXT1 );
		}else{
			TranslateKeyCode( code, KB_UP, KB_EXT1 );
		}
		break;
	case KB_EXT2:	//带两个附加码
		printf("[kb]## kbd can't handle ext2 key code\n");
		code = inbyte( KB_DATA );
		code = inbyte( KB_DATA );
		code = inbyte( KB_DATA );
		break;
	default:
		if( code < 0x80 )
		{
			TranslateKeyCode( code, KB_DOWN, 0 );
		}else{//是释放
			TranslateKeyCode( code - 0x80, KB_UP, 0 );
		}
	}
}

void HandleKeyboardInterrupt( int no )
{
	while(GetStatus()&KB_STATUS_OUTPUT_BUFFER_FULL)
		ReadKey();
	SysEnableInterrupt( KB_IRQ, 1 );
}


void SetKeymap( KEYMAP* km )
{
	currentKeymap = km;
}

void InitializeKeyboard()
{
	capsLock=0; numLock=0; scrollLock=0;
	shiftFlag = altFlag = ctrlFlag = 0;
	SetKeymap( KeyMap_US );
	if( SysAddInterruptHandler( KB_IRQ, SysGetCurrentThreadId() ) < 0 )
		printf("[kb]failed to add keyboard handler.\n");
	while(GetStatus()&KB_STATUS_OUTPUT_BUFFER_FULL)
		inbyte(KB_DATA);
	SysEnableInterrupt( KB_IRQ, 1 );
	if( !(GetStatus()&KB_STATUS_OK) )
	{
		printf("[kb]Resetting keyboard ...\n");
		ResetKeyboard();
	}
	SetKeyboardLeds();
	while(GetStatus()&KB_STATUS_OUTPUT_BUFFER_FULL)
		inbyte(KB_DATA);
}

