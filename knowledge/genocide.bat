@ECHO OFF
FOR %%d IN (asheep cstrike dmc gearbox tfc valve) DO (
   CD %%d
   IF EXIST *.brn DEL *.brn > NUL
   IF EXIST *.nav DEL *.nav > NUL
   IF EXIST *.map DEL *.map > NUL
   CD ..
)
ECHO All bots exterminated, sir.
