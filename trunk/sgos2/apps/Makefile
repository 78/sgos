#Makefile for SGOS2:services


all:
	make -C../crt
	make -C../api
	make -Cfs
	make -Chd
	make -Cservicemanager
	make -Chello
	make -Cwprocess
	make -Cstartup
	make -Cwkernel
clean:
	make -C../crt clean
	make -C../api clean
	make -Cfs clean
	make -Chd clean
	make -Cservicemanager clean
	make -Chello clean
	make -Cwprocess clean
	make -Cstartup clean
	make -Cwkernel clean

