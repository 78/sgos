#include <system.h>

using namespace System::Service;

static void startSound(unsigned int freq)
{
	unsigned int Divisor;
	if( freq==0 )
		return;
	Divisor= 1193180 / freq;

	outbyte(0x43, 0xB6);
	outbyte(0x42, Divisor & 0xFF);
	outbyte(0x42, Divisor >> 8);

	outbyte(0x61, inbyte(0x61) | 3);
}

static void stopSound()
{
	outbyte(0x61, inbyte(0x61) & ~3);
}

void play(double freq, unsigned int delay)
{
	startSound((unsigned int)freq);
	System::Thread::Sleep(delay);
	stopSound();
}

void test()
{
	play(261.626,300); //C4
	play(293.665,300); //D4
	play(329.628,300); //E4
	play(329.628,1000); //E4
	play(329.628,300); //E4
	play(349.228,300); //F4
	play(391.995,300); //G4
	play(391.995,800); //G4
	play(391.995,300); //G4
	play(349.228,300); //F4
	play(329.628,300); //E4
	play(293.665,1000); //D4
	play(293.665,300); //D4
	play(329.628,300); //E4
	play(261.626,1000); //C4
	
}

void beep()
{
	play(392,100); //G4
}
