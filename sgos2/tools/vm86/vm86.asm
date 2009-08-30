[bits 16]
[section .text]
[org 0x100]
; popping stack
pop	ax
mov byte [cs:int_no], al
pop	es
pop	ds
popa
db	0xCD
int_no
db	0x03
; save
pusha
push	ds
push	es
; exit
int	0xFB

