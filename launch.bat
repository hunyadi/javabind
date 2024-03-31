@echo off
rem Compiles and runs the Java test application
rem rmdir /s /q jar
javac -d jar -cp java java\hu\info\hunyadi\test\TestJavaBind.java
if errorlevel 1 exit /b %ERRORLEVEL%
java -Djava.library.path=build\Debug -cp jar -ea hu.info.hunyadi.test.TestJavaBind
if errorlevel 1 exit /b %ERRORLEVEL%
