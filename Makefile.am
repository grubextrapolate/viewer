## process this file with automake to generate Makefile.in

AUTOMAKE_OPTIONS = dist-zip dist-bzip2
bin_PROGRAMS = viewer
viewer_SOURCES = viewer.c align.c main.c mono.c list.c shared.c viewer.h
man_MANS = viewer.1
data_DATA = AUTHORS COPYING ChangeLog INSTALL NEWS README
EXTRA_DIST = mac.r viewer.1 viewer.spec viewer.vcproj Makefile.win

if DEBUG_ENABLE
DEBUG = -DDEBUG -g
else
DEBUG =
endif

LDFLAGS = @VIEWER_EXTRALIBS@
LDADD = @VIEWER_LIBS@ @VIEWER_LIBDIRS@ @VIEWER_LDFLAGS@
CFLAGS = @VIEWER_INCDIRS@ -DOS_@OS_TYPE@ ${DEBUG}

mac: viewer mac.r
	/Developer/Tools/Rez -t APPL -o viewer mac.r
