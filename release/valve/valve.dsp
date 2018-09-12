# Microsoft Developer Studio Project File - Name="valve" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=valve - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "valve.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "valve.mak" CFG="valve - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "valve - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
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
# ADD CPP /nologo /G6 /MT /W3 /WX /GX /ZI /Od /I "..\common" /I "../../../devtools/sdk/Single-Player Source/cl_dll" /I "../../../devtools/sdk/Single-Player Source/common" /I "../../../devtools/sdk/Single-Player Source/dlls" /I "../../../devtools/sdk/Single-Player Source/engine" /I "../../../devtools/sdk/Single-Player Source/game_shared" /I "../../../devtools/sdk/Single-Player Source/pm_shared" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "RACC" /D "VALVE_DLL" /Fr /Fp"./Release/racc.pch" /YX /FD /c
# SUBTRACT CPP /X
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
# SUBTRACT LINK32 /pdb:none
# Begin Target

# Name "valve - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "c;cpp;def"
# Begin Source File

SOURCE=..\common\bmpfile.cpp
# End Source File
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

SOURCE=.\bot_client.cpp
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

SOURCE=.\dll.cpp
# End Source File
# Begin Source File

SOURCE=..\common\dxffile.cpp
# End Source File
# Begin Source File

SOURCE=.\engine.cpp
# End Source File
# Begin Source File

SOURCE=.\linkfunc.cpp
# End Source File
# Begin Source File

SOURCE=..\common\mapdata.cpp
# End Source File
# Begin Source File

SOURCE=..\common\mfile.cpp
# End Source File
# Begin Source File

SOURCE=..\common\pathmachine.cpp
# End Source File
# Begin Source File

SOURCE=..\common\util.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\mod_specific.h
# End Source File
# Begin Source File

SOURCE=..\common\racc.h
# End Source File
# End Group
# End Target
# End Project