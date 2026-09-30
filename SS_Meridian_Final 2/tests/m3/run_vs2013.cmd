@echo off
setlocal
cd /d "%~dp0\.."
if not defined VS120COMNTOOLS (
 echo Visual Studio 2013 C++ tools not found.
 exit /b 1
)
call "%VS120COMNTOOLS%..\..\VC\vcvarsall.bat" x86
if errorlevel 1 exit /b 1
if not exist "tests\bin" mkdir "tests\bin"
cl /nologo /EHsc /W4 /MT /Isrc tests\Mission3Tests.cpp src\Mission3.cpp src\Mission3Config.cpp /Fe:tests\bin\Mission3Tests.exe /Fo:tests\bin\
if errorlevel 1 exit /b 1
tests\bin\Mission3Tests.exe
exit /b %ERRORLEVEL%
