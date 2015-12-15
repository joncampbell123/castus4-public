
CFLAGS_COMMON=-I.

host=$(shell uname -m)
ifeq ($(host),x86_64)
LIBNAME=lib64
else
LIBNAME=lib
endif

os=$(shell uname -s)
ifeq ($(os),Linux)
CFLAGS_COMMON += -DLINUX
endif

CFLAGS=$(CFLAGS_COMMON)
CXXFLAGS=$(CFLAGS_COMMON)

DESTDIR ?= 
PREFIX ?= /usr
CXX ?= g++
CPP ?= cpp
CC ?= gcc
AR ?= ar
LD ?= ld

all: parsetime

clean:
	rm -fv *.a *.la *.o

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/$(LIBNAME)
	mkdir -p $(DESTDIR)$(PREFIX)/include

parsetime: parsetime.o
	$(CXX) -o $@ $< $(LDFLAGS)

parsetime.o: parsetime.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

