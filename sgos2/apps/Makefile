#Makefile for SGOS2:services


all:
	make -C../api
	make -C../libc
	make -Cfs
	make -Chd
	make -Cservicemanager
	make -Chello

clean:
	make -C../api clean
	make -C../libc clean
	make -Cfs clean
	make -Chd clean
	make -Cservicemanager clean
	make -Chello clean

