#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include <sstream>
#include <sys/mman.h>
#define stop exit(1) // easier than always typing exit(number)
using namespace std;
/*
    geohot's course tells me to create ARM7 using verilog. but im really lazy to learn verilog, will do it on summer and finish those verilog stuff in course.
    for now , just an emulator (more like translator)
*/
//this emulator should be able to just translate all the ARM7 directly to X86 code ,and show me the path of stuff that happened during "execution" of code
// === REGISTERS === 
vector<signed int> registers(17);
//r0-r12 , pc ,sp,lr,apsr

vector<string>STACK;
vector<long int>RAM;

// == REVERSE ENGINEERING ARM7 ASSEMBLER == 
//e3a00013       mov     r0, #19
//e3a01013       mov     r1, #19
// MOV NUMBER -> opcode e3a0{regnum}0{number in hex}

//lets make this pretty simple , so lets just have , push , mov , sub, add and interrupt 


vector<pair<string,string> >opcodes={
    //x in push is representing the number of the register, will be done with .replace in strings! 
    {"movreg","e1a"},{"subnum","e24"},{"subreg","e04"},{"push","e52"},{"addnum","e28"},{"addreg","e08"},{"movnum","e3a"}
};

bool debug=true;
vector<string>executePhase;
void debugstat()
{
    cout<<"\n*********** DEBUGGING = ON ************"<<endl;
    for(int i =0;i<registers.size();i++){
       if(i< 14){
        cout<<"R"<<i<<"="<<registers[i]<<",";    
       }
       else{
            cout<<"| pc->"<<registers[14]<<" | sp-> "<<registers[15]<<endl;
       }
    }
    if(STACK.size() >0){
    cout<<"stack data->";
    for(int i =0;i<STACK.size();i++){
        if(i < STACK.size()-1){
            cout<<STACK[i]<<" | ";
        }
        else{
            cout<<STACK[i]<<endl;
        }
    }
    }
    cout<<"\n************************************** "<<endl;
}
int main(int argc,char *argv[]){
    if(argc != 2){
        cout<<"please give me the binary i can examine dude."<<endl;
        exit(1);
    }

    string filename = argv[1];
    unsigned char h[2];

    vector<string>rawdata;
    FILE *fd = fopen(filename.c_str(),"r");
    int c =0 ;
    while(true){
        int re = fread(h,sizeof(unsigned char),1,fd);
        if(re<=0){
            break;
        }
        char *b = (char*)malloc(sizeof(unsigned char*)*2);
        sprintf(b,"%02x",h[0]);
        string g = b;
        rawdata.push_back(g);
        c++;
        free(b);
    }
    stringstream ss;

    ///vector<string>rev  (rawdata.rbegin(),rawdata.rend());
    string faf;
    for(auto& coc:rawdata){
        faf+=coc;
    }
    vector<string>data;
    for(int i =0;i<faf.length();i+=8){
        data.push_back(faf.substr(i,8));
    }
    int sp = faf.length()/8;
    if (faf.length() % sp != 0)
    {
        data.push_back(faf.substr(sp * 8));
    }

    vector<string>dat = data;
    //this is where comparison starts
    for(int i =0;i<dat.size();i++){
        string th = dat[i];
        string bytecode;
        bytecode+=th[0];
        bytecode+=th[1];
        bytecode+=th[2];
        ss.clear();  
        for(const auto& op:opcodes){
            string opcode = op.second;
            string label = op.first;
    
            if(bytecode == opcode){
                
                if(label == "movreg"){
                    // e1a 02 0 03
                    // op  to 0 from
                    string to1,from1;
                    ss.clear();
                    ss<<th[3]<<th[4];
                    ss>>to1;
                    ss.clear();
                    ss<<th[5]<<th[6]<<th[7];
                    ss>>from1;
                    ss.clear();
                    executePhase.push_back("movr:"+from1+"to"+to1);
                }
                else if(label == "movnum"){
                    //e3a 00 013
                    //op reg num
                    string to;
                    int num;
                    ss.clear();
                    ss<<th[3]<<th[4];
                    ss>>to;
                    ss.clear();
                    ss<<th[5]<<th[6]<<th[7];
                    ss>>hex>>num;
                    ss.clear();
                    cout.clear();

                    string nn = to_string(num);
                    executePhase.push_back("movn:"+nn+"to"+to);
                }
                else if(label =="subnum"){
                    //e24 11 013        sub     r1,  #19
                    ss.clear();
                    string regto,num;
                    int n;
                    ss<<th[3];
                    ss>>regto;
                    ss.clear();
                    ss<<th[5]<<th[6]<<th[7];
                    ss>>hex>>n;
                    num  = to_string(n);
                    executePhase.push_back("subn:"+num+"from"+regto);
                    
                }
                else if(label =="subreg"){
                  //e04 11 002        sub     r1,  r2
                    ss.clear();
                    string regto,num;
                    int n;
                    ss<<th[3];
                    ss>>regto;
                    ss.clear();
                    ss<<th[5]<<th[6]<<th[7];
                    ss>>hex>>n;
                    num  = to_string(n);
                    executePhase.push_back("subr:"+num+"from"+regto);

                }
                else if(label =="push"){
                  //e52 d 1 004        push    {r1}
                    ss.clear();
                    string reg;
                    reg+=th[4];
                    
                    cout.clear();
                    executePhase.push_back("push:"+reg);
                    
                }
                else if(label =="addnum"){
                    ss.clear();
                    string regto,num;
                    int n;
                    regto+=th[3];
                    ss<<th[5]<<th[6]<<th[7];
                    ss>>hex>>n;
                    num  = to_string(n);
                    executePhase.push_back("addn:"+num+"from"+regto);
                }
                else if(label =="addreg"){
                     ss.clear();
                    string regto,num;
                    int n;
                    regto+=th[3];
                    
                    ss<<th[5]<<th[6]<<th[7];
                    ss>>hex>>n;
                    num  = to_string(n);
                    executePhase.push_back("addr:"+num+"from"+regto);
                }
                break;
            }
        }
    }
    //execute phase -> executing everything basically
    for(int i =0 ;i<executePhase.size();i++){
        if(debug==true){
            debugstat();
        }
    
        string instruction = executePhase[i].substr(0,executePhase[i].find(":"));
        if(instruction == "movn"){
            string cmd = executePhase[i].substr(executePhase[i].find(":")+1);
            string num = cmd.substr(0,cmd.find("to"));
            int reg = stoi(cmd.substr(cmd.find("to")+2));
            registers[reg]=stoi(num);
        }
        if(instruction == "movr"){
            string cmd = executePhase[i].substr(executePhase[i].find(":")+1);
            int from = stoi(cmd.substr(0,cmd.find("to")));
            int to = stoi(cmd.substr(cmd.find("to")+2));
            registers[to]=registers[from];
        }
        if(instruction == "push"){
            string regtopush= executePhase[i].substr(executePhase[i].find(":")+1);
            char *b = (char*)malloc(sizeof(unsigned long int)*2);
            sprintf(b,"%p",&registers[stoi(regtopush)]);
            string t = b;
            string data = t+":"+regtopush;
            STACK.push_back(data);
            registers[15]++;
            free(b);
        }
        if(instruction=="subn"){
            string cmd = executePhase[i].substr(executePhase[i].find(":")+1);
            int num = stoi(cmd.substr(0,cmd.find("from")));
            int reg = stoi(cmd.substr(cmd.find("from")+4));
          
            registers[reg] -= num;
            
            
            //AN UNSIGNED CANNOT NEVER EVER BE NEGATIVE!!
        }
        if(instruction=="subr"){
            string cmd = executePhase[i].substr(executePhase[i].find(":")+1);
            int num = stoi(cmd.substr(0,cmd.find("from")));
            int reg = stoi(cmd.substr(cmd.find("from")+4));
           
            registers[reg] -= registers[num];
            
           
            //AN UNSIGNED CANNOT NEVER EVER BE NEGATIVE!!
        }
        if(instruction=="addn"){
            string cmd = executePhase[i].substr(executePhase[i].find(":")+1);
            int num = stoi(cmd.substr(0,cmd.find("from")));
            int reg = stoi(cmd.substr(cmd.find("from")+4));
            registers[reg]+=num;
            //AN UNSIGNED CANNOT NEVER EVER BE NEGATIVE!!
        }
        if(instruction=="addr"){
            string cmd = executePhase[i].substr(executePhase[i].find(":")+1);
            int num = stoi(cmd.substr(0,cmd.find("from")));
            int reg = stoi(cmd.substr(cmd.find("from")+4));
            registers[reg]+=registers[num];
            //AN UNSIGNED CANNOT NEVER EVER BE NEGATIVE!!
        }
        registers[14]++; // pc 
    }
}
