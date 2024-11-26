set -e
# Builds the C++ library
cmake -B build -D JAVABIND_INTEGER_WIDENING_CONVERSION=ON
cmake --build build

# Compiles and runs the Java test application
find java -name "*.java" > build/sources.txt
mkdir -p jar
javac -d jar -cp java @build/sources.txt
java -Djava.library.path=build -cp jar -ea hu.info.hunyadi.test.TestJavaBind
