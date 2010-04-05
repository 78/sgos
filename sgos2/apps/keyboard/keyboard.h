#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_


//如果什么键盘类型需要增加，
//就尽管增好了，只要序号不同。
#define KEY_NONE	0x0
#define KEY_SPACE	0x20
#define KEY_ESC		0x1B
#define KEY_BS		0x08
#define KEY_BACKSPACE	KEY_BS
#define KEY_TAB		0x09
#define KEY_ENTER	0x0A
#define KEY_RETURN	KEY_ENTER
#define KEY_DEL		0x7F
#define KEY_DELETE	KEY_DEL


#define KEYSNUM 256
typedef struct KEYMAP{
	int style;
	unsigned short int nkey;
	unsigned short int mkey;
} KEYMAP;

#define KEYMAP_NORMAL	0
#define KEYMAP_SHIFT	1
#define KEYMAP_LETTER	2
#define KEYMAP_NUMBER	3
#define KEYMAP_EXTRA	4



/* Normal function keys */
#define FUN_KEY(key)	(0x0100 | (key))
#define SHIFT_KEY(key)	(0x0200 | (key))
#define CTRL_KEY(key)	(0x0400 | (key))
#define ALT_KEY(key)	(0x0800 | (key))
#define LWIN_KEY(key)	(0x1000 | (key))
#define RWIN_KEY(key)	(0x2000 | (key))
#define MENU_KEY(key)	(0x4000 | (key))

/* Numeric keypad */
#define KEY_HOME	FUN_KEY(0x01)
#define KEY_END		FUN_KEY(0x02)
#define KEY_UP		FUN_KEY(0x03)
#define KEY_DOWN	FUN_KEY(0x04)
#define KEY_LEFT	FUN_KEY(0x05)
#define KEY_RIGHT	FUN_KEY(0x06)
#define KEY_PGUP	FUN_KEY(0x07)
#define KEY_PAGEUP	KEY_PGUP
#define KEY_PGDN	FUN_KEY(0x08)
#define KEY_PAGEDOWN	KEY_PGDN
#define KEY_MID		FUN_KEY(0x09)
#define KEY_NMIN	FUN_KEY(0x0A)
#define KEY_PLUS	FUN_KEY(0x0B)
#define KEY_INSERT	FUN_KEY(0x0C)
#define KEY_INS		KEY_INSERT

/* Lock keys */
#define KEY_CAPS	FUN_KEY(0x0D)	/* caps lock	*/
#define KEY_NUMBER	FUN_KEY(0x0E)	/* number lock	*/
#define KEY_SCROLL	FUN_KEY(0x0F)	/* scroll lock	*/

/* Function keys */
#define KEY_F1		FUN_KEY(0x80)
#define KEY_F2		FUN_KEY(0x81)
#define KEY_F3		FUN_KEY(0x82)
#define KEY_F4		FUN_KEY(0x83)
#define KEY_F5		FUN_KEY(0x84)
#define KEY_F6		FUN_KEY(0x85)
#define KEY_F7		FUN_KEY(0x86)
#define KEY_F8		FUN_KEY(0x87)
#define KEY_F9		FUN_KEY(0x88)
#define KEY_F10		FUN_KEY(0x89)
#define KEY_F11		FUN_KEY(0x8A)
#define KEY_F12		FUN_KEY(0x8B)
#define KEY_F13		FUN_KEY(0x8C)
#define KEY_F14		FUN_KEY(0x8D)
#define KEY_F15		FUN_KEY(0x8E)
#define KEY_F16		FUN_KEY(0x8F)
#define KEY_F17		FUN_KEY(0x90)
#define KEY_F18		FUN_KEY(0x91)
#define KEY_F19		FUN_KEY(0x92)
#define KEY_F20		FUN_KEY(0x93)
#define KEY_F21		FUN_KEY(0x94)
#define KEY_F22		FUN_KEY(0x95)
#define KEY_F23		FUN_KEY(0x96)
#define KEY_F24		FUN_KEY(0x97)

#define KEY_LSHIFT	FUN_KEY(0x30)
#define KEY_RSHIFT	FUN_KEY(0x31)
#define KEY_LCTRL	FUN_KEY(0x32)
#define KEY_RCTRL	FUN_KEY(0x33)
#define KEY_LALT	FUN_KEY(0x34)
#define KEY_RALT	FUN_KEY(0x35)

#define KEY_SHIFT	FUN_KEY(0x36)
#define KEY_CTRL	FUN_KEY(0x37)
#define KEY_ALT		FUN_KEY(0x38)

#define KEY_KANA		FUN_KEY(0x40)
#define KEY_ABNT_C1		FUN_KEY(0x41)
#define KEY_CONVERT		FUN_KEY(0x42)
#define KEY_NOCONVERT	FUN_KEY(0x43)
#define KEY_YEN			FUN_KEY(0x44)
#define KEY_ABNT_C2		FUN_KEY(0x45)
#define KEY_NUMPADEQUALS FUN_KEY(0x46)
#define KEY_PREVTRACK	FUN_KEY(0x47)
#define KEY_AT			FUN_KEY(0x48)
#define KEY_COLON		FUN_KEY(0x49)
#define KEY_UNDERLINE	FUN_KEY(0x4A)
#define KEY_KANJI		FUN_KEY(0x4B)
#define KEY_STOP		FUN_KEY(0x4C)
#define KEY_AX			FUN_KEY(0x4D)
#define KEY_UNLABELED	FUN_KEY(0x4E)
#define KEY_NEXTTRACK	FUN_KEY(0x4F)
#define KEY_MUTE		FUN_KEY(0x50)
#define KEY_CALCULATOR	FUN_KEY(0x51)
#define KEY_PLAYPAUSE	FUN_KEY(0x52)
#define KEY_MEDIASTOP	FUN_KEY(0x53)
#define KEY_VOLUMEDOWN	FUN_KEY(0x54)
#define KEY_VOLUMEUP	FUN_KEY(0x55)
#define KEY_WEBHOME		FUN_KEY(0x56)
#define KEY_SYSRQ		FUN_KEY(0x57)
#define KEY_PAUSE		FUN_KEY(0x58)

#define KEY_LWIN		FUN_KEY(0x60)
#define KEY_RWIN		FUN_KEY(0x61)
#define KEY_WIN			FUN_KEY(0x62)

#define KEY_APPS		FUN_KEY(0x63)
#define KEY_POWER		FUN_KEY(0x64)
#define KEY_SLEEP		FUN_KEY(0x65)
#define KEY_WAKE		FUN_KEY(0x66)
#define KEY_WEBSEARCH	FUN_KEY(0x67)
#define KEY_WEBFAVORITES FUN_KEY(0x68)
#define KEY_WEBREFRESH	FUN_KEY(0x69)
#define KEY_WEBSTOP		FUN_KEY(0x6A)
#define KEY_WEBFORWARD	FUN_KEY(0x6B)
#define KEY_WEBBACK		FUN_KEY(0x6C)
#define KEY_MYCOMPUTER	FUN_KEY(0x6D)
#define KEY_MAIL		FUN_KEY(0x6E)
#define KEY_MEDIASELECT	FUN_KEY(0x6F)

#define KEY_OEM_102		FUN_KEY(0x70)
#define KEY_NUMPADCOMMA	FUN_KEY(0x71)

/* Mouse Buttons */
#define KEY_BUTTON1	FUN_KEY(0xF0)
#define KEY_LBUTTON	KEY_BUTTON1
#define KEY_BUTTON2	FUN_KEY(0xF1)
#define KEY_RBUTTON	KEY_BUTTON2
#define KEY_BUTTON3	FUN_KEY(0xF2)
#define KEY_MBUTTON	KEY_BUTTON3


void SetKeymap( KEYMAP* km );
void InitializeKeyboard();
void HandleKeyboardInterrupt( int no );
void AddHandler( uint tid, uint cmd, uint mode );
void DelHandler( uint tid );
void DispatchKeyboardMessage( uint keycode, uint ascii, uint flag );

#endif
