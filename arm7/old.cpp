#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pty.h>
#include <elf.h>
#include <sstream>
#include <errno.h> 
#include <termios.h>
#include <sys/mman.h>
using namespace std;
/*
   alright, geohot's course is actually trying to show me how cpu works physically , so i will have to learn verilog. but before that. i should build an emulator
   so this is an emulator.
*/

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
class ARM7CPU{
    public:
        string filename;
        vector<pair<string,string> >opcodes={
            //x in push is representing the number of the register, will be done with .replace in strings! 
            {"movreg","e1a"},{"subnum","e24"},{"subreg","e04"},{"push","e52"},{"addnum","e28"},{"addreg","e08"},{"movnum","e3a"},{"svc","ef0"},{"ldr","e59"}
        
        };

        bool debug=false;
        vector<string>executePhase;
        void debugstat()
        {
            cout<<"\n*********** DEBUGGING = ON ************"<<endl;
            for(int i =0;i<registers.size();i++){
            if(i< 14){
                cout<<"R"<<i<<"="<<registers[i]<<",";    
            }
            else{
                    cout<<"| pc->"<<registers[14]<<" | sp-> "<<registers[15]<< "APSR:"<<registers[16]<<endl;
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
    void ExecuteInstruction(){
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
                    if(label == "svc"){
                       
                        string data = th.substr(th.find("ef")+2);
                        //cout<<data<<endl;
                        int vl;
                        ss.clear();
                        ss<<data;
                        ss>>hex>>vl;
                        data = to_string(vl);
                        executePhase.push_back("movn:"+data+"to16");

                    }
                    if(label == "ldr" || bytecode == "e59"){
                        string data = th.substr(th.find("e59f")+4);
                        string reg;
                        reg+=data[0];
                        data = data.substr(data.find(reg)+1);
                        ifstream str(data);
                        string text;
                        str>>text;
                        executePhase.push_back("movw:"+text+"to"+reg);
                    }
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
        vector<string> executes(executePhase.rbegin(),executePhase.rend());
        //execute phase -> executing everything basically
        for(int i =0 ;i<executes.size();i++){
            string instruction = executes[i].substr(0,executes[i].find(":"));
            if(registers[16] ==0 && registers[7] == 4){
                string wrd;
                string regval = to_string(registers[0]);
                for(int j =0;j<STACK.size();j++){
                    string reg = STACK[j].substr(STACK[j].find(":")+1);
                    if(regval == reg){
                        wrd =  STACK[j].substr(0,STACK[j].find(":"));
                        break;
                    }
                }
                if(debug == true){
                    printf("\n********** STDOUT ********");
                    for(int i =0;i<2;i++){
                        printf("\n                                           \n");
                    }
                }
                cout<<wrd<<endl;
                
                if(debug){
                    for(int i =0;i<2;i++){
                        printf("\n                                           \n");
                    }
                    printf("\n********** END OF STDOUT ********");
                }
            }
            if(instruction == "movn"){
                string cmd = executes[i].substr(executes[i].find(":")+1);
                string num = cmd.substr(0,cmd.find("to"));
                int reg = stoi(cmd.substr(cmd.find("to")+2));
                registers[reg]=stoi(num);
            }
            if(instruction == "movr"){
                string cmd = executes[i].substr(executes[i].find(":")+1);
                int from = stoi(cmd.substr(0,cmd.find("to")));
                int to = stoi(cmd.substr(cmd.find("to")+2));
                registers[to]=registers[from];
            }
            if(instruction == "push"){
                string regtopush= executes[i].substr(executes[i].find(":")+1);
                char *b = (char*)malloc(sizeof(unsigned long int)*2);
                sprintf(b,"%p",&registers[stoi(regtopush)]);
                string t = b;
                string data = t+":"+regtopush;
                STACK.push_back(data);
                registers[15]++;
                free(b);
            }
            if(instruction=="subn"){
                string cmd = executes[i].substr(executes[i].find(":")+1);
                int num = stoi(cmd.substr(0,cmd.find("from")));
                int reg = stoi(cmd.substr(cmd.find("from")+4));
            
                registers[reg] -= num;
                
                
                //AN UNSIGNED CANNOT NEVER EVER BE NEGATIVE!!
            }
            if(instruction=="subr"){
                string cmd = executes[i].substr(executes[i].find(":")+1);
                int num = stoi(cmd.substr(0,cmd.find("from")));
                int reg = stoi(cmd.substr(cmd.find("from")+4));
            
                registers[reg] -= registers[num];
                
            
                //AN UNSIGNED CANNOT NEVER EVER BE NEGATIVE!!
            }
            if(instruction == "movw"){
                string cmd = executes[i].substr(executes[i].find(":")+1);
                string reg = cmd.substr(cmd.find("to")+2);
                string wrd = cmd.substr(0,cmd.find("to"));
                for(int j =0;j<STACK.size();j++){
                    string addr =STACK[j];
                    string val = addr.substr(0,addr.find(":"));
                    string streg = addr.substr(addr.find(":")+1);
                    if(reg == streg){
                        STACK.erase(STACK.begin()+j);
                        STACK.push_back(wrd+":"+streg);
                    }
                }
            }
            if(instruction=="addn"){
                string cmd = executes[i].substr(executes[i].find(":")+1);
                int num = stoi(cmd.substr(0,cmd.find("from")));
                int reg = stoi(cmd.substr(cmd.find("from")+4));
                registers[reg]+=num;
                //AN UNSIGNED CANNOT NEVER EVER BE NEGATIVE!!
            }
            if(instruction=="addr"){
                string cmd = executes[i].substr(executes[i].find(":")+1);
                int num = stoi(cmd.substr(0,cmd.find("from")));
                int reg = stoi(cmd.substr(cmd.find("from")+4));
                registers[reg]+=registers[num];
                //AN UNSIGNED CANNOT NEVER EVER BE NEGATIVE!!
            }
            if(debug){
                debugstat();
            }
        
            registers[14]++; // pc 
        }

        executePhase.clear();
        STACK.clear();
        RAM.clear();
        registers.clear();

    }
    void setDebug(bool stat){
        debug = stat;
    }
    void SerialCom(){
       int master,slave;
       char name[1024];
       int port = openpty(&master,&slave,&name[0],0,0); // opening pseudoterminal and getting name, master ,slave

       unlockpt(slave); // unlocks pseudoterminal so it can write
       int acc = strtol("0777",0,8); // str attribute to 0777 so i can write
       chmod(name,slave);//modify 
       cout<<"serial-port:"<<name<<endl;
       struct termios tios;
       tcgetattr(master,&tios);
       tios.c_lflag=0;
       char buff[2048];
       while(true){
            memset(&buff,0,sizeof(buff));
            read(master,buff,sizeof(buff));
            string g = buff;
            if(g.length()  > 0 )
            {
                if(g.find("exec") != string::npos){
                    string r = "cc";
                    g = g.substr(g.find(":")+1);
                    filename = g;
                    write(master,r.c_str(),r.length());                
                    ExecuteInstruction();
                    for(int i =0;i<10;i++){
                    cout<<"                                                "<<endl;
                    }
                    cout<<"Execution finished"<<endl;
                }
                if(g.find("bootrom") != string::npos){
                    cout<<"** Received bootrom image ** "<<endl;
                    string data = g.substr(g.find(":")+1);
                    ifstream z("flag.cpu");
                    string cont;
                    z>>cont;
                    if(cont !="W"){
                        string r = "sigsegv";
                        ROMWrite(data);
                        write(master,r.c_str(),r.length());
                    }
                    
                }     
            }
       }
    }
    void ROMWrite(string data){
        ofstream rr("rom.cpu");
        rr<<data;
        rr.close();
        ofstream flag("flag.cpu");
        if(flag.is_open()){
            flag<<"W";
            cout<<"** write successful **"<<endl;
        }
        flag.close();
    }
};
int main(int argc,char *argv[]){
    ARM7CPU* arm7 = new ARM7CPU;
    arm7->setDebug(false);
    arm7->SerialCom();
}
