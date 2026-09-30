@echo off
cd /d "%~dp0"
if not exist bin mkdir bin
cl /nologo /EHsc /W4 /MT /D_CRT_SECURE_NO_WARNINGS LogicTests.cpp /Fe:bin\LogicTests.exe /Fo:bin\LogicTests.obj
if errorlevel 1 exit /b 1
bin\LogicTests.exe
if errorlevel 1 exit /b 1
cl /nologo /EHsc /W4 /MT CombatSimulation.cpp /Fe:bin\CombatSimulation.exe /Fo:bin\CombatSimulation.obj
if errorlevel 1 exit /b 1
bin\CombatSimulation.exe
exit /b %errorlevel%
