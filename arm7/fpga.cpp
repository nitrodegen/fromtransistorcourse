# include <iostream>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <util.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SEGFAULT 25
using namespace std;


vector<pair<string,string> > opcodes={
            //x in push is representing the number of the register, will be done with .replace in strings! 
            {"movreg","e1a"},{"subnum","e24"},{"subreg","e04"},{"push","e52"},{"addnum","e28"},{"addreg","e08"},{"movnum","e3a"},{"svc","ef0"},{"ldr","e59"},{"pop","e49"},{"andreg","e00"},{"andnum","e20"},{"orreg","e18"},{"ornum","e38"} ,{"strreg","e78"},{"strnum","e58"}};


// == TEMPLATES == 
// mov reg: e1a0R00K
// mov num: e3a0ZNUM
// push: e52dR004
// subnum: e24PNUM
// subreg: e04P00R
// addnum: e28PNUM
// addreg: e08PR
// svc: efADDR 


vector<string>templates={
"e1a0R00K","e24PNUM","e04P00R","e52dR004","e28PNUM","e08PR","e3a0ZNUM","efADDR"
};
vector<signed int> registers(17);
//r0-r12 , pc ,sp,lr,apsr

vector<string>Decoded;


vector<unsigned long int >STACK(8000000);
vector<long int>RAM;
vector<string>PROGRAM;

void SendSIGNAL(int code){
    
    if(code == SEGFAULT){
       RAM.clear();
       registers.clear();
       Decoded.clear();
       PROGRAM.clear();
       STACK.clear();
       cout<<"Segmentation fault"<<endl;
    }

}
void cleanup(){
       RAM.clear();
       registers.clear();
       Decoded.clear();
       PROGRAM.clear();
       STACK.clear();
}
void debug(){
   for(int i =0;i<13 ;i++){
        if(i%4==0){
           cout<<"\n"<<endl;
          }
          if( i != 12 && i != 0 && i != 4 && i != 8){
            cout<<"  r"<<i<<":"<<registers[i]<<"  ";
          
          }
          else{
            cout<<"r"<<i<<":"<<registers[i]<<"  ";
          
          }
        
    }
    cout<<"\n\npc->"<<registers[14]<<endl;
    cout<<"sp->"<<registers[15]<<endl;
    cout<<"lr->"<<registers[16]<<endl;
    cout<<"============================================"<<endl;
}
void loadPROG(string code){  
    unsigned char b[2];
    int OFFSET =0;
    stringstream ss;


  
   vector<string>rawdat;
   vector<string>ofs;
   for(int i =0;i < code.length();i++){
        unsigned char ch= code[i];
        char *bob = (char*)malloc(sizeof(unsigned char)*2);

        sprintf(bob,"%02x",ch);
        ofs.push_back(bob);
        rawdat.push_back(bob);
        free(bob);
    }
   vector<string>data(rawdat.rbegin(),rawdat.rend());
    string of="0x"+ofs[24];
    ss.clear();
    ss<<of;
    ss>>hex>>OFFSET;
    string didi;
    for(int i =0;i<data.size();i++){
        if(i>=OFFSET){
            didi+=data[i];
        } 
    }

    for(int i =0;i<didi.length();i+=8){
        PROGRAM.push_back(didi.substr(i,8));

    }
    int sp = didi.length()/8;
    if(didi.length() % sp !=0){
        PROGRAM.push_back(didi.substr(sp*8));
    }
}

void decode(){
    stringstream ss;
    for(int i =PROGRAM.size();i>=0;i--){
      int kik =0;
      string code = PROGRAM[i];
      string opcode;
      opcode+=code[0];
      opcode+=code[1];
      string res = "";
      opcode+=code[2];
      for(auto const& th: opcodes){
          string op = th.second;
          string label = th.first;
          if(opcode == op){
              if(opcode != "e59" && opcode != "e78" && opcode != "e58"  &&opcode != "e18" && opcode != "e38" && opcode != "e52" && opcode  != "e49" && opcode != "e00" && opcode != "e20"){
                    code = code.substr(code.find(opcode)+3);
                    string to;
 //                   to+=code[0];
                    to+=code[1];
                     
                    string from;
                    ss.clear();
                    ss<<to;
                    int ff;
                    ss>>hex>>ff;
                    to = to_string(ff);
                    ss.clear();
                    for(int z =2;z<code.length();z++){
                       from+=code[z];
                    }
                    ss<<from;
                    int tt;
                    ss>>hex>>tt;
                    from = to_string(tt);
                    res=label+":"+from+"to"+to;
  //                  cout<<res<<endl;                         
                  
  
              }
              else if(opcode == "e59"){
                  code = code.substr(code.find(opcode)+3);;
                  string ld,src;
                  ld+=code[1];
                  src+=code[0];
                  ss.clear();
                  int l,s;
                  ss<<ld;
                  ss>>hex>>l;
                  ld = to_string(l);

                  ss.clear();
                 
                  ss<<src;
                  ss>>hex>>s;
                  src = to_string(s);
                  res = label+":"+ld+"to"+src;
                  
              
              }
              else if(opcode == "e78"){
                //5  2 001
                  int f,k,z;

                  code = code.substr(code.find(opcode)+3);
                  string  op1 ,op2 ,src;
                  op1+=code[0];
                  
                  op2 = code.substr(code.find(op1+op2)+2);
                  src += code[1];
                  
                  ss.clear();
                  ss<<src;
                  ss>>hex>>f;
                  src = to_string(f);
 
                  ss.clear();
                  ss<<op1;
                  ss>>hex>>k;
                  op1 = to_string(k);
 
                  ss.clear();
                  ss<<op2;
                  ss>>hex>>z;
                  op2 = to_string(z);
        
 //                 cout<<op1<<":"<<op2<<":"<<src<<endl;
                  res=label+":"+src+"addr"+op1+"/"+op2;
                  //cout<<res<<endl;
                 // exit(1);
             }
              else if(opcode == "e58"){
                 int f,k,z;

                  code = code.substr(code.find(opcode)+3);
                  string  op1 ,op2 ,src;
                  op1+=code[0];
                  
                  op2 = code.substr(code.find(op1+op2)+2);
                  src += code[1];
                  
                  ss.clear();
                  ss<<src;
                  ss>>hex>>f;
                  src = to_string(f);
 
                  ss.clear();
                  ss<<op1;
                  ss>>hex>>k;
                  op1 = to_string(k);
 
                  ss.clear();
                  ss<<op2;
                  ss>>hex>>z;
                  op2 = to_string(z);
        
 //                 cout<<op1<<":"<<op2<<":"<<src<<endl;
                  res=label+":"+src+"addr"+op1+"/"+op2;
             }


              else if(opcode == "e52"){
                    code = code.substr(code.find(opcode)+4);
                    string to;
                    to+=code[0];
                    res=label+":"+to;
              
              }
              else if(opcode == "e49"){
                     code = code.substr(code.find(opcode)+4);
                    string to;
                    to+=code[0];
                    res=label+":"+to;   
              }
              else if(opcode == "e18"){
                  code = code.substr(code.find(opcode)+3);
   //               cout<<code<<endl;
                  string dst,op1,op2;
                  dst+=code[1];
                  op1+=code[0];
                  op2+=code[code.length()-1];
                  res=label+":"+dst+"lab"+op1+"/"+op2;

              }
              else if(opcode == "e38"){
                  code = code.substr(code.find(opcode)+3);
 //                 cout<<code<<endl;
                  string dst,op1,op2;
                  dst+=code[1];
                  op1+=code[0];
                  op2+=code.substr(code.find(dst+op1)+2);
                  ss.clear();
                  ss<<op2;
                  int ff;
                  ss>>hex>>ff;
                  op2 = to_string(ff);
                  res=label+":"+dst+"lab"+op1+"/"+op2;
                 
              }



              else if(opcode == "e00"){
                  code = code.substr(code.find(opcode)+3);
   //               cout<<code<<endl;
                  string dst,op1,op2;
                  dst+=code[1];
                  op1+=code[0];
                  op2+=code[code.length()-1];
                  res=label+":"+dst+"lab"+op1+"/"+op2;

              }
              else if(opcode == "e20"){
                  code = code.substr(code.find(opcode)+3);
 //                 cout<<code<<endl;
                  string dst,op1,op2;
                  dst+=code[1];
                  op1+=code[0];
                  op2+=code.substr(code.find(dst+op1)+2);
                  ss.clear();
                  ss<<op2;
                  int ff;
                  ss>>hex>>ff;
                  op2 = to_string(ff);
                  res=label+":"+dst+"lab"+op1+"/"+op2;
                 
              }



     }
      }
      if(res.length() >0){
       
        Decoded.push_back(res);
      
      }
  }
}

void execute(){
  stringstream ss;
   for(int i =0;i<Decoded.size();i++){
        
       string instr = Decoded[i];
       string cmd = instr.substr(0,instr.find(":"));
       string vals  = instr.substr(instr.find(":")+1);
       if(cmd == "orreg"){
          string dst;
          int op1,op2;
          dst = vals[0];
         string te = vals.substr(vals.find("lab")+3);
          op1 = registers[stoi(te.substr(0,te.find("/")))];
          op2 = registers[stoi(te.substr(te.find("/")+1))];
          registers[stoi(dst)] = op1 | op2;
     
       }
      if(cmd == "ldr"){
        string t = vals.substr(0,vals.find("to"));
        string s = vals.substr(vals.find("to")+2);
        int src,regto;
        src = stoi(s);
        regto = stoi(t);
        registers[src] = STACK[regto];
      }
      if(cmd == "strreg"){
        
         string t,g,z;
          t+=vals[0];
          vals = vals.substr(vals.find("addr")+4);
          g= vals.substr(0,vals.find("/"));
          z= vals.substr(vals.find("/")+1);
          int val = registers[stoi(t)];
          int addr = registers[stoi(g)]+registers[stoi(z)];
          STACK[addr] = val;
          cout<<addr<<endl; 
          }
      if(cmd == "strnum"){
         string t,g,z;
          t+=vals[0];
          vals = vals.substr(vals.find("addr")+4);
          g= vals.substr(0,vals.find("/"));
          z= vals.substr(vals.find("/")+1);
          int val = registers[stoi(t)];
          int addr = registers[stoi(g)]+stoi(z);
          STACK[addr] = val;
          cout<<addr<<endl;
    
      }


      if(cmd == "ornum"){
           string dst;
          int op1,op2;
          dst = vals[0];
          string te = vals.substr(vals.find("lab")+3);
          op1 = registers[stoi(te.substr(0,te.find("/")))];
          op2 = stoi(te.substr(te.find("/")+1));
          registers[stoi(dst)] = op1 | op2;
       }


       if(cmd == "andreg"){
          string dst;
          int op1,op2;
          dst = vals[0];
         string te = vals.substr(vals.find("lab")+3);
          op1 = registers[stoi(te.substr(0,te.find("/")))];
          op2 = registers[stoi(te.substr(te.find("/")+1))];
          registers[stoi(dst)] = op1 & op2;
     
       }
      
      if(cmd == "andnum"){
           string dst;
          int op1,op2;
          dst = vals[0];
          string te = vals.substr(vals.find("lab")+3);
          op1 = registers[stoi(te.substr(0,te.find("/")))];
          op2 = stoi(te.substr(te.find("/")+1));
          registers[stoi(dst)] = op1 & op2;
       }


       if(cmd == "push"){
          string reg = instr.substr(instr.find(":")+1);
          
          ss.clear();
          unsigned long int  addr ;
          ss<<&registers[stoi(reg)];
          ss>>hex>>addr;
          if(registers[15] == (8000000-4)){
              SendSIGNAL(SEGFAULT);
          }
          STACK.insert(STACK.begin()+0,addr);
          registers[15]--;
        }
  
       if(cmd == "pop"){
          string reg = instr.substr(instr.find(":")+1);
          int index = 0;
          ss.clear();
          unsigned long addr;
          ss<<&registers[stoi(reg)];
          ss>>hex>>addr;
          
          for(int i =0;i<STACK.size();i++){
              if(addr ==  STACK[i]){
                STACK.erase(STACK.begin()+i);
                break;
              }
          }
          registers[stoi(reg)]=0;
          registers[15]++;

        }


        if(cmd == "subreg"){
            string move = vals.substr(0,vals.find("to"));
            string to =  vals.substr(vals.find("to")+2);
            registers[stoi(to)] -= registers[stoi(move)];
 //           cout<<instr<<endl;
   //         exit(1);
        }
        if(cmd == "subnum"){

            string move = vals.substr(0,vals.find("to"));
            string to =  vals.substr(vals.find("to")+2);
            registers[stoi(to)] -= stoi(move);

        }
  //                exit(1);            
           if(cmd == "addreg"){
            string move = vals.substr(0,vals.find("to"));
            string to =  vals.substr(vals.find("to")+2);
            registers[stoi(to)] += registers[stoi(move)];
 
        }
        if(cmd == "addnum"){

            string move = vals.substr(0,vals.find("to"));
            string to =  vals.substr(vals.find("to")+2);
            registers[stoi(to)] += stoi(move);
        }


        if(cmd == "movreg"){
            string move = vals.substr(0,vals.find("to"));
            string to =  vals.substr(vals.find("to")+2);
            registers[stoi(to)] = registers[stoi(move)];
 
        }
        if(cmd == "movnum"){

            string move = vals.substr(0,vals.find("to"));
            string to =  vals.substr(vals.find("to")+2);
            registers[stoi(to)] = stoi(move);

        }
        debug();
        registers[14]++;
      
   }
    
}

void Run(){
    

  string content;
  int master,slave;
  char name[1024];
  int port = openpty(&master,&slave,&name[0],0,0);
  unlockpt(slave);
  chmod(name,slave);
  struct termios term;
  tcgetattr(master,&term);
  term.c_lflag=0;
  char buffer[10000];
  int rc = 0;
  cout<<name<<endl;
  while(true){
    
    //memset(&buffer,0,sizeof(buffer));
    read(master,buffer,sizeof(buffer));
    content = buffer;
    cout<<content<<endl;
    cout<<rc<<endl;
    if(content.length()>0 && rc > 1) {
      
      content = content.substr(content.find(":")+1);
      ifstream fstr(content);
      stringstream ss;
      ss<<fstr.rdbuf();
      string incdata(ss.str());
      //cout<<incdata<<endl;
      loadPROG(incdata);
      decode();
      execute();
      cleanup();
      ss.clear();
      if(rc > 2){
          rc=0;
      }
    }
    rc++;
    write(master,(char*)0x41,sizeof(char));
    

  }
}

int main(int argc, char *argv[]){
    if(argc<2){
      exit(1);
    }
    Run();
} 

