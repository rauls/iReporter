##############-fno-strict-prototype -fcond-mismatch
CC		= cc
CPP		= c++
OBJ		= /tmp/obj2
CSRC	= src_common
USRC	= src_unix
GSRC	= $(CSRC)/GD_Graphics
XSRC	= src_unix/gtk

# ----------------------------------------------

APPNAME		= iReporter
VER			= 2.0
UNIXBIN		= iReporter
UNIXBIN_S	= iReporter-static
UNIXBIN_F	= iReporter-free
XBIN		= xiReporter
XBIN_S		= xiReporter-static

INSTALL		= /usr/local/$(APPNAME)
ARCHFILE	= $(APPNAME)-$(VER)-$(OSTYPE)

ifeq ($(OSTYPE),solaris)
	INSTALL		= /$(APPNAME)
endif

ifeq ($(OSTYPE),darwin)
	INSTALL		= /Applications/$(APPNAME)
	ARCHFILE	= $(APPNAME)-$(VER)-macosx
endif

DESTDIR		= $(INSTALL)/bin/

DESTBIN		= $(DESTDIR)/$(UNIXBIN)
DESTBIN_S	= $(DESTDIR)/$(UNIXBIN_S)
DESTBIN_F	= $(DESTDIR)/$(UNIXBIN_F)
DESTBIN_X	= /usr/bin/$(XBIN)

TARFILE	= $(ARCHFILE).tar
ZIPFILE	= $(ARCHFILE).zip

# ----------------------------------------------------------------------------------------------------------------------------------------

DEFINES			= -D_PRO -D_UNIX -D__dest_os=6 -DDEF_ENTERPRISE -DDEF_FIREWALL -DUSEPNG -DUSEJPEG -DNOTHREAD

OPTIM_LINUX		= -mcpu=pentium -O2 -fomit-frame-pointer -ffast-math -finline-functions -I/usr/include/glib $(DEFINES)
OPTIM_LINUX_DYN = -mcpu=pentium -O2 -fomit-frame-pointer -ffast-math -finline-functions -I/usr/include/glib $(DEFINES)
OPTIM_FBSD		= -O3 -m486 -fomit-frame-pointer -ffast-math -finline-functions $(DEFINES)
OPTIM_SUNOS		= -O2 -fomit-frame-pointer -ffast-math -finline-functions -fpermissive -D_SUNOS -I/usr/include/glib -R/usr/local/lib/ $(DEFINES)
OPTIM_OSX		= -D_OSX -D__MACOSX__ -D__FreeBSD__ -Dsocklen_t=int -O2 -fomit-frame-pointer -ffast-math -fpermissive  $(DEFINES)

# -DUSEPNG
CFLAGS = $(OPTIM) $(CFLAGS2) -fpermissive
C++FLAGS = $(OPTIM) $(CFLAGS2) -fpermissive
GTKFLAGS = `gtk-config --cflags`

#
# --------- INCLUDES ----------
#
#LIBS_PATH = $(USRC)/linux86_libs/
LIBSPATH_LINUX = $(USRC)/linux86-libs
LIBSPATH_SUNOS = $(USRC)/solaris-libs
LIBSPATH_OSX = $(USRC)/macosx-libs

INCLUDES = -I$(CSRC) -I$(CSRC)/Engine -I$(GSRC) -I$(USRC)\
	-I$(CSRC)/Images -I$(CSRC)/Translate -I$(CSRC)/Reports -I$(CSRC)/Reports/pdf -I$(CSRC)/Serial_Registration -I$(CSRC)/Encryption\
	-I$(CSRC)/Networking -I$(CSRC)/Engine/Database  -I$(LIBS_PATH) -I/usr/local/include

#
# --------- LIBRARIES ----------
#

LIBS_UTIL = -lz -lbz2 -lares -lpng -ljpeg
LIBS_UTILSTATIC = $(LIBS_PATH)/libz.a $(LIBS_PATH)/libbz2.a $(LIBS_PATH)/libares.a  $(LIBS_PATH)/libpng.a $(LIBS_PATH)/libjpeg.a

LIBS_LINUX       = -lm -lstdc++ -lgcc
LIBS_LINUXSTATIC = /usr/lib/libm.a /usr/lib/libstdc++.a /usr/lib/libgcc.a

LIBS_SUNOS       = -lsocket -lmalloc -lnsl -lm /usr/local/lib/libstdc++.a
LIBS_SUNOSSTATIC = -lsocket -lmalloc -lm /usr/lib/libnsl.a /usr/local/lib/libstdc++.a /usr/lib/libm.a

LIBS_OSX       = -lm /usr/lib/libstdc++.a
LIBS_OSXSTATIC = -lsocket -lmalloc -lm /usr/lib/libstdc++.a /usr/lib/libm.a


LIBS   = -L/usr/local/lib $(LIBS_UTIL) $(LIBS_DYNAMIC)
LIBS_S = -L/usr/local/lib $(LIBS_UTILSTATIC) $(LIBS_STATIC)

GTKLIBS = `gtk-config --libs`
GTKSTATICLIBS = /usr/local/lib/libgtk.a /usr/local/lib/libgdk.a /usr/local/lib/libgmodule.a /usr/local/lib/libglib.a -ldl -lXext -lX11






# 
# ------------------ SOURCE FILES ---------------
#
SEARCH_DIR=/usr/local/lib/
LFLAGS=-L$(OBJ)
BROKEN_BPRINTF_FLAGS=
RANLIB=true

SPACE := $(null) $(null)
VPATH = $(CSRC) $(CSRC)/Serial_Registration $(CSRC)/Translate $(CSRC)/xml $(CSRC)/Networking/ $(CSRC)/Encryption/ $(CSRC)/Engine/Database/ $(GSRC) $(CSRC)/Reports $(CSRC)/Reports/PDF $(CSRC)/Images/ $(CSRC)/Engine $(USRC) $(XSRC) $(OBJ)

COREOBJS	= DateFixFileName.o HelpCard.o schedule.o GlobalPaths.o \
		Myansi.o OSInfo.o postproc.o resource_strings.o unzip.o zip.o\
		unzip_layer.o Utilities.o version.o datetime.o
SERIALOBJ	= ice.o IceStr.o serialReg.o
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


.SUFFIXES:	.cpp

.cpp.o:
	@echo;echo CPP Compiling $< with $@
	$(CPP) -c $(INCLUDES) $(C++FLAGS) $(CFLAGS2) $(SPACER) $< -o $(OBJ)/$@

.c.o:
	@echo;echo C Compiling $< with $@
	$(CC) -c $(INCLUDES) $(CFLAGS) $(CFLAGS2) $(SPACER) $< -o $(OBJ)/$@


all: echo and?


# ---- Make the final executeable builds
console: $(UOBJECTS) 
	mkdir -p $(DESTDIR)
	@echo libs are = $(LIBS_PATH)
	cd $(OBJ) && $(CC) $(LFLAGS) -o $(DESTBIN) $(UOBJECTS) $(LIBS)
	@rm -rf /usr/bin/$(UNIXBIN)
	@ln -sf $(DESTBIN) /usr/bin/$(UNIXBIN)
	@make gzipit 'EXE=$(DESTBIN)'

consolestatic: $(UOBJECTS) 
	mkdir -p $(DESTDIR)
	cd $(OBJ) && $(CC) $(LFLAGS) -o $(DESTBIN) $(UOBJECTS) $(LIBS_S)
	@rm -rf /usr/bin/$(UNIXBIN)
	@ln -sf $(DESTBIN) /usr/bin/$(UNIXBIN)
	@make gzipit 'EXE=$(DESTBIN)'


gui: $(XOBJECTS) 
	cd $(OBJ) && $(CC) $(LFLAGS) -o $(DESTBIN_X) $(XOBJECTS) $(LIBS) $(GTKLIBS)
#	@rm -rf /usr/bin/$(XBIN)
#	@ln -sf $(DESTDIR)/$(XBIN) /usr/bin/$(XBIN)
	@make gzipit 'EXE=$(XBIN)'

guistatic: $(XOBJECTS) 
	@cd $(OBJ) && $(CC) $(LFLAGS) -o $(DESTBIN_X) $(XOBJECTS) $(LIBS_S) $(GTKLIBS)
#	@rm -rf /usr/bin/$(XBIN)
#	@ln -sf $(DESTDIR)/$(XBIN) /usr/bin/$(XBIN)
	@make gzipit 'EXE=$(XBIN)'



gtk: $(XOBJS)
	@echo GTK Compiling



# ----------- MAIN PLATFORMS ------------

cli:
	@echo Building Analyzer CLI 'Full commercial version' for $(OSTYPE) to $(INSTALL)
	@echo ____________________________________________________________________
#make console 'LIBS_PATH=$(PWD)/$(LIBSPATH_LINUX)' 'LIBS_DYNAMIC=$(LIBS_LINUX)' 'OPTIM=$(OPTIM_LINUX)'
	make consolestatic 'LIBS_PATH=$(PWD)/$(LIBSPATH_LINUX)' 'LIBS_STATIC=$(LIBS_LINUXSTATIC)' 'OPTIM=$(OPTIM_LINUX)'
	@echo All Done.
	@echo ____________________________________________________________________

xwin:
	@echo Building Analyzer  GUI version for $(OSTYPE) to $(INSTALL)
	@echo ____________________________________________________________________
	make gtk 'CFLAGS2=$(GTKFLAGS)'
	@echo Gtk Done.
	@echo Building Executeable
	@echo ____________________________________________________________________
#make gui 'LIBS_PATH=$(PWD)/$(LIBSPATH_LINUX)' 'LIBS_DYNAMIC=$(LIBS_LINUX)' 'CFLAGS2=$(GTKFLAGS) -DUSEXWIN' 'OPTIM=$(OPTIM_LINUX)'
	make guistatic 'LIBS_PATH=$(PWD)/$(LIBSPATH_LINUX)' 'LIBS_STATIC=$(LIBS_LINUXSTATIC)' 'CFLAGS2=$(GTKFLAGS) -DUSEXWIN' 'OPTIM=$(OPTIM_LINUX)'
	@echo All Done.
	@echo ____________________________________________________________________




# ---- FREEBSD Builds
fbsd:
	make cli 'OPTIM=$(OPTIM_FBSD)'



# --------- SOLARIS AREA
cli-solaris:
	@echo Building Analyzer CLI for $(OSTYPE) to $(INSTALL)
	@echo ____________________________________________________________________
	-mkdir $(OBJ)
	make consolestatic 'LIBS_PATH=$(PWD)/$(LIBSPATH_SUNOS)' 'LIBS_STATIC=$(LIBS_SUNOSSTATIC)' 'OPTIM=$(OPTIM_SUNOS)'
	@echo All Done.
	@echo ____________________________________________________________________

xwin-solaris:
	@echo Building Static Analyzer GUI for $(OSTYPE) to $(INSTALL)
	@echo ____________________________________________________________________
	make gtk 'CFLAGS2=$(GTKFLAGS)' 'OPTIM=$(OPTIM_SUNOS)'
	@echo Gtk Done.
	@echo Building Executeable
	@echo ____________________________________________________________________
	make guistatic 'LIBS_PATH=$(PWD)/$(LIBSPATH_SUNOS)' 'LIBS_STATIC=$(LIBS_SUNOSSTATIC)' 'CFLAGS2=-DUSEXWIN' 'OPTIM=$(OPTIM_SUNOS)' 'GTKLIBS=$(GTKSTATICLIBS)'
	@echo All Done.
	@echo ____________________________________________________________________




# --------- MAC OSX
cli-osx:
	@echo Building Analyzer CLI for $(OSTYPE) to $(INSTALL)
	@echo ____________________________________________________________________
	make consolestatic 'LIBS_PATH=$(PWD)/$(LIBSPATH_OSX)' 'LIBS_STATIC=$(LIBS_OSX)' 'OPTIM=$(OPTIM_OSX)'
	@echo All Done.
	@echo ____________________________________________________________________




clean:
	-rm $(OBJ)/*~
	-rm $(OBJ)/*.o

english:
	@echo Converting english.lang
	@cd ApplicationData/Languages && bin2c English.lang 1 ../../src_common/English.h


resource:
	@cd src_win/GUI/ && bin2c Fweb.rc 1 ../../src_unix/gtk/resource.c
	@perl bin/makenames.pl src_win/gui/resource.h | grep IDC > src_unix/gtk/resourceid.h
	@perl bin/makenames.pl src_win/gui/resource.h | grep IDM >> src_unix/gtk/resourceid.h

stringresource:
	@perl bin/makestringres.pl src_win/gui/fweb.rc > src_common/resource_strings.c
	@perl bin/makestringres.pl src_win/gui/resource.h > src_common/resource_strings.h







# --------- MISC stuff



gzipit:
	@echo
	@echo Stripping $(EXE)
	cp $(EXE) /tmp/iReporter
	strip /tmp/iReporter
	echo Compressing $(EXE)
	gzip -c /tmp/iReporter > /unix_builds/$(ARCHFILE)-execbinary.gz
	zip -D /unix_builds/$(ARCHFILE)-execbinary.zip /tmp/iReporter
	@echo


install:
	@echo
	@echo Building $(VER) install archive $(ARCHFILE) from $(INSTALL)
	@echo
	@echo Stripping binaries...
	-mkdir $(INSTALL)
	-strip $(INSTALL)/bin/$(UNIXBIN)
	-strip $(INSTALL)/bin/$(UNIXBIN_S)
	-strip $(INSTALL)/bin/$(UNIXBIN_F)
	-strip $(INSTALL)/bin/$(XBIN)
	-strip $(INSTALL)/bin/$(XBIN_S)
	@echo
	@echo Copying programs...
	-mkdir $(INSTALL)/bin
	@echo
	@echo Copying data...
	-cp -pR ApplicationData/* $(INSTALL)
	-rm -rf $(INSTALL)/Analyzer*
	-rm -rf $(INSTALL)/http_config
	-rm -f $(INSTALL)/dnr.cache
	-rm -f $(INSTALL)/schedlog.txt
	-find $(INSTALL)/* -name "vssver.scc" -exec rm -f {} \;
	@echo
	@echo Copying manuals...
	-mkdir $(INSTALL)/manual
	-cp ../manual/* $(INSTALL)/manual/
	@echo
	@echo Redoing permissions...
	-chown -R nobody.nobody $(INSTALL)/*
	-chmod -R 755 $(INSTALL)/*
	find $(INSTALL)/ -name "*" -exec touch {} \;
	find $(INSTALL)/ -name "*.scc" -exec rm {} \;
	@echo Done.


tarpackage:
	@echo
	@echo Making tar ...
	tar cfv /tmp/$(TARFILE) $(INSTALL)/*
	@echo
	@echo Gziping tar...
	gzip -9f /tmp/$(TARFILE)
	mv /tmp/$(TARFILE).gz /unix_builds/
	@echo
	ls -l /unix_builds/$(TARFILE).gz
	@echo Done.


zippackage:
	@echo
	@echo Making ZIP ...
	zip -r /tmp/$(ZIPFILE) $(INSTALL)/*
	@echo
	mv /tmp/$(ZIPFILE) /unix_builds/
	@echo
	ls -l /unix_builds/$(ZIPFILE)
	@echo Done.



# -------- PKGINFO FILE
#PKG="SCIr2"
#NAME="Analyzer"
#VERSION="4.5"
#ARCH="sparc"
#CLASSES="none"
#CATEGORY="web"
#VENDOR="Software (C)"
#PSTAMP="10thMay02"
#EMAIL="support@ireporter.com"
#BASEDIR="/opt/"

PROT = /tmp/$(APPNAME).prototype

DEV = $(PWD)

# ---- make a solaris package install file.
sunpackage:
	@echo "!search $(INSTALL),$(INSTALL)/bin,$(INSTALL)/Languages,$(INSTALL)/Languages/English,$(INSTALL)/Languages/English/HelpCards,$(INSTALL)/manual" > $(PROT)
	@echo "i pkginfo=$(DEV)/pkginfo" >> $(PROT)
	@echo "i request=$(DEV)/pkginfo-request.sh" >> $(PROT)
	@echo "i postinstall=$(DEV)/pkginfo-postinstall.sh" >> $(PROT)
	cd /; find ./Analyzer -print | pkgproto >> $(PROT)
	cd $(DEV)
	pkgmk -o -f $(PROT)

	pkgtrans -s /var/spool/pkg /tmp/$(APPNAME)-4.5-$(MACHTYPE)-$(OSTYPE) SCiReporter
	ls -l /tmp/$(APPNAME)-4.5-$(MACHTYPE)-$(OSTYPE).pkg
	cp /tmp/$(APPNAME)-4.5-$(MACHTYPE)-$(OSTYPE).pkg /unix_builds/
	gzip -f /tmp/$(APPNAME)-4.5-$(MACHTYPE)-$(OSTYPE).pkg
	mv /tmp/$(APPNAME)-4.5-$(MACHTYPE)-$(OSTYPE).pkg.gz /unix_builds/
	
# ------------------------ Solaris 

sunrelease:
	make install
	make tarpackage
	make sunpackage

all-solaris:
	make cli-solaris
	make sunrelease
	
	

# ------------------------ Linux 

release:
	make install
	make tarpackage

all-linux:
	make cli 'LFLAGS=$(LFLAGS) -static'
	make release


# ------------------------ Mac OSX

getsource:
	-cp -uRpfv /projects/iReporter/DEV/ApplicationData/* .
	-cp -uRpfv /projects/iReporter/DEV/src_common/* .
	-cp -uRpfv /projects/iReporter/DEV/src_unix/* .
	-cp -uRpfv /projects/iReporter/DEV/Makefile .

osxrelease:
	make install
	make zippackage

all-osx:
	make cli-osx
	make osxrelease


# ---------------
get:
	-cp -uRpfv ApplicationData/* /root/Dev/ApplicationData/
	-cp -uRpfv src_common/* /root/Dev/src_common/
	-cp -uRpfv src_unix/* /root/Dev/src_unix/
	-cp -uRpfv src_win/* /root/Dev/src_win/
	-cp -uRpfv Makefile /root/Dev/


put:
	-cp -uRpfv src_common/* /projects/iReporter/Dev/src_common/
	-cp -uRpfv src_unix/* /projects/iReporter/Dev/src_unix/
	-cp -uRpfv Makefile /projects/iReporter/Dev/


srctar:
	tar zcfv ../iReporter-src.tgz src_common/* src_unix/* src_unix/gtk/* *.rust pkginfo* config*.sh Makefile ApplicationData/Languages/* ApplicationData/*.html ApplicationData/*.txt ApplicationData/*.gif


gtkwin.o:  gtkwin.c gtkwin.h


