#!/bin/bash
g++ arm7.cpp -o arm7 -fno-stack-protector -z execstack;
./arm7 out.o;
rm out.o;
rm arm7;
