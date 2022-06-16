import os,io,sys
import binascii
import struct
def little(num):
    return int(num).to_bytes(4,"little")
def sectionHeader(name,stype,flags,addr,offset,size,link,info,align,entsize):
    return little(name)+little(stype)+little(flags)+little(addr)+little(offset)+little(size)+little(link)+little(info)+little(align)+little(entsize)
def PHT(ptype,offset,vaddr,paddr,filesz,memsz,flags,align):
    return little(ptype)+little(offset)+little(vaddr)+little(paddr)+little(filesz)+little(memsz)+little(flags)+little(align)


def linker(rdf,otf):
    pheader=PHT(1,0,0x10000,0x10000,100,100,0x5,0x10000)

    # section header #0
    section_head0 = sectionHeader(0,0,0,0,0,0,0,0,0,0)
    # .text
    memaddr=0x10054 # vaddr and adding 64 for space
    textsec = sectionHeader(27,1,6,memaddr,84,16,0,0,4,0)

    # .arm_attributes
    armattr1 = sectionHeader(33,1879048195,0,0x0,100,18,0,0,1,0)
    # .symtab
    symtab1= sectionHeader(1,2,0,0,120,208,4,5,4,16)
    # .strtab
    strtab1= sectionHeader(9,3,0,0,328,66,0,0,1,0)
    # .shstrtab
    shstrtab1 = sectionHeader(17,3,0,0,394,49,0,0,1,0)

    #ELF STRUCTURE -> HEADER-> PHT -> SECTION TABLE (CLEAN) -> .text -> arm_attributes -> symtab -> strtab -> binary data
    #ARM7 Header representing little-endian and 32bit arch
    header=b'\x7fELF\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00(\x00\x01\x00\x00\x00T\x00\x01\x004\x00\x00\x00\xbc\x01\x00\x00\x00\x02\x00\x054\x00 \x00\x01\x00(\x00\x06\x00\x05\x00'

    #======== REVERSE ENGINEERING PURPOSES ============
    d = open(rdf,"rb")
    text =d.read()
    #print(text)
    text = text[::-1]
    #print(text)
    pheader=PHT(1,0,0x10000,0x10000,100+(100-len(text)),100+(100-len(text)) ,0x5,0x10000)

    textsec = sectionHeader(27,1,6,memaddr,84,16,0,0,4,0)

    tot = b'\xe3A\x11\x00\x00\x00aeabi\x00\x01\x07\x00\x00\x00\x08\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x5a\x00\x01\x00\x00\x00\x00\x00\x03\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x02\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\xf1\xff\x05\x00\x00\x5a\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x17\x00\x00\x00d\x00\x02\x00\x00\x00\x00\x00\x10\x00\x01\x00\x08\x00\x00\x00d\x00\x02\x00\x00\x00\x00\x00\x10\x00\x01\x00\x16\x00\x00\x00d\x00\x02\x00\x00\x00\x00\x00\x10\x00\x01\x00\'\x00\x00\x5a\x00\x01\x00\x00\x00\x00\x00\x10\x00\x01\x00"\x00\x00\x00d\x00\x02\x00\x00\x00\x00\x00\x10\x00\x01\x00.\x00\x00\x00d\x00\x02\x00\x00\x00\x00\x00\x10\x00\x01\x006\x00\x00\x00d\x00\x02\x00\x00\x00\x00\x00\x10\x00\x01\x00=\x00\x00\x00d\x00\x02\x00\x00\x00\x00\x00\x10\x00\x01\x00\x00out.o\x00$a\x00__bss_start__\x00__bss_end__\x00__bss_start\x00__end__\x00_edata\x00_end\x00\x00.symtab\x00.strtab\x00.shstrtab\x00.text\x00.ARM.attributes\x00\x00'
    tot = pheader+text+tot
    #print(tot)
    elf = header+tot+section_head0+textsec+armattr1+symtab1+strtab1+shstrtab1+text
    f = open(otf,"wb")
    f.write(elf)
    f.close()


def assembler(name,out):
    z = open(name,"r")
    data = z.read()
    lines = data.split("\n")
    linedata =[]
    for l in lines:
        if l != '\n' and l != '\t' and l !=  '' :
            linedata.append(l)

    regs=[["r0","0"],["r1","1"],["r2","2"],["r3","3"],["r4","4"],["r5","5"],["r6","6"],["r7","7"],["r8","8"],["r9","9"],["r10","a"],["r11","b"],["r12","c"],["pc","f"],["lr","e"],["sp","d"]]
    data=data.split("\n")
    raw=[]
    for x in data:
        if(len(x)>0):
            raw.append(x.replace("\t",""))

    opcodes=[]
    for x in raw:
        if "svc" in x:
            val = x.split("#")[1]
            fin = "ef"
            for i in range(6-len(val)):
                fin+="0"
            fin+=val   
            opcodes.append(fin)

        if "ldr" in x:
            fin = "e59f"
            x = x.split(" ")
            reg = x[1]
            for r in regs:
                if reg == r[0]:
                    fin+=r[1]

            flag = x[2].split(",=")[1]
            lom = ""
            stringval = ""
            for line,i in zip(linedata,range(len(linedata))):
                if(flag in line and "ldr" not in line):
                    lom = str(hex(i)).replace("0x","")

                    stringval = line.split(".asciz")[1]
            
                    stringval = stringval.replace("\"","")
                    
                    break
            for i in range(3-len(lom)):
                lom = "0"+lom
            fin+=lom
            f = open(lom,"w")
            f.write(stringval)
            f.close()
            opcodes.append(fin)

        if "mov" in x and "#" in x:
                fin="e3a0"
                #e3a 00 019
                x = x.split(" ")[1]
                x = x.split(",")
                for y in regs:
                    if x[0] == y[0]:
                        fin+=y[1]

                
                num = ""
                
                if('0x'  not in x[1]):
                    num = "0"+str(hex(int(x[1].replace("#","")))).replace("0x","")
                else:
                    num = "0"+str(int(x[1].replace("#",""))).replace("0x","")

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
            num = ""
            if('0x' not in x[1]):

                num = "0"+str(hex(int(x[1].replace("#","")))).replace("0x","")
            else:
                num = "0"+str(int(x[1].replace("#",""))).replace("0x","")
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
                num = ""
                if('0x' not in x[1]):

                    num = "0"+str(hex(int(x[1].replace("#","")))).replace("0x","")
                else:
                    num = "0"+str(int(x[1].replace("#",""))).replace("0x","")
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
                num = ""
                if('0x' not in x[1]):
                    num = "0"+str(hex(int(x[1].replace("#","")))).replace("0x","")
                else:
                    num = "0"+str(int(x[1].replace("#",""))).replace("0x","")
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
            num = ""
            if('0x' not in x[1]):

                num = "0"+str(hex(int(x[1].replace("#","")))).replace("0x","")
            else:
                num = "0"+str(int(x[1].replace("#",""))).replace("0x","")
            if(len(num) == 2):
                num = "0"+num
            elif len(num) == 1:
                num = "00"+num
            fin+=to+num
            opcodes.append(fin)

        if "push" in x:
            fin="e52d"
            reg = x.split("{")[1]
            reg = reg.replace("}","")
            for r in regs:
                if reg == r[0]:
                    fin+=r[1]
                    break
            fin+="004"
            opcodes.append(fin)
    readf = out
    f = open(readf,"wb")
    opcodes = opcodes[::-1]
    for i in range(len(opcodes)):
        f.write(bytes.fromhex(opcodes[i]))

    f.close()



if(len(sys.argv) <3):
    print("example: {sourcefile} -o {output name}")
    exit()

if(sys.argv[2] == "-o"):
    sourcefile = sys.argv[1]
    out = sys.argv[3]
    readf = sourcefile.split(".")[0]+".o"

    assembler(sourcefile,out)
    #linker(readf,out)

