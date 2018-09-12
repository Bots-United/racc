# Microsoft Developer Studio Project File - Name="cstrike" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=cstrike - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cstrike.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cstrike.mak" CFG="cstrike - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cstrike - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/SDKSrc/Public/dlls", NVGBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G6 /MT /W3 /WX /GX /ZI /Od /I "..\common" /I "../../../metamod/metamod" /I "../../../devtools/hlsdk-2.3/multiplayer/cl_dll" /I "../../../devtools/hlsdk-2.3/multiplayer/common" /I "../../../devtools/hlsdk-2.3/multiplayer/dlls" /I "../../../devtools/hlsdk-2.3/multiplayer/engine" /I "../../../devtools/hlsdk-2.3/multiplayer/game_shared" /I "../../../devtools/hlsdk-2.3/multiplayer/pm_shared" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "RACC" /D "CSTRIKE_DLL" /Fr /Fp"./Release/racc.pch" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o".\Release/racc.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /incremental:yes /debug /machine:I386 /def:"..\common\racc.def" /out:".\Release/racc.dll"
# SUBTRACT LINK32 /pdb:none /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PreLink_Desc=Updating resources...
PreLink_Cmds=makeres -d"Rational Autonomous Cybernetic Commandos AI" >nul racc_mm.rc	rc racc_mm.rc 	move racc_mm.RES Release
# End Special Build Tool
# Begin Target

# Name "cstrike - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "c;cpp;def"
# Begin Group "navmesh"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\bmpfile.cpp
# End Source File
# Begin Source File

SOURCE=..\common\dxffile.cpp
# End Source File
# Begin Source File

SOURCE=..\common\mapdata.cpp
# End Source File
# Begin Source File

SOURCE=..\common\pathmachine.cpp
# End Source File
# End Group
# Begin Group "interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\client.cpp
# End Source File
# Begin Source File

SOURCE=.\interface.cpp
# End Source File
# End Group
# Begin Group "misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\console.cpp
# End Source File
# Begin Source File

SOURCE=..\common\display.cpp
# End Source File
# Begin Source File

SOURCE=..\common\lrand.cpp
# End Source File
# Begin Source File

SOURCE=..\common\math.cpp
# End Source File
# Begin Source File

SOURCE=..\common\mfile.cpp
# End Source File
# Begin Source File

SOURCE=.\racc_mm.rc
# End Source File
# Begin Source File

SOURCE=..\common\util.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\bot.cpp
# End Source File
# Begin Source File

SOURCE=..\common\bot_body.cpp
# End Source File
# Begin Source File

SOURCE=..\common\bot_chat.cpp
# End Source File
# Begin Source File

SOURCE=.\bot_cognition.cpp
# End Source File
# Begin Source File

SOURCE=.\bot_combat.cpp
# End Source File
# Begin Source File

SOURCE=..\common\bot_ears.cpp
# End Source File
# Begin Source File

SOURCE=..\common\bot_eyes.cpp
# End Source File
# Begin Source File

SOURCE=..\common\bot_navigation.cpp
# End Source File
# Begin Source File

SOURCE=.\bot_start.cpp
# End Source File
# Begin Source File

SOURCE=..\common\weapons.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\mod_specific.h
# End Source File
# Begin Source File

SOURCE=..\common\racc.h
# End Source File
# End Group
# Begin Group "Config files"

# PROP Default_Filter ".cfg"
# Begin Source File

SOURCE=..\..\knowledge\cstrike\footstepsounds.cfg
# End Source File
# Begin Source File

SOURCE=..\..\knowledge\cstrike\game.cfg
# End Source File
# Begin Source File

SOURCE=..\..\knowledge\cstrike\likelevels.cfg
# End Source File
# Begin Source File

SOURCE=..\..\knowledge\cstrike\playerbones.cfg
# End Source File
# Begin Source File

SOURCE=..\..\knowledge\cstrike\ricochetsounds.cfg
# End Source File
# Begin Source File

SOURCE=..\..\knowledge\cstrike\sounds.cfg
# End Source File
# Begin Source File

SOURCE=..\..\knowledge\cstrike\usermsgs.cfg
# End Source File
# Begin Source File

SOURCE=..\..\knowledge\cstrike\weapons.cfg
# End Source File
# End Group
# End Target
# End Project
