bin_PROGRAMS = viewer
viewer_SOURCES = viewer.c align.c main.c mono.c list.c shared.c
man_MANS = viewer.1
viewer_LDFLAGS = @EXTRALIBS@
viewer_LDADD = @LIBDIRS@ @LDFLAGS@
if DEBUG_ENABLE
DEBUG = -DDEBUG -g
else
DEBUG =
endif
viewer_CFLAGS = @CFLAGS@ @INCDIRS@ -DOS_@OS_TYPE@ ${DEBUG}
