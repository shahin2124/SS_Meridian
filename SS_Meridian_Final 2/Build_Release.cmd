@echo off
setlocal
cd /d "%~dp0"
if defined VS120COMNTOOLS call "%VS120COMNTOOLS%..\..\VC\vcvarsall.bat" x86
where msbuild >nul 2>nul
if errorlevel 1 (
 echo Open a Visual Studio 2013 x86 Native Tools Command Prompt and run this file again.
 exit /b 1
)
msbuild SS_Meridian.sln /m /t:Build /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v120
if errorlevel 1 exit /b 1
echo Game: %~dp0bin\Release\SS_Meridian.exe
