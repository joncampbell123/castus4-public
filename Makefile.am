
ACLOCAL_AMFLAGS = -I m4

pkgconfiglibdir = $(libdir)/pkgconfig
# This REALLY should be based in libexec!
schedfilterdir = /mnt/main/tv/schedule-filters
castusheaderdir = $(includedir)/castus4-public

lib_LTLIBRARIES = \
    libcastus4-public.la

# NOTE: Verify these get subdir'd
castusheader_HEADERS = castus4-public/*.h

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
    castus4-public/chomp.c \
    castus4-public/gentime.cpp \
    castus4-public/metadata.cpp \
    castus4-public/parsetime.cpp \
    castus4-public/schedule_object.cpp \
    castus4-public/schedule_helpers.cpp

castus4_public_demo_parsetime_SOURCES = parsetime.cpp
castus4_public_demo_parsetime_LDADD = libcastus4-public.la

castus4_public_demo_gentime_SOURCES = gentime.cpp
castus4_public_demo_gentime_LDADD = libcastus4-public.la

autochop1_SOURCES = autochop1.cpp
autochop1_LDADD = libcastus4-public.la

loadschedule_SOURCES = loadschedule.cpp
loadschedule_LDADD = libcastus4-public.la

lintschedule_SOURCES = lintschedule.cpp
lintschedule_LDADD = libcastus4-public.la

lintschedule2_SOURCES = lintschedule2.cpp
lintschedule2_LDADD = libcastus4-public.la

lintschedule3_SOURCES = lintschedule3.cpp
lintschedule3_LDADD = libcastus4-public.la

showmeta_SOURCES = showmeta.cpp
showmeta_LDADD = libcastus4-public.la
