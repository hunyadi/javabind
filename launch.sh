set -e

# Build the C++ library
cmake -B build -D JAVABIND_INTEGER_WIDENING_CONVERSION=ON
cmake --build build

# Emit Java function signatures
build\javabind_codegen codegen/signatures

# Compile and run the Java test application
find java -name "*.java" > build/sources.txt
mkdir -p jar
javac -d jar -cp java @build/sources.txt
java -Djava.library.path=build -cp jar -ea hu.info.hunyadi.test.TestJavaBind
