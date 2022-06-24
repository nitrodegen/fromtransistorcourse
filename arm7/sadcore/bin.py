#!/opt/homebrew/bin/python3

import binascii 
import os,io,sys

num = int(sys.argv[1],base=16)
num = str(bin(num)).replace("0b","")
f = "" 
for i in range(0,32-len(num)):
  f+="0"

f +=num
print(f)

