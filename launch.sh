set -e
find java -name "*.java" > build/sources.txt
javac -d jar -cp java @build/sources.txt
java -Djava.library.path=build -cp jar -ea hu.info.hunyadi.test.TestJavaBind
