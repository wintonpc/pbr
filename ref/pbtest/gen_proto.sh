#!/bin/bash

protoc -I=../protos --java_out=src/main/java ../protos/test.proto
