.PHONY: all clean lib src test

all:
	cd src; make all

clean:
	cd lib/nanomsg; make clean
	cd src; make clean

lib:
	cd lib/nanomsg; ./autogen.sh; ./configure; make

src:
	cd src; make

