CC = gcc
CFLAGS = -Wall -Wextra -ggdb
LDFLAGS = -Ilibharu/include -Ilibharu/build/include -lm

all: build libharu/build/src/libhpdf.so build/rubicon

build: build
	mkdir -p build

build/rubicon: src/*.c
	$(CC) $(CFLAGS) $(LDFLAGS) src/*.c -o build/rubicon


libharu/build/src/libhpdf.so: libharu/*
	cd libharu;			\
		mkdir -p build
	cd libharu/build;	\
		cmake ../;		\
		make install

.PHONY: all
