#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <sgos.h>

#define INPUT_BUFFER_SIZE 256

class SimpleConsole{
private:
	ushort	*video;
	ushort	*buffer;
	uint	inputQueue[INPUT_BUFFER_SIZE];
	int ifront, irear;
	int charWidth;
	int width, height;
	int cursorX, cursorY;
	int charStyle;
	
	void ScrollUp();
	void NextLine();
	void MoveCursor();
public:
	SimpleConsole();
	~SimpleConsole();
	int Initialize( int dev, int screen );
	void MoveCursorTo( int x, int y );
	void ClearScreen();
	void OutputChar( int ch );
	void OutputChar( int ch, int x, int y );
	int InputChar();
	void PutKeyChar( int ch );
	void ClearInput();
	void RefreshBuffer();
};

#endif
