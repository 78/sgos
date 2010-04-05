#include <sgos.h>
#include <string.h>
#include <stdlib.h>
#include <api.h>
#include "keyboard.h"

#define MAX_HANDLER 16

struct EventHandler{
	uint	threadId;
	uint	mode;
	uint	command;
};


static EventHandler handlerList[MAX_HANDLER];
static int handlerListCount=0;


static int SendEvent( uint tid, uint cmd, uint keycode, uint ascii, uint flag )
{
	Message msg;
	memset( &msg, 0, sizeof(msg) );
	msg.ThreadId = tid;
	msg.Command = cmd;
	msg.Arguments[0] = keycode;
	msg.Arguments[1] = ascii;
	msg.Arguments[2] = flag;
	return Api_Send( &msg, 0 );
}

void DispatchKeyboardMessage( uint keycode, uint ascii, uint flag )
{
	int i;
	for( i=0; i<handlerListCount; i++ )
	{
		if( handlerList[i].mode == KeyboardEvent_Character && !ascii )
			continue;
		if( SendEvent( handlerList[i].threadId, handlerList[i].command, keycode, ascii, flag ) < 0 )
			DelHandler( handlerList[i].threadId );
	}
}


void AddHandler( uint tid, uint cmd, uint mode )
{
	int i;
	for( i=0; i<handlerListCount; i++ )
		if( handlerList[i].threadId==tid )
			return ;
	i = handlerListCount;
	handlerList[i].threadId = tid;
	handlerList[i].mode = mode;
	handlerList[i].command = cmd;
	handlerListCount ++;
}

void DelHandler( uint tid )
{
	int i;
	for( i=0; i<handlerListCount; i++ )
		if( handlerList[i].threadId==tid ){
			if( i!=handlerListCount-1 )
				handlerList[i] = handlerList[handlerListCount-1];
			handlerListCount --;
		}
}

