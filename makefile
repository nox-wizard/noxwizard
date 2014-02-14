#############################################################################
# Makefile for building noxwizard
# Generated by tmake at 20:28, 2001/11/20
#     Project: nxw
#    Template: app
#############################################################################

####### Compiler, tools and options

CC	=	gcc
CXX	=	g++

CFLAGS	=	-pipe -D_REENTRANT -O2
CXXFLAGS	= -pipe -funsigned-char -D_REENTRANT -O2 -fpermissive
INCPATH =	-I ./
LINK	=	g++
LFLAGS	=	-lc -lstdc++
LIBS	=	$(SUBLIBS) -lpthread
MOC	=	$(QTDIR)/bin/moc
UIC	=	$(QTDIR)/bin/uic

TAR	=	tar -cf
GZIP	=	gzip -9f

include makefile.inc

INTERFACES =
UICDECLS =
UICIMPLS =
SRCMOC	=
OBJMOC	=
DIST	=
TARGET	=	noxwizard
INTERFACE_DECL_PATH = .

####### Implicit rules

.SUFFIXES: .cpp .cxx .cc .C .c

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

####### Build rules

## old all:
# all: $(TARGET)

all:
	@echo "/-----------------------------------------------"
	@echo "| Using make is deprecated! Use \`scons\` instead!"
	@echo "| The makefile will be removed in the future!"
	@echo "\-----------------------------------------------"
	@scons
	@echo "/-----------------------------------------------"
	@echo "| Using make is deprecated! Use \`scons\` instead!"
	@echo "| The makefile will be removed in the future!"
	@echo "\-----------------------------------------------"

$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC)
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)

moc: $(SRCMOC)

tmake:
	tmake nxw.pro

dist:
	$(TAR) nxw.tar nxw.pro $(SOURCES) $(HEADERS) $(INTERFACES) $(DIST)
	$(GZIP) nxw.tar

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC) $(UICIMPLS) $(UICDECLS) $(TARGET)
	-rm -f *~ core

####### Sub-libraries


###### Combined headers


####### Compile

