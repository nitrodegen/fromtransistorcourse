#!/bin/bash
arm-linux-gnueabi-as test.s  -o t;
arm-linux-gnueabi-objdump -d t;