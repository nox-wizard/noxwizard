# ---------------------------------------------------------------------------
!if !$d(BCB)
BCB = $(MAKEDIR)\..
!endif

# ---------------------------------------------------------------------------
# IDE SECTION
# ---------------------------------------------------------------------------
# The following section of the project makefile is managed by the BCB IDE.
# It is recommended to use the IDE to change any of the values in this
# section.
# ---------------------------------------------------------------------------

VERSION = BCB.06.00
# ---------------------------------------------------------------------------
PROJECT = noxwizard.exe
OBJFILES = nox-wizard.obj worldmain.obj addmenu.obj amx\amxcback.obj \
    amx\amxfile.obj amx\amxscript.obj amx\amxvarserver.obj amx\amxwrap2.obj \
    amx\amxwraps.obj area.obj \
    basics.obj boats.obj books.obj bounty.obj calendar.obj \
    char_array.obj chars.obj client.obj cmdtable.obj combat.obj \
    commands.obj common_libs.obj console.obj containers.obj crontab.obj \
    custmenu.obj dbl_single_click.obj debug.obj dragdrop.obj \
    encryption.obj fishing.obj amx\float.obj gamedate.obj globals.obj guild.obj \
    guildstones.obj gumps.obj house.obj html.obj item_array.obj itemid.obj \
    items.obj jail.obj logsystem.obj magic.obj map.obj data.obj \
    mount.obj msgboard.obj network.obj \
    newbie.obj npcai.obj npcs.obj ntservice.obj nxw_utils.obj nxwgui.obj \
    nxwGump.obj nxwString.obj nxwStringList.obj object.obj packets.obj \
    particles.obj party.obj pointer.obj poll.obj race.obj raceability.obj \
    racegender.obj racerequiredtype.obj racescriptentry.obj raceskill.obj \
    racestartitem.obj racetokenizer.obj racetype.obj range.obj rcvpkg.obj \
    regions.obj remadmin.obj scp_parser.obj scripts.obj set.obj skills.obj \
    skitarg.obj sndpkg.obj spawn.obj speech.obj sregions.obj srvparms.obj \
    targeting.obj telport.obj thievery.obj timers.obj timing.obj titles.obj \
    tmpeff.obj trade.obj translation.obj trigger.obj utils.obj walking.obj \
    weight.obj accounts.obj commands\tweaking.obj commands\privlevels.obj \
    commands\addremove.obj amxcore.obj amx_comp.obj amx_dbg.obj amxcons.obj \
    amx.obj extractionskills.obj archive.obj item.obj abstraction\endian.obj \
    oldmenu.obj menu.obj organisation.obj ai.obj
RESFILES = xwgui.res
MAINSOURCE = noxwizard.bpf
RESDEPEN = $(RESFILES)
LIBFILES = 
IDLFILES =
IDLGENFILES =
LIBRARIES = 
PACKAGES = vcldbx.bpi cds.bpi teeui.bpi teedb.bpi tee.bpi teeqr.bpi inetdb.bpi \
    dbxcds.bpi
SPARELIBS = 
DEFFILE = 
OTHERFILES = 
# ---------------------------------------------------------------------------
DEBUGLIBPATH = $(BCB)\lib\debug
RELEASELIBPATH = $(BCB)\lib\release
USERDEFINES = WIN32;NDEBUG;_WINDOWS
SYSDEFINES = NO_STRICT;_NO_VCL;_RTLDLL
INCLUDEPATH = $(BCB)\include;$(BCB)\include\vcl;commands;.
LIBPATH = $(BCB)\Projects\Lib;$(BCB)\lib\obj;$(BCB)\lib;commands
WARNINGS= -w-par
PATHCPP = .;commands;amx;abstraction;db;
PATHASM = .;
PATHPAS = .;
PATHRC = .;
PATHOBJ = .;$(LIBPATH)
# ---------------------------------------------------------------------------
CFLAG1 = -O2 -w- -Vx -Ve -X- -a1 -6 -b -k- -vi -tW -tWM -c
IDLCFLAGS = -Icommands -I$(BCB)\include -I$(BCB)\include\vcl -src_suffix cpp -DWIN32 \
    -DNDEBUG -D_WINDOWS -boa
PFLAGS = -$Y- -$L- -$D- -$A8 -v -JPHNE -M
RFLAGS = 
AFLAGS = /mx /w2 /zn
LFLAGS = -D"" -aa -Tpe -x -Gn
# ---------------------------------------------------------------------------
ALLOBJ = c0w32.obj $(OBJFILES)
ALLRES = $(RESFILES)
ALLLIB = $(LIBFILES) $(LIBRARIES) import32.lib cw32mti.lib
# ---------------------------------------------------------------------------
!ifdef IDEOPTIONS

[Version Info]
IncludeVerInfo=0
AutoIncBuild=0
MajorVer=1
MinorVer=0
Release=0
Build=0
Debug=0
PreRelease=0
Special=0
Private=0
DLL=0

[Version Info Keys]
CompanyName=
FileDescription=
FileVersion=1.0.0.0
InternalName=
LegalCopyright=
LegalTrademarks=
OriginalFilename=
ProductName=
ProductVersion=1.0.0.0
Comments=

[Debugging]
DebugSourceDirs=$(BCB)\source\vcl

!endif





# ---------------------------------------------------------------------------
# MAKE SECTION
# ---------------------------------------------------------------------------
# This section of the project file is not used by the BCB IDE.  It is for
# the benefit of building from the command-line using the MAKE utility.
# ---------------------------------------------------------------------------

.autodepend
# ---------------------------------------------------------------------------
!if "$(USERDEFINES)" != ""
AUSERDEFINES = -d$(USERDEFINES:;= -d)
!else
AUSERDEFINES =
!endif

!if !$d(BCC32)
BCC32 = bcc32
!endif

!if !$d(CPP32)
CPP32 = cpp32
!endif

!if !$d(DCC32)
DCC32 = dcc32
!endif

!if !$d(TASM32)
TASM32 = tasm32
!endif

!if !$d(LINKER)
LINKER = ilink32
!endif

!if !$d(BRCC32)
BRCC32 = brcc32
!endif


# ---------------------------------------------------------------------------
!if $d(PATHCPP)
.PATH.CPP = $(PATHCPP)
.PATH.C   = $(PATHCPP)
!endif

!if $d(PATHPAS)
.PATH.PAS = $(PATHPAS)
!endif

!if $d(PATHASM)
.PATH.ASM = $(PATHASM)
!endif

!if $d(PATHRC)
.PATH.RC  = $(PATHRC)
!endif

!if $d(PATHOBJ)
.PATH.OBJ  = $(PATHOBJ)
!endif
# ---------------------------------------------------------------------------
$(PROJECT): $(OTHERFILES) $(IDLGENFILES) $(OBJFILES) $(RESDEPEN) $(DEFFILE)
    $(BCB)\BIN\$(LINKER) @&&!
    $(LFLAGS) -L$(LIBPATH) +
    $(ALLOBJ), +
    $(PROJECT),, +
    $(ALLLIB), +
    $(DEFFILE), +
    $(ALLRES)
!
# ---------------------------------------------------------------------------
.pas.hpp:
    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }

.pas.obj:
    $(BCB)\BIN\$(DCC32) $(PFLAGS) -U$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -O$(INCLUDEPATH) --BCB {$< }

.cpp.obj:
    $(BCB)\BIN\$(BCC32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }

.c.obj:
    $(BCB)\BIN\$(BCC32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n$(@D) {$< }

.c.i:
    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }

.cpp.i:
    $(BCB)\BIN\$(CPP32) $(CFLAG1) $(WARNINGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -n. {$< }

.asm.obj:
    $(BCB)\BIN\$(TASM32) $(AFLAGS) -i$(INCLUDEPATH:;= -i) $(AUSERDEFINES) -d$(SYSDEFINES:;= -d) $<, $@

.rc.res:
    $(BCB)\BIN\$(BRCC32) $(RFLAGS) -I$(INCLUDEPATH) -D$(USERDEFINES);$(SYSDEFINES) -fo$@ $<



# ---------------------------------------------------------------------------




