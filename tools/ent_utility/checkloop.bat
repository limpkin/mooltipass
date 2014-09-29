@ECHO OFF
:loop
  ent.exe  ..\python_comms\randombytes.bin
  timeout /t 2
goto loop