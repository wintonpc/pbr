#!/bin/bash

protoc -I=protos --java_out=pbtest/src/main/java protos/test.proto
protoc -I=protos --pbr_out=. --plugin=../bin/protoc-gen-pbr protos/test.proto
