@echo off

echo Ensuring the Unitex C++ annotator dependency is up to date through Maven
mvn -Doutdir=%1 dependency:unpack

echo Removing linguistic resources from directory %1
rd /s /q %1\unitex

