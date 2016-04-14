CC = gcc
CFLAGS2 = `gtk-config --cflags` -D__dest_os=6 
INCLUDES = -I../../src_common/ -I../
OBJ = /tmp/obj4
OBJS = xfweb.o gtkwin.o XPrefsGUI.o XSchedule.o resource.o gtkfiledialog.o
VPATH = /tmp/obj4

.SUFFIXES:	.cpp

.c.o:
	@echo ===== Compiling $< with $@	
	$(CC) -c $(INCLUDES) $(CFLAGS2) $(SPACER) $< -o $(OBJ)/$@


libs: $(OBJS)

resource.o: resource.c

xfweb.o: xfweb.c

gtkwin.o:  gtkwin.c gtkwin.h

XPrefsGUI.o:   XPrefsGUI.c

PrefsIO.o:   ../../src_common/PrefsIO.c

XSchedule.o:	XSchedule.c

gtkfiledialog.o: gtkfiledialog.c

