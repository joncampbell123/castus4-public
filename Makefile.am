
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(top_srcdir)/include

pkgconfiglibdir = $(libdir)/pkgconfig
# This REALLY should be based in libexec!
schedfilterdir = /mnt/main/tv/schedule-filters
castusheaderdir = $(includedir)/castus4-public

lib_LTLIBRARIES = \
    libcastus4-public.la

castusheader_HEADERS = $(top_srcdir)/include/castus4-public/*

bin_PROGRAMS = \
    castus4-public_demo_parsetime \
    castus4-public_demo_gentime

schedfilter_PROGRAMS = \
    autochop1

noinst_PROGRAMS = \
    loadschedule \
    lintschedule \
    lintschedule2 \
    lintschedule3 \
    showmeta

pkgconfiglib_DATA = \
	castus4-public.pc

libcastus4_public_la_SOURCES = \
    src/lib/chomp.c \
    src/lib/gentime.cpp \
    src/lib/metadata.cpp \
    src/lib/parsetime.cpp \
    src/lib/schedule_object.cpp \
    src/lib/schedule_helpers.cpp

castus4_public_demo_parsetime_SOURCES = src/bin/parsetime.cpp
castus4_public_demo_parsetime_LDADD = libcastus4-public.la

castus4_public_demo_gentime_SOURCES = src/bin/gentime.cpp
castus4_public_demo_gentime_LDADD = libcastus4-public.la

autochop1_SOURCES = src/bin/autochop1.cpp
autochop1_LDADD = libcastus4-public.la

loadschedule_SOURCES = src/bin/loadschedule.cpp
loadschedule_LDADD = libcastus4-public.la

lintschedule_SOURCES = src/bin/lintschedule.cpp
lintschedule_LDADD = libcastus4-public.la

lintschedule2_SOURCES = src/bin/lintschedule2.cpp
lintschedule2_LDADD = libcastus4-public.la

lintschedule3_SOURCES = src/bin/lintschedule3.cpp
lintschedule3_LDADD = libcastus4-public.la

showmeta_SOURCES = src/bin/showmeta.cpp
showmeta_LDADD = libcastus4-public.la
