@echo off
md tmp
cd tmp
if %errorlevel% neq 0 exit /b %errorlevel%

set /p Build=<..\buildnr.txt
set /a BUILD=%BUILD%+1
echo %BUILD% > buildnr.txt

rem clean first
del release\*

rem Compile program
set PATH=%PATH%;E:\Qt\Tools\mingw492_32\bin
E:\Qt\5.5\mingw492_32\bin\qmake ..\..\ -r
if %errorlevel% neq 0 exit /b %errorlevel%
mingw32-make
if %errorlevel% neq 0 exit /b %errorlevel%

cd ..

rem Make installer
"C:\Program Files (x86)\NSIS\makensis.exe" installer_x86_mingw492_32.nsis
if %errorlevel% neq 0 exit /b %errorlevel%

echo %BUILD% > buildnr.txt