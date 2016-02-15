
TARGETS=parsetime gentime loadschedule lintschedule lintschedule2 lintschedule3 showmeta autochop1

CFLAGS_COMMON=-I.

host=$(shell uname -m)
ifeq ($(host),x86_64)
LIBNAME=lib64
else
LIBNAME=lib
endif

os=$(shell uname -s)
ifeq ($(os),Linux)
CFLAGS_COMMON += -DLINUX -D_FILE_OFFSET_BITS=64
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

all: parsetime gentime loadschedule lintschedule lintschedule2 lintschedule3 showmeta autochop1

clean:
	rm -fv *.a *.la *.o */*.o $(TARGETS)

install: libcastus4public.a
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -v gentime $(DESTDIR)$(PREFIX)/bin/castus4public_demo_gentime
	cp -v parsetime $(DESTDIR)$(PREFIX)/bin/castus4public_demo_parsetime
	mkdir -p $(DESTDIR)$(PREFIX)/$(LIBNAME)
	cp -v *.a $(DESTDIR)$(PREFIX)/$(LIBNAME)/
	mkdir -p $(DESTDIR)$(PREFIX)/include
	mkdir -p $(DESTDIR)$(PREFIX)/include/castus4-public
	cp -v castus4-public/*.h $(DESTDIR)$(PREFIX)/include/castus4-public/
	mkdir -p $(DESTDIR)/mnt/main/tv/schedule-filters/
	cp -v autochop1 $(DESTDIR)/mnt/main/tv/schedule-filters/

libcastus4public.a: castus4-public/libcastus4public_parsetime.o castus4-public/libcastus4public_gentime.o castus4-public/libcastus4public_chomp.o castus4-public/libcastus4public_schedule_object.o castus4-public/libcastus4public_schedule_helpers.o castus4-public/libcastus4public_metadata.o
	rm -fv $@
	$(AR) r $@ $^

castus4-public/libcastus4public_chomp.o: castus4-public/libcastus4public_chomp.c
	$(CC) $(CXXFLAGS) -c -o $@ $^

castus4-public/libcastus4public_gentime.o: castus4-public/libcastus4public_gentime.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

castus4-public/libcastus4public_metadata.o: castus4-public/libcastus4public_metadata.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

castus4-public/libcastus4public_parsetime.o: castus4-public/libcastus4public_parsetime.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

castus4-public/libcastus4public_schedule_object.o: castus4-public/libcastus4public_schedule_object.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

castus4-public/libcastus4public_schedule_helpers.o: castus4-public/libcastus4public_schedule_helpers.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

parsetime: parsetime.o libcastus4public.a
	$(CXX) -o $@ $^ $(LDFLAGS)

parsetime.o: parsetime.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

gentime: gentime.o libcastus4public.a
	$(CXX) -o $@ $^ $(LDFLAGS)

gentime.o: gentime.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

loadschedule: loadschedule.o libcastus4public.a
	$(CXX) -o $@ $^ $(LDFLAGS)

loadschedule.o: loadschedule.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

lintschedule: lintschedule.o libcastus4public.a
	$(CXX) -o $@ $^ $(LDFLAGS)

lintschedule.o: lintschedule.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

lintschedule2: lintschedule2.o libcastus4public.a
	$(CXX) -o $@ $^ $(LDFLAGS)

lintschedule2.o: lintschedule2.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

lintschedule3: lintschedule3.o libcastus4public.a
	$(CXX) -o $@ $^ $(LDFLAGS)

lintschedule3.o: lintschedule3.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

showmeta: showmeta.o libcastus4public.a
	$(CXX) -o $@ $^ $(LDFLAGS)

showmeta.o: showmeta.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

autochop1: autochop1.o libcastus4public.a
	$(CXX) -o $@ $^ $(LDFLAGS)

autochop1.o: autochop1.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

