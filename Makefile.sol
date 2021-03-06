##############-fno-strict-prototype -fcond-mismatch
CC = gcc
CPP = g++
OBJ	= /tmp/obj
CSRC	= src_common
USRC	= src_unix
GSRC	= $(CSRC)/GD_Graphics
XSRC	= src_unix/gtk

OPTIM=-mcpu=pentium -O -fomit-frame-pointer -ffast-math -finline-functions -I/usr/include/glib
DEFINES = -D_PRO -D_UNIX -D__dest_os=6 -DDEF_APP_ENTERPRISE -DUSEPNG -DUSEJPEG
OPTIM_FBSD=-O3 -m486 -fomit-frame-pointer -ffast-math -finline-functions
OPTIM_SUNOS=-O2 -fomit-frame-pointer -ffast-math -finline-functions -fpermissive -D_SUNOS -I/usr/include/glib
# -DUSEPNG
CFLAGS = $(DEFINES) $(OPTIM) -fpermissive
GTKFLAGS = `gtk-config --cflags` -D__dest_os=6 
C++FLAGS = $(DEFINES) $(OPTIM) -fpermissive

INCLUDES = -I$(CSRC) -I$(CSRC)/Engine -I$(GSRC) -I$(USRC)\
	-I$(CSRC)/Images -I$(CSRC)/Translate -I$(CSRC)/Reports -I$(CSRC)/Reports/pdf -I$(CSRC)/Serial_Registration \
	-I$(CSRC)/Networking -Izlib -Iimages -I/usr/local/include
UNIXZLIB = /usr/lib/libz.a /usr/lib/libbz2.a
# /usr/lib/libpng.a
UNIXLIBS=-L/usr/local/lib -lm $(UNIXZLIB) /usr/local/lib/libares.a  /usr/lib/libpng.a /usr/lib/libjpeg.a -lstdc++
SUNOSLIBS=-L/usr/local/lib -lm -lsocket -lnsl -lmalloc $(UNIXZLIB) /usr/local/lib/libares.a /usr/local/lib/libpng.a /usr/local/lib/libstdc++.a.2.10.0 /usr/local/lib/libjpeg.a
GTKLIBS = `gtk-config --libs`
GTKSTATICLIBS = /usr/local/lib/libgtk.a /usr/local/lib/libgdk.a /usr/local/lib/libgmodule.a /usr/local/lib/libglib.a -ldl -lXext -lX11
# 
SEARCH_DIR=/usr/local/lib/
LFLAGS=-L$(OBJ)
BROKEN_BPRINTF_FLAGS=
RANLIB=true

SPACE := $(null) $(null)
VPATH = $(CSRC) $(CSRC)/Serial_Registration $(CSRC)/Translate $(CSRC)/xml $(CSRC)/Networking/ $(GSRC) $(CSRC)/Reports $(CSRC)/Reports/PDF $(CSRC)/Images/ $(CSRC)/Engine $(USRC) $(XSRC) $(OBJ)

COREOBJS	= DateFixFileName.o HelpCard.o schedule.o GlobalPaths.o \
		Myansi.o OSInfo.o postproc.o resource_strings.o unzip.o\
		unzip_layer.o Utilities.o version.o datetime.o
SERIALOBJ	= ice.o serialReg.o
TRANSOBJ	= translate.o
XMLOBJ		= qsxml.o
NETWORKOBJ	= httpinterface.o OutputMessages.o asyncdnr.o http_query.o smtp.o\
		unixnet_io.o ftp_client.o
ENGINEOBJS	=  engine_process.o engine_proc.o engine_drill.o\
		engine_dbio.o engine_notify.o engine_sql.o\
		EngineAddStats.o EngineBuff.o EngineClientDNR.o EngineCluster.o\
		EngineMeanPath.o EngineParse.o EngineRegion.o EngineRestoreStat.o\
		EngineStatus.o EngineVirtualDomain.o FailedRequestInfo.o FileTypes.o\
		Hash.o LogFileHistory.o BrokenLinkInfo.o DNSCache.o editpath.o\
		log_io.o StatCmp.o StatList.o Stats.o
SETTINGOBJS	= SettingsAppWide.o SettingsRobotsBrowsers.o SettingsSearchEngines.o\
		config.o
REPORTOBJS	= Report.o ReportInterface.o html_decodetemplate.o\
		ReportBase.o ReportFuncs.o report_keypages.o ReportSessions.o\
		ReportHTML.o ReportHTMLSessions.o HTMLFormat.o\
		ReportPDF.o ReportPDFSessions.o\
		ReportComma.o ReportExcel.o ReportRTF.o StandardPlot.o PDFPlot.o
PDFOBJS		= PDFCore.o PDFElements.o PDFFile.o PDFFont.o PDFImages.o\
		PDFSettings.o PDFTableData.o Icons.o

COMMONOBJS	= $(COREOBJS) $(SERIALOBJ) $(TRANSOBJ) $(XMLOBJ) $(NETWORKOBJ)\
		  $(ENGINEOBJS) $(SETTINGOBJS) $(REPORTOBJS) $(PDFOBJS)

GIFOBJS		= gd.o gdfont_Arial.o gdfonts.o gdfontt.o gdfont_Geneva.o webpalette.o
XOBJS		= Xmain.o gtkwin.o xfweb.o XPrefsGUI.o PrefsIO.o XSchedule.o resource.o gtkfiledialog.o
UNIXOBJS	= util.o unixreg.o dnr.o remotecontrol.o
CLIOBJS		= main.o

XOBJECTS	= $(COMMONOBJS) $(UNIXOBJS) $(XOBJS) $(GIFOBJS)
UOBJECTS	= $(COMMONOBJS) $(UNIXOBJS) $(CLIOBJS) $(GIFOBJS)

UNIXBIN		= iReporter
XBIN		= xiReporter
XSTATICBIN	= xiReporter-static
DEST		= /usr/local/Analyzer/bin/

.SUFFIXES:	.cpp

.cpp.o:
	@echo CPP Compiling $< with $@
	$(CPP) -c $(INCLUDES) $(C++FLAGS) $(SPACER) $< -o $(OBJ)/$@

.c.o:
	@echo C Compiling $< with $@
	$(CC) -c $(INCLUDES) $(CFLAGS) $(CFLAGS2) $(SPACER) $< -o $(OBJ)/$@


all: cli


# ---- Make the final executeable builds
console: $(UOBJECTS) 
	mkdir -p $(DEST)
	@cd $(OBJ) && $(CC) $(LFLAGS) -o $(DEST)/$(UNIXBIN) $(UOBJECTS) $(UNIXLIBS)
	@ln -sf $(DEST)/$(UNIXBIN) /usr/bin/$(UNIXBIN)
	@./gzipfweb $(UNIXBIN)

gui: $(XOBJECTS) 
	@cd $(OBJ) && $(CC) $(LFLAGS) -o $(DEST)/$(XBIN) $(XOBJECTS) $(UNIXLIBS) $(GTKLIBS)
	@ln -sf $(DEST)/$(XBIN) /usr/bin/$(XBIN)
	@./gzipfweb $(XBIN)

guistatic: $(XOBJECTS) 
	@cd $(OBJ) && $(CC) $(LFLAGS) -o $(DEST)/$(XSTATICBIN) $(XOBJECTS) $(UNIXLIBS) $(GTKLIBS)
	@ln -sf $(DEST)/$(XSTATICBIN) /usr/bin/$(XSTATICBIN)



gtk: $(XOBJS)
	@echo GTK Compiling



cli: $(UOBJECTS)
	@echo Building Analyzer CLI
	@echo ____________________________________________________________________
	make console 'CFLAGS=$(CFLAGS) -DTHREAD' 'UNIXLIBS=$(UNIXLIBS) -lpthread'
	@echo All Done.
	@echo ____________________________________________________________________

xwin:
	@echo Building Analyzer  GUI version
	@echo ____________________________________________________________________
	make gtk 'CFLAGS2=$(GTKFLAGS)'
	@echo Gtk Done.
	@echo Building Executeable
	@echo ____________________________________________________________________
	make gui 'CFLAGS=$(CFLAGS) -DUSEXWIN -DNOTHREAD' 'UNIXLIBS=$(UNIXLIBS) -lpthread -lgcc'
	@echo All Done.
	@echo ____________________________________________________________________

xstatic:
	@echo Building Static Analyzer  GUI
	@echo ____________________________________________________________________
	make gtk 'CFLAGS2=$(GTKFLAGS)'
	@echo Gtk Done.
	@echo Building Executeable
	@echo ____________________________________________________________________
	make guistatic 'CFLAGS=$(CFLAGS) -DUSEXWIN -DNOTHREAD' 'UNIXLIBS=$(UNIXLIBS) -lgcc'
	@echo All Done.
	@echo ____________________________________________________________________

complete:
	make
	make xwin
	make xstatic 'LFLAGS=$(LFLAGS) -static'
	make release


# ---- FREEBSD Builds
fbsd:
	make cli 'OPTIM=$(OPTIM_FBSD)'



# --------- SOLARIS AREA
cli-solaris:
	@echo CLI Solaris
	@echo =====================
	make cli 'UNIXLIBS=$(SUNOSLIBS)' 'OPTIM=$(OPTIM_SUNOS)'
	@echo Done.
	@echo =====


xwin-solaris:
	@echo Building GUI Solaris
	@echo =====================
	make xwin 'UNIXLIBS=$(SUNOSLIBS)' 'OPTIM=$(OPTIM_SUNOS)'
	@echo Done.
	@echo =====


xstatic-solaris:
	@echo Building Static GUI Solaris
	@echo =====================
	make xwinstatic 'UNIXLIBS=$(SUNOSLIBS)' 'OPTIM=$(OPTIM_SUNOS)' 'GTKLIBS=$(GTKSTATICLIBS)'
	@echo Done.
	@echo =====


complete-solaris:
	echo Building all Solaris
	make cli-solaris
	make xwin-solaris
	make xstatic-solaris

# --------- MISC stuff

english:
	@echo Converting english.lang
	@cd ApplicationData/Languages && bin2c English.lang 1 ../../src_common/English.h


resource:
	@cd src_win/gui && bin2c Fweb.rc 1 ../../src_unix/gtk/resource.c

stringresource:
	@bin/makestringres.pl src_win/gui/fweb.rc > src_common/resource_strings.c
	@bin/makestringres.pl src_win/gui/resource.h > src_common/resource_strings.h

unixinstall:
	cp $(UNIXBIN) /usr/bin/

get:
	-cp -uRpfv src_common/* /root/Dev/src_common/
	-cp -uRpfv src_unix/* /root/Dev/src_unix/
	-cp -uRpfv src_win/* /root/Dev/src_win/
	-cp -uRpfv Makefile /root/Dev/


put:
	-cp -uRpfv src_common/* /projects/iReporter/Dev/src_common/
	-cp -uRpfv src_unix/* /projects/iReporter/Dev/src_unix/
	-cp -uRpfv Makefile /projects/iReporter/Dev/

backup:
	-rm -f iReporter_src.zip
	-zip -r iReporter_src.zip src_common/*
	-zip -r iReporter_src.zip src_win/*
	-zip -r iReporter_src.zip src_unix/*
	-zip iReporter_src.zip iReporter\ VC++/*.dsp iReporter\ VC++/*.dsw
	-zip iReporter_src.zip Makefile gzipfweb buildrelease LICENCE README.TXT install-sh Languages/*


clean:
	-rm -f iReporter xiReporter xiReporter-static *.o *~ core
	-rm $(OBJ)/*~
	-rm $(OBJ)/*.o

tar:
	tar zcfv ../iReporter-unix.tgz src_common/* src_unix/* *.txt Makefile* src_unix/gtk/* buildrelease README.TXT LICENCE install-sh Languages/*

release:
	bash buildrelease


test:
	@echo $(SERSRC)
	@echo done
	

#serialReg.o: src_common/Serial?Registration/serialReg.c
#ice.o: src_common/Serial?Registration/ice.c
#gd.o: src_common/GD?Graphics/gd.c
#gdfont_Arial.o: src_common/GD?Graphics/gdfont_Arial.c
#gdfonts.o: src_common/GD?Graphics/gdfonts.c
#gdfontt.o: src_common/GD?Graphics/gdfontt.c
#gdfont_Geneva.o: src_common/GD?Graphics/gdfont_Geneva.c


gtkwin.o:  gtkwin.c gtkwin.h


