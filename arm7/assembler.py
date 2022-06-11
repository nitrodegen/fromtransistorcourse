import opcode
import os,io,sys
import struct
import binascii

if(len(sys.argv) < 2):
    print("provide the file to assemble!")
    exit()
z = open(sys.argv[1],"r")
data = z.read()
regs=[["r0","0"],["r1","1"],["r2","2"],["r3","3"],["r4","4"],["r5","5"],["r6","6"],["r7","7"],["r8","8"],["r9","9"],["r10","a"],["r11","b"],["r12","c"],["pc","f"],["lr","e"],["sp","d"]]
data=data.split("\n")
raw=[]
for x in data:
    if(len(x)>0):
        raw.append(x.replace("\t",""))

opcodes=[]
for x in raw:
    if "mov" in x and "#" in x:
        fin="e3a0"
        #e3a 00 019
        x = x.split(" ")[1]
        x = x.split(",")
        for y in regs:
            if x[0] == y[0]:
                fin+=y[1]

        num = str(hex(int(x[1].replace("#","")))).replace("0x","")
        if(len(num) == 1):
            fin+="00"+num
        elif(len(num) == 2):
            fin+="0"+num
        else:
            fin+=num

        
        opcodes.append(fin)

    if "mov" in x and "#" not in x:
        #e1a 02 0 03
        #op  to 0 from
        fin="e1a"
        x = x.split(" ")[1]
        x = x.split(",")
        f = "0"+x[0][1]
        to = "0"+x[1][1]
        fin+=f+"0"+to
        opcodes.append(fin)

    if "sub" in x and "#" in x:
        #e24 11 013        sub     r1,  #19
        fin = "e24"
        x = x.split(" ")[1]
        x = x.split(",") 
        num = "0"+str(hex(int(x[1].replace("#","")))).replace("0x","")
        if(len(num) == 2):
            num = "0"+num
        if(len(num) == 1):
            num = "00"+num
        r = x[0][1]+x[0][1]
        fin+=r+num
        opcodes.append(fin)
    if "sub" in x and "#" not in x:
        fin="e04"
        x = x.split(" ")[1]
        x =  x.split(",")
        to = x[0][1]
        if(len(to) == 1):
            to = "0"+to
        num = str(hex(int(x[1][1]))).replace("0x","")
        if(len(num) == 2):
            num = "0"+num
        elif len(num) == 1:
            num = "00"+num
        fin+=to+num
        opcodes.append(fin)     
    if "add" in x and "#" in x:
        #e24 11 013        sub     r1,  #19
        fin = "e28"
        x = x.split(" ")[1]
        x = x.split(",") 
        num = "0"+str(hex(int(x[1].replace("#","")))).replace("0x","")
        if(len(num) == 2):
            num = "0"+num
        if(len(num) == 1):
            num = "00"+num
        r = x[0][1]+x[0][1]
        fin+=r+num
        opcodes.append(fin)
    if "add" in x and "#" not in x:
        fin="e08"
        x = x.split(" ")[1]
        x =  x.split(",")
        to = x[0][1]
        if(len(to) == 1):
            to = "0"+to
        num = str(hex(int(x[1][1]))).replace("0x","")
        if(len(num) == 2):
            num = "0"+num
        elif len(num) == 1:
            num = "00"+num
        fin+=to+num
        opcodes.append(fin)        
        
    if "push" in x:
        fin="e52d"
        x = x.split("{")
        x=x[1].replace("}","")
        x = str(hex(int(x[1]))).replace("0x","")
        fin+=x+"004"
        opcodes.append(fin)


f = open("out.o","wb")
for i in range(len(opcodes)):
    f.write(bytes.fromhex(opcodes[i]))
f.close()


