@echo off
setlocal

rem Build the C++ library
cmake -B build -D JAVABIND_INTEGER_WIDENING_CONVERSION=ON
if errorlevel 1 exit /b %ERRORLEVEL%
cmake --build build
if errorlevel 1 exit /b %ERRORLEVEL%

rem Emit Java function signatures
build\Debug\javabind_codegen.exe codegen\signatures

rem Compile and run the Java test application
rem rmdir /s /q jar
dir /s /b java\*.java > build\sources.txt
javac -d jar -cp java @build\sources.txt
if errorlevel 1 exit /b %ERRORLEVEL%
java -Djava.library.path=build\Debug -cp jar -ea hu.info.hunyadi.test.TestJavaBind
if errorlevel 1 exit /b %ERRORLEVEL%
