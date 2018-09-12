@ECHO OFF

:: Install script for RACC

:modelsdetect
IF NOT EXIST "..\valve\models\player\raccmodels.txt" GOTO models
:_modelsdetect

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

:models
IF NOT EXIST "additional\models.exe" GOTO _modelsdetect
ECHO Installing additional player models...
CD "..\valve\models\player"
"..\..\..\racc\additional\models.exe"
ECHO RACC player models installed > "raccmodels.txt"
CD "..\..\..\racc"
GOTO _modelsdetect

:valve
ECHO Installing RACC in Standard Half-Life...
SET MODDIR="valve"
IF NOT EXIST "install\%MODDIR%_liblist.prv" COPY "..\%MODDIR%\liblist.gam" "install\%MODDIR%_liblist.prv" > NUL
IF NOT EXIST "install\%MODDIR%_kb_act.prv" COPY "..\%MODDIR%\gfx\shell\kb_act.lst" "install\%MODDIR%_kb_act.prv" > NUL
IF NOT EXIST "install\%MODDIR%_settings.prv" COPY "..\%MODDIR%\settings.scr" "install\%MODDIR%_settings.prv" > NUL
"release\install\release\patch.exe" %MODDIR%
GOTO _valvedetect

:gearbox
ECHO Installing RACC in Opposing Force...
SET MODDIR="gearbox"
IF NOT EXIST "install\%MODDIR%_liblist.prv" COPY "..\%MODDIR%\liblist.gam" "install\%MODDIR%_liblist.prv" > NUL
IF NOT EXIST "install\%MODDIR%_kb_act.prv" COPY "..\%MODDIR%\gfx\shell\kb_act.lst" "install\%MODDIR%_kb_act.prv" > NUL
IF NOT EXIST "install\%MODDIR%_settings.prv" COPY "..\%MODDIR%\settings.scr" "install\%MODDIR%_settings.prv" > NUL
"release\install\release\patch.exe" %MODDIR%
GOTO _gearboxdetect

:asheep
ECHO Installing RACC in Azure Sheep...
SET MODDIR="asheep"
IF NOT EXIST "install\%MODDIR%_liblist.prv" COPY "..\%MODDIR%\liblist.gam" "install\%MODDIR%_liblist.prv" > NUL
IF NOT EXIST "install\%MODDIR%_kb_act.prv" COPY "..\%MODDIR%\gfx\shell\kb_act.lst" "install\%MODDIR%_kb_act.prv" > NUL
IF NOT EXIST "install\%MODDIR%_settings.prv" COPY "..\%MODDIR%\settings.scr" "install\%MODDIR%_settings.prv" > NUL
"release\install\release\patch.exe" %MODDIR%
GOTO _asheepdetect

:dmc
ECHO Installing RACC in Deathmatch Classic...
SET MODDIR="dmc"
IF NOT EXIST "install\%MODDIR%_liblist.prv" COPY "..\%MODDIR%\liblist.gam" "install\%MODDIR%_liblist.prv" > NUL
IF NOT EXIST "install\%MODDIR%_kb_act.prv" COPY "..\%MODDIR%\gfx\shell\kb_act.lst" "install\%MODDIR%_kb_act.prv" > NUL
IF NOT EXIST "install\%MODDIR%_settings.prv" COPY "..\%MODDIR%\settings.scr" "install\%MODDIR%_settings.prv" > NUL
"release\install\release\patch.exe" %MODDIR%
GOTO _dmcdetect

:tfc
ECHO Installing RACC in Team Fortress Classic...
SET MODDIR="tfc"
IF NOT EXIST "install\%MODDIR%_liblist.prv" COPY "..\%MODDIR%\liblist.gam" "install\%MODDIR%_liblist.prv" > NUL
IF NOT EXIST "install\%MODDIR%_kb_act.prv" COPY "..\%MODDIR%\gfx\shell\kb_act.lst" "install\%MODDIR%_kb_act.prv" > NUL
IF NOT EXIST "install\%MODDIR%_commandmenu.prv" COPY "..\%MODDIR%\commandmenu.txt" "install\%MODDIR%_commandmenu.prv" > NUL
IF NOT EXIST "install\%MODDIR%_settings.prv" COPY "..\%MODDIR%\settings.scr" "install\%MODDIR%_settings.prv" > NUL
"release\install\release\patch.exe" %MODDIR%
GOTO _tfcdetect

:cstrike
ECHO Installing RACC in Counter-Strike...
SET MODDIR="cstrike"
IF NOT EXIST "install\%MODDIR%_liblist.prv" COPY "..\%MODDIR%\liblist.gam" "install\%MODDIR%_liblist.prv" > NUL
IF NOT EXIST "install\%MODDIR%_kb_act.prv" COPY "..\%MODDIR%\gfx\shell\kb_act.lst" "install\%MODDIR%_kb_act.prv" > NUL
IF NOT EXIST "install\%MODDIR%_commandmenu.prv" COPY "..\%MODDIR%\commandmenu.txt" "install\%MODDIR%_commandmenu.prv" > NUL
IF NOT EXIST "install\%MODDIR%_settings.prv" COPY "..\%MODDIR%\settings.scr" "install\%MODDIR%_settings.prv" > NUL
"release\install\release\patch.exe" %MODDIR%
GOTO _cstrikedetect


:quit
ECHO Finished.
:: Garbage Cleanup
IF EXIST INSTALL.PIF DEL INSTALL.PIF > NUL
