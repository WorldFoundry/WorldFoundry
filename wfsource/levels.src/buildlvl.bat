@echo off

rem ***** "buildlvl.bat"
rem ***** Copyright 1997 Recombinant Limited.  All Rights Reserved.
rem ***** by William B. Norris IV

cd >files.lst
dir /s /b *.max >>files.lst
..\..\bin\prep ..\assetdir.prp assetdir.bat
call assetdir.bat
rem dir /S /B *.max >>files.lst
rem dir /S /B *.iff >>files.lst
rem dir /S /B *.map >>files.lst
rem dir /S /B *.wav >>files.lst

if "%1" == "" goto env_var
..\..\bin\perl ..\makelvl.pl %1 <files.lst
goto end

:env_var
..\..\bin\perl ..\makelvl.pl %WF_TARGET <files.lst

:end
