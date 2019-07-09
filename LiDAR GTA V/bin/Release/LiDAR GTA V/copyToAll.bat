@ECHO off
echo Copying...

for /D %%a in ("*") do xcopy /y /d colorize.py "%%a\"
for /D %%a in ("*") do xcopy /y /d colorize.bat "%%a\"

echo Done!