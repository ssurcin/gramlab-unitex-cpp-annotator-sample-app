@echo off

echo Ensuring the Unitex C++ annotator dependency is up to date through Maven.
echo Output directory = %1
echo Profile = %2
mvn -P%2 -Doutdir=%1 dependency:unpack

echo Removing linguistic resources from directory %1
rd /s /q %1\unitex

