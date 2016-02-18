
ACLOCAL_AMFLAGS = -I m4

pkgconfiglibdir = $(libdir)/pkgconfig
# This REALLY should be based in libexec!
schedfilterdir = /mnt/main/tv/schedule-filters
castusheaderdir = $(includedir)/castus4-public

lib_LTLIBRARIES = \
    libcastus4public.la

# NOTE: Verify these get subdir'd
castusheader_HEADERS = castus4-public/*.h

bin_PROGRAMS = \
    castus4public_demo_parsetime \
    castus4public_demo_gentime

schedfilter_PROGRAMS = \
    autochop1

noinst_PROGRAMS = \
    loadschedule \
    lintschedule \
    lintschedule2 \
    lintschedule3 \
    showmeta

pkgconfiglib_DATA = \
	castus4public.pc

libcastus4public_la_SOURCES = \
    castus4-public/libcastus4public_chomp.c \
    castus4-public/libcastus4public_gentime.cpp \
    castus4-public/libcastus4public_metadata.cpp \
    castus4-public/libcastus4public_parsetime.cpp \
    castus4-public/libcastus4public_schedule_object.cpp \
    castus4-public/libcastus4public_schedule_helpers.cpp

castus4public_demo_parsetime_SOURCES = parsetime.cpp
castus4public_demo_parsetime_LDADD = libcastus4public.la

castus4public_demo_gentime_SOURCES = gentime.cpp
castus4public_demo_gentime_LDADD = libcastus4public.la

autochop1_SOURCES = autochop1.cpp
autochop1_LDADD = libcastus4public.la

loadschedule_SOURCES = loadschedule.cpp
loadschedule_LDADD = libcastus4public.la

lintschedule_SOURCES = lintschedule.cpp
lintschedule_LDADD = libcastus4public.la

lintschedule2_SOURCES = lintschedule2.cpp
lintschedule2_LDADD = libcastus4public.la

lintschedule3_SOURCES = lintschedule3.cpp
lintschedule3_LDADD = libcastus4public.la

showmeta_SOURCES = showmeta.cpp
showmeta_LDADD = libcastus4public.la
