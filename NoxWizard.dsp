# Microsoft Developer Studio Project File - Name="NoxWizard" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=NoxWizard - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NoxWizard.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NoxWizard.mak" CFG="NoxWizard - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NoxWizard - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "NoxWizard - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "NoxWizard"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NoxWizard - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /J /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x410 /d "NDEBUG"
# ADD RSC /l 0x410 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "NoxWizard - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /J /FD /GZ /c
# SUBTRACT CPP /WX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x410 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCMT" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "NoxWizard - Win32 Release"
# Name "NoxWizard - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "encryption"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\encryption\aes.h
# End Source File
# Begin Source File

SOURCE=.\encryption\ClientCrypt.cpp
# End Source File
# Begin Source File

SOURCE=.\encryption\clientCrypt.h
# End Source File
# Begin Source File

SOURCE=.\encryption\common.h
# End Source File
# Begin Source File

SOURCE=.\encryption\crypt.cpp
# End Source File
# Begin Source File

SOURCE=.\encryption\crypt.h
# End Source File
# Begin Source File

SOURCE=.\encryption\platform.h
# End Source File
# Begin Source File

SOURCE=.\encryption\table.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\accounts.cpp
# End Source File
# Begin Source File

SOURCE=.\addmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\ai.cpp
# End Source File
# Begin Source File

SOURCE=.\amx\amx.c
# End Source File
# Begin Source File

SOURCE=.\amx\amx_comp.c
# End Source File
# Begin Source File

SOURCE=.\amx\amx_dbg.c
# End Source File
# Begin Source File

SOURCE=.\amx\amxcback.cpp
# End Source File
# Begin Source File

SOURCE=.\amx\amxcons.c
# End Source File
# Begin Source File

SOURCE=.\amx\amxcore.c
# End Source File
# Begin Source File

SOURCE=.\amx\amxfile.cpp
# End Source File
# Begin Source File

SOURCE=.\amx\amxscript.cpp
# End Source File
# Begin Source File

SOURCE=.\amx\amxvarserver.cpp
# End Source File
# Begin Source File

SOURCE=.\amx\amxwrap2.cpp
# End Source File
# Begin Source File

SOURCE=.\amx\amxwraps.cpp
# End Source File
# Begin Source File

SOURCE=.\archive.cpp
# End Source File
# Begin Source File

SOURCE=.\area.cpp
# End Source File
# Begin Source File

SOURCE=.\basics.cpp
# End Source File
# Begin Source File

SOURCE=.\boats.cpp
# End Source File
# Begin Source File

SOURCE=.\books.cpp
# End Source File
# Begin Source File

SOURCE=.\bounty.cpp
# End Source File
# Begin Source File

SOURCE=.\calendar.cpp
# End Source File
# Begin Source File

SOURCE=.\chars.cpp
# End Source File
# Begin Source File

SOURCE=.\client.cpp
# End Source File
# Begin Source File

SOURCE=.\cmds.cpp
# End Source File
# Begin Source File

SOURCE=.\cmdtable.cpp
# End Source File
# Begin Source File

SOURCE=.\combat.cpp
# End Source File
# Begin Source File

SOURCE=.\commands.cpp
# End Source File
# Begin Source File

SOURCE=.\common_libs.cpp
# End Source File
# Begin Source File

SOURCE=.\console.cpp
# End Source File
# Begin Source File

SOURCE=.\containers.cpp
# End Source File
# Begin Source File

SOURCE=.\crontab.cpp
# End Source File
# Begin Source File

SOURCE=.\data.cpp
# End Source File
# Begin Source File

SOURCE=.\dbl_single_click.cpp
# End Source File
# Begin Source File

SOURCE=.\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\dragdrop.cpp
# End Source File
# Begin Source File

SOURCE=.\encryption.cpp
# End Source File
# Begin Source File

SOURCE=.\abstraction\endian.cpp
# End Source File
# Begin Source File

SOURCE=.\extractionskills.cpp
# End Source File
# Begin Source File

SOURCE=.\fishing.cpp
# End Source File
# Begin Source File

SOURCE=.\amx\float.cpp
# End Source File
# Begin Source File

SOURCE=.\gamedate.cpp
# End Source File
# Begin Source File

SOURCE=.\globals.cpp
# End Source File
# Begin Source File

SOURCE=.\guild.cpp
# End Source File
# Begin Source File

SOURCE=.\guildstones.cpp
# End Source File
# Begin Source File

SOURCE=.\house.cpp
# End Source File
# Begin Source File

SOURCE=.\html.cpp
# End Source File
# Begin Source File

SOURCE=.\item.cpp
# End Source File
# Begin Source File

SOURCE=.\itemid.cpp
# End Source File
# Begin Source File

SOURCE=.\items.cpp
# End Source File
# Begin Source File

SOURCE=.\jail.cpp
# End Source File
# Begin Source File

SOURCE=.\logsystem.cpp
# End Source File
# Begin Source File

SOURCE=.\magic.cpp
# End Source File
# Begin Source File

SOURCE=.\map.cpp
# End Source File
# Begin Source File

SOURCE=.\menu.cpp
# End Source File
# Begin Source File

SOURCE=.\mount.cpp
# End Source File
# Begin Source File

SOURCE=.\msgboard.cpp
# End Source File
# Begin Source File

SOURCE=.\network.cpp
# End Source File
# Begin Source File

SOURCE=.\newbie.cpp
# End Source File
# Begin Source File

SOURCE=".\nox-wizard.cpp"
# End Source File
# Begin Source File

SOURCE=.\npcai.cpp
# End Source File
# Begin Source File

SOURCE=.\npcs.cpp
# End Source File
# Begin Source File

SOURCE=.\ntservice.cpp
# End Source File
# Begin Source File

SOURCE=.\nxw_utils.cpp
# End Source File
# Begin Source File

SOURCE=.\nxwgui.cpp
# End Source File
# Begin Source File

SOURCE=.\object.cpp
# End Source File
# Begin Source File

SOURCE=.\oldmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\packets.cpp
# End Source File
# Begin Source File

SOURCE=.\particles.cpp
# End Source File
# Begin Source File

SOURCE=.\party.cpp
# End Source File
# Begin Source File

SOURCE=.\pointer.cpp
# End Source File
# Begin Source File

SOURCE=.\poll.cpp
# End Source File
# Begin Source File

SOURCE=.\race.cpp
# End Source File
# Begin Source File

SOURCE=.\raceability.cpp
# End Source File
# Begin Source File

SOURCE=.\racegender.cpp
# End Source File
# Begin Source File

SOURCE=.\racerequiredtype.cpp
# End Source File
# Begin Source File

SOURCE=.\racescriptentry.cpp
# End Source File
# Begin Source File

SOURCE=.\raceskill.cpp
# End Source File
# Begin Source File

SOURCE=.\racestartitem.cpp
# End Source File
# Begin Source File

SOURCE=.\racetokenizer.cpp
# End Source File
# Begin Source File

SOURCE=.\racetype.cpp
# End Source File
# Begin Source File

SOURCE=.\range.cpp
# End Source File
# Begin Source File

SOURCE=.\rcvpkg.cpp
# End Source File
# Begin Source File

SOURCE=.\regions.cpp
# End Source File
# Begin Source File

SOURCE=.\remadmin.cpp
# End Source File
# Begin Source File

SOURCE=.\scp_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\scripts.cpp
# End Source File
# Begin Source File

SOURCE=.\set.cpp
# End Source File
# Begin Source File

SOURCE=.\skills.cpp
# End Source File
# Begin Source File

SOURCE=.\skitarg.cpp
# End Source File
# Begin Source File

SOURCE=.\sndpkg.cpp
# End Source File
# Begin Source File

SOURCE=.\spawn.cpp
# End Source File
# Begin Source File

SOURCE=.\speech.cpp
# End Source File
# Begin Source File

SOURCE=.\sregions.cpp
# End Source File
# Begin Source File

SOURCE=.\srvparms.cpp
# End Source File
# Begin Source File

SOURCE=.\target.cpp
# End Source File
# Begin Source File

SOURCE=.\targeting.cpp
# End Source File
# Begin Source File

SOURCE=.\telport.cpp
# End Source File
# Begin Source File

SOURCE=.\thievery.cpp
# End Source File
# Begin Source File

SOURCE=.\timers.cpp
# End Source File
# Begin Source File

SOURCE=.\timing.cpp
# End Source File
# Begin Source File

SOURCE=.\titles.cpp
# End Source File
# Begin Source File

SOURCE=.\tmpeff.cpp
# End Source File
# Begin Source File

SOURCE=.\trade.cpp
# End Source File
# Begin Source File

SOURCE=.\translation.cpp
# End Source File
# Begin Source File

SOURCE=.\trigger.cpp
# End Source File
# Begin Source File

SOURCE=.\utils.cpp
# End Source File
# Begin Source File

SOURCE=.\walking.cpp
# End Source File
# Begin Source File

SOURCE=.\weight.cpp
# End Source File
# Begin Source File

SOURCE=.\worldmain.cpp
# End Source File
# Begin Source File

SOURCE=.\xwgui.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\accounts.h
# End Source File
# Begin Source File

SOURCE=.\addmenu.h
# End Source File
# Begin Source File

SOURCE=.\ai.h
# End Source File
# Begin Source File

SOURCE=.\amx\amx.h
# End Source File
# Begin Source File

SOURCE=.\amx\amx_api.h
# End Source File
# Begin Source File

SOURCE=.\amx\amx_comp.h
# End Source File
# Begin Source File

SOURCE=.\amx\amxcback.h
# End Source File
# Begin Source File

SOURCE=.\amx\amxfile.h
# End Source File
# Begin Source File

SOURCE=.\amx\amxscript.h
# End Source File
# Begin Source File

SOURCE=.\amx\amxvarserver.h
# End Source File
# Begin Source File

SOURCE=.\amx\amxwraps.h
# End Source File
# Begin Source File

SOURCE=.\archive.h
# End Source File
# Begin Source File

SOURCE=.\area.h
# End Source File
# Begin Source File

SOURCE=.\basics.h
# End Source File
# Begin Source File

SOURCE=.\boats.h
# End Source File
# Begin Source File

SOURCE=.\books.h
# End Source File
# Begin Source File

SOURCE=.\bounty.h
# End Source File
# Begin Source File

SOURCE=.\calendar.h
# End Source File
# Begin Source File

SOURCE=.\chars.h
# End Source File
# Begin Source File

SOURCE=.\classes.h
# End Source File
# Begin Source File

SOURCE=.\client.h
# End Source File
# Begin Source File

SOURCE=.\cmds.h
# End Source File
# Begin Source File

SOURCE=.\cmdtable.h
# End Source File
# Begin Source File

SOURCE=.\commands.h
# End Source File
# Begin Source File

SOURCE=.\common_libs.h
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# Begin Source File

SOURCE=.\constants.h
# End Source File
# Begin Source File

SOURCE=.\containers.h
# End Source File
# Begin Source File

SOURCE=.\crontab.h
# End Source File
# Begin Source File

SOURCE=.\data.h
# End Source File
# Begin Source File

SOURCE=.\dbl_single_click.h
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\dragdrop.h
# End Source File
# Begin Source File

SOURCE=.\encryption.h
# End Source File
# Begin Source File

SOURCE=.\abstraction\endian.h
# End Source File
# Begin Source File

SOURCE=.\extractionskills.h
# End Source File
# Begin Source File

SOURCE=.\fishing.h
# End Source File
# Begin Source File

SOURCE=.\gamedate.h
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\guild.h
# End Source File
# Begin Source File

SOURCE=.\house.h
# End Source File
# Begin Source File

SOURCE=.\html.h
# End Source File
# Begin Source File

SOURCE=.\inlines.h
# End Source File
# Begin Source File

SOURCE=.\item.h
# End Source File
# Begin Source File

SOURCE=.\itemid.h
# End Source File
# Begin Source File

SOURCE=.\items.h
# End Source File
# Begin Source File

SOURCE=.\jail.h
# End Source File
# Begin Source File

SOURCE=.\layer.h
# End Source File
# Begin Source File

SOURCE=.\logsystem.h
# End Source File
# Begin Source File

SOURCE=.\magic.h
# End Source File
# Begin Source File

SOURCE=.\map.h
# End Source File
# Begin Source File

SOURCE=.\menu.h
# End Source File
# Begin Source File

SOURCE=.\mount.h
# End Source File
# Begin Source File

SOURCE=.\msgboard.h
# End Source File
# Begin Source File

SOURCE=.\network.h
# End Source File
# Begin Source File

SOURCE=.\newbie.h
# End Source File
# Begin Source File

SOURCE=".\nox-wizard.h"
# End Source File
# Begin Source File

SOURCE=.\npcai.h
# End Source File
# Begin Source File

SOURCE=.\npcs.h
# End Source File
# Begin Source File

SOURCE=.\ntservice.h
# End Source File
# Begin Source File

SOURCE=.\nxw_utils.h
# End Source File
# Begin Source File

SOURCE=.\nxwcommn.h
# End Source File
# Begin Source File

SOURCE=.\nxwgui.h
# End Source File
# Begin Source File

SOURCE=.\object.h
# End Source File
# Begin Source File

SOURCE=.\oldmenu.h
# End Source File
# Begin Source File

SOURCE=.\osdefs.h
# End Source File
# Begin Source File

SOURCE=.\packets.h
# End Source File
# Begin Source File

SOURCE=.\particles.h
# End Source File
# Begin Source File

SOURCE=.\party.h
# End Source File
# Begin Source File

SOURCE=.\pointer.h
# End Source File
# Begin Source File

SOURCE=.\poll.h
# End Source File
# Begin Source File

SOURCE=.\race.h
# End Source File
# Begin Source File

SOURCE=.\raceability.h
# End Source File
# Begin Source File

SOURCE=.\racegender.h
# End Source File
# Begin Source File

SOURCE=.\racerequiredtype.h
# End Source File
# Begin Source File

SOURCE=.\racescriptentry.h
# End Source File
# Begin Source File

SOURCE=.\raceskill.h
# End Source File
# Begin Source File

SOURCE=.\racestartitem.h
# End Source File
# Begin Source File

SOURCE=.\racetokenizer.h
# End Source File
# Begin Source File

SOURCE=.\racetype.h
# End Source File
# Begin Source File

SOURCE=.\range.h
# End Source File
# Begin Source File

SOURCE=.\rcvpkg.h
# End Source File
# Begin Source File

SOURCE=.\regions.h
# End Source File
# Begin Source File

SOURCE=.\remadmin.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\sclinux.h
# End Source File
# Begin Source File

SOURCE=.\scp_parser.h
# End Source File
# Begin Source File

SOURCE=.\scripts.h
# End Source File
# Begin Source File

SOURCE=.\set.h
# End Source File
# Begin Source File

SOURCE=.\skills.h
# End Source File
# Begin Source File

SOURCE=.\sndpkg.h
# End Source File
# Begin Source File

SOURCE=.\spawn.h
# End Source File
# Begin Source File

SOURCE=.\speech.h
# End Source File
# Begin Source File

SOURCE=.\sregions.h
# End Source File
# Begin Source File

SOURCE=.\srvparms.h
# End Source File
# Begin Source File

SOURCE=.\target.h
# End Source File
# Begin Source File

SOURCE=.\targeting.h
# End Source File
# Begin Source File

SOURCE=.\telport.h
# End Source File
# Begin Source File

SOURCE=.\timers.h
# End Source File
# Begin Source File

SOURCE=.\timing.h
# End Source File
# Begin Source File

SOURCE=.\titles.h
# End Source File
# Begin Source File

SOURCE=.\tmpeff.h
# End Source File
# Begin Source File

SOURCE=.\trade.h
# End Source File
# Begin Source File

SOURCE=.\translation.h
# End Source File
# Begin Source File

SOURCE=.\trigger.h
# End Source File
# Begin Source File

SOURCE=.\typedefs.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# Begin Source File

SOURCE=.\walking.h
# End Source File
# Begin Source File

SOURCE=.\weight.h
# End Source File
# Begin Source File

SOURCE=.\worldmain.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
