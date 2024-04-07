@echo off
rem Compiles and runs the Java test application
rem rmdir /s /q jar
dir /s /b *.java > build\sources.txt
javac -d jar -cp java @build\sources.txt
if errorlevel 1 exit /b %ERRORLEVEL%
java -Djava.library.path=build\Debug -cp jar -ea hu.info.hunyadi.test.TestJavaBind
if errorlevel 1 exit /b %ERRORLEVEL%
