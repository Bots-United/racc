@ECHO OFF

:: Removal script for RACC

:valvedetect
IF EXIST "..\valve\liblist.gam" GOTO valve
:_valvedetect

:gearboxdetect
IF EXIST "..\gearbox\liblist.gam" GOTO gearbox
:_gearboxdetect

:asheepdetect
IF EXIST "..\asheep\liblist.gam" GOTO asheep
:_asheepdetect

:dmcdetect
IF EXIST "..\dmc\liblist.gam" GOTO dmc
:_dmcdetect

:tfcdetect
IF EXIST "..\tfc\liblist.gam" GOTO tfc
:_tfcdetect

:cstrikedetect
IF EXIST "..\cstrike\liblist.gam" GOTO cstrike
:_cstrikedetect

GOTO quit


:valve
ECHO Removing RACC in Standard Half-Life...
SET MODDIR="valve"
IF EXIST "install\%MODDIR%_liblist.prv" COPY "install\%MODDIR%_liblist.prv" "..\%MODDIR%\liblist.gam" > NUL
IF EXIST "install\%MODDIR%_settings.prv" COPY "install\%MODDIR%_settings.prv" "..\%MODDIR%\settings.scr" > NUL
IF EXIST "install\%MODDIR%_kb_act.prv" COPY "install\%MODDIR%_kb_act.prv" "..\%MODDIR%\gfx\shell\kb_act.lst" > NUL
IF EXIST "install\%MODDIR%_commandmenu.prv" COPY "install\%MODDIR%_commandmenu.prv" "..\%MODDIR%\commandmenu.txt" > NUL
DEL "install\%MODDIR%_*.prv" > NUL
GOTO _valvedetect

:gearbox
ECHO Removing RACC in Opposing Force...
SET MODDIR="gearbox"
IF EXIST "install\%MODDIR%_liblist.prv" COPY "install\%MODDIR%_liblist.prv" "..\%MODDIR%\liblist.gam" > NUL
IF EXIST "install\%MODDIR%_settings.prv" COPY "install\%MODDIR%_settings.prv" "..\%MODDIR%\settings.scr" > NUL
IF EXIST "install\%MODDIR%_kb_act.prv" COPY "install\%MODDIR%_kb_act.prv" "..\%MODDIR%\gfx\shell\kb_act.lst" > NUL
IF EXIST "install\%MODDIR%_commandmenu.prv" COPY "install\%MODDIR%_commandmenu.prv" "..\%MODDIR%\commandmenu.txt" > NUL
DEL "install\%MODDIR%_*.prv" > NUL
GOTO _gearboxdetect

:asheep
ECHO Removing RACC in Azure Sheep...
SET MODDIR="asheep"
IF EXIST "install\%MODDIR%_liblist.prv" COPY "install\%MODDIR%_liblist.prv" "..\%MODDIR%\liblist.gam" > NUL
IF EXIST "install\%MODDIR%_settings.prv" COPY "install\%MODDIR%_settings.prv" "..\%MODDIR%\settings.scr" > NUL
IF EXIST "install\%MODDIR%_kb_act.prv" COPY "install\%MODDIR%_kb_act.prv" "..\%MODDIR%\gfx\shell\kb_act.lst" > NUL
IF EXIST "install\%MODDIR%_commandmenu.prv" COPY "install\%MODDIR%_commandmenu.prv" "..\%MODDIR%\commandmenu.txt" > NUL
DEL "install\%MODDIR%_*.prv" > NUL
GOTO _asheepdetect

:dmc
ECHO Removing RACC in Deathmatch Classic...
SET MODDIR="dmc"
IF EXIST "install\%MODDIR%_liblist.prv" COPY "install\%MODDIR%_liblist.prv" "..\%MODDIR%\liblist.gam" > NUL
IF EXIST "install\%MODDIR%_settings.prv" COPY "install\%MODDIR%_settings.prv" "..\%MODDIR%\settings.scr" > NUL
IF EXIST "install\%MODDIR%_kb_act.prv" COPY "install\%MODDIR%_kb_act.prv" "..\%MODDIR%\gfx\shell\kb_act.lst" > NUL
IF EXIST "install\%MODDIR%_commandmenu.prv" COPY "install\%MODDIR%_commandmenu.prv" "..\%MODDIR%\commandmenu.txt" > NUL
DEL "install\%MODDIR%_*.prv" > NUL
GOTO _dmcdetect

:tfc
ECHO Removing RACC in Team Fortress Classic...
SET MODDIR="tfc"
IF EXIST "install\%MODDIR%_liblist.prv" COPY "install\%MODDIR%_liblist.prv" "..\%MODDIR%\liblist.gam" > NUL
IF EXIST "install\%MODDIR%_settings.prv" COPY "install\%MODDIR%_settings.prv" "..\%MODDIR%\settings.scr" > NUL
IF EXIST "install\%MODDIR%_kb_act.prv" COPY "install\%MODDIR%_kb_act.prv" "..\%MODDIR%\gfx\shell\kb_act.lst" > NUL
IF EXIST "install\%MODDIR%_commandmenu.prv" COPY "install\%MODDIR%_commandmenu.prv" "..\%MODDIR%\commandmenu.txt" > NUL
DEL "install\%MODDIR%_*.prv" > NUL
GOTO _tfcdetect

:cstrike
ECHO Removing RACC in Counter-Strike...
SET MODDIR="cstrike"
IF EXIST "install\%MODDIR%_liblist.prv" COPY "install\%MODDIR%_liblist.prv" "..\%MODDIR%\liblist.gam" > NUL
IF EXIST "install\%MODDIR%_settings.prv" COPY "install\%MODDIR%_settings.prv" "..\%MODDIR%\settings.scr" > NUL
IF EXIST "install\%MODDIR%_kb_act.prv" COPY "install\%MODDIR%_kb_act.prv" "..\%MODDIR%\gfx\shell\kb_act.lst" > NUL
IF EXIST "install\%MODDIR%_commandmenu.prv" COPY "install\%MODDIR%_commandmenu.prv" "..\%MODDIR%\commandmenu.txt" > NUL
DEL "install\%MODDIR%_*.prv" > NUL
GOTO _cstrikedetect


:quit
ECHO Finished.
:: Garbage Cleanup
IF EXIST INSTALL.PIF DEL INSTALL.PIF > NUL
