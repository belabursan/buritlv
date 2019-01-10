# Makefile for ctlv

all:
	gcc -m64 -Wall -c -O3 -g0 -Werror -s -std=c99 -fPIC  -MMD -MP -MF "tlv.o.d" -o tlv.o tlv.c
	gcc -m64 -Wall -o libctlv.so tlv.o  -shared -s -fPIC
	rm -f *.d
	rm -f *.o

help:
	@echo "  Run \"make\" or \"make -j2\" to compile the shared library"
	@echo "  Run \"make clean\" to remove the shared library and object files"
	@echo "  Run \"make help\" to show this help"

.PHONY: clean

clean:
	rm -f *.d
	rm -f *.o
	rm -f *.so
