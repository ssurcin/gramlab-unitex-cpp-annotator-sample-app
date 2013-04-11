@echo off
rem %1 = project directory (source files)
rem %2 = destination directory (executables)
rem %3 = build mode (Debug or Release)

if "%3"=="" goto missingmode
if "%3"=="Debug" goto copydbglib 

echo
echo Copy release libraries into %2
copy %1\lib\unitex-uima-cpp\*.* %2
goto copyres

:copydbglib
echo
echo Copy debug libraries into %2
copy %1\lib-dbg\*.* %2

:copyres
echo
echo Copy linguistic resources into %2
xcopy /s %1\resources %2

:copydesc
echo
echo Copy descriptors into %2
copy %1\desc\unitex-uima-cpp\*.xml %2
echo Override UnitexAnnotatorCpp descriptor with our own parameters
copy %1\desc\UnitexAnnotatorCpp.xml %2
goto end

:missingmode
echo The build mode (3rd argument) is missing!

:end
