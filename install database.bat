@REM Hello World
@echo --- Database installation ---
time /T
@echo.

@set ZIPFILENAME=MuehleWin_Database_ver1.0.zip
@set SEVENZIP_PATH="C:\Program Files\7-Zip\7z.exe"

@REM Dat file available?
@iF NOT EXIST preCalculatedVars.dat @goto datFileNotFound
@iF NOT EXIST plyInfo.dat           @goto datFileNotFound
@iF NOT EXIST shortKnotValue.dat    @goto datFileNotFound

@REM Are dat files already compressed?
@compact *.dat | findstr /c:"380928 = "
@IF %ERRORLEVEL%==1 @goto compressAgain
@echo --- Database files are already compressed. No re-compression necessary. ---
@goto end

:datFileNotFound
@REM Zip file available?
@iF EXIST %ZIPFILENAME% @goto zipFileFound
@echo --- ERROR: The file database.zip does not exist in the current directory. ---
@goto end

:zipFileFound
@REM 7z available?
@iF EXIST %SEVENZIP_PATH% @goto sevenZipFound
@echo --- ERROR: Please install 7z first. ---
@goto end

:sevenZipFound
@REM Activate normal compression for new files in this folder.
@compact /c /q . >nul
@echo.

@REM Unzip database files
@echo --- Unzip file database.zip into compressed directory... ---
@call %SEVENZIP_PATH% x %ZIPFILENAME%
@del %ZIPFILENAME%
time /T
@echo.

:compressAgain
REM @REM Compact files in new folder
REM @echo --- Compress files again... ---
REM @compact /c /f /exe:LZX *.dat
REM @echo.

:success
@REM Everything was fine
@echo --- The re-compression of the database was successful. ---

:end
time /T