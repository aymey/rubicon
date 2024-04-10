CC = gcc
CFLAGS = -Wall -Wextra -ggdb

all: build libharu/build/src/libhpdf.so build/rubicon

build: build
	mkdir -p build

build/rubicon: src/main.c
	export LD_LIBRARY_PATH=libharu/build/src:$LD_LIBRARY_PATH
	$(CC) $(CFLAGS) -Ilibharu/include -Ilibharu/build/include -Llibharu/build/src/ -lhpdf src/main.c -o build/rubicon


libharu/build/src/libhpdf.so: libharu/*
	cd libharu;			\
		mkdir -p build
	cd libharu/build;	\
			cmake ../;	\
			make

.PHONY: all
