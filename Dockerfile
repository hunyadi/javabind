FROM ubuntu:latest

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && \
    apt-get -y install gcc g++ cmake openjdk-8-jdk && \
    rm -rf /var/lib/apt/lists/*

ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64
COPY include/ include/
COPY test/ test/
RUN g++ -fPIC -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux -Iinclude -shared -Wall -Werror test/javabind.cpp
