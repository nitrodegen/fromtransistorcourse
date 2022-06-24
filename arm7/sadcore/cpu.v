module core(opcode,oprand1,oprand2,oprand3, addr,data,clk,wl,rdl,dout);
  input[31:0] opcode; 
  input [31:0] oprand1;
  input [31:0] oprand2;
  input [31:0] oprand3;
  output reg[31:0]  dout;
  input [31:0] addr;
  input [31:0] data;
  input wl;
  input rdl;
  input clk;
  //CPU begins here
  reg [12:0] registers[31:0];
  reg [31:0] sp=0;
  reg [31:0] ip=0;
  reg [31:0] lr=0;
  reg [0:16000000] sram[31:0]; // SRAM (16 MB in size) 
   //STACK IS LOCATED FROM 0 -> 0x7a1200 (8MB STACK) 


  parameter freq = 2500000; // 100MHZ speed 
  //instruction opcodes
  reg [31:0] movreg = 32'b00000000000000000000111000011010;
  reg [31:0] movnum = 32'b00000000000000000000111000111010;
  reg [31:0] svc= 32'b00000000000000000000111011110000;
  reg [31:0] ldr = 32'b00000000000000000000111001011001;
  reg [31:0] subnum = 32'b00000000000000000000111000100100;
  reg [31:0] subreg = 32'b00000000000000000000111000000100;
  reg [31:0] push = 32'b00000000000000000000111001010010;
  reg [31:0] addnum = 32'b00000000000000000000111000101000;
  reg [31:0] addreg = 32'b00000000000000000000111000001000;
  reg [31:0] pop = 32'b00000000000000000000111001001001;
  reg [31:0] andreg = 32'b00000000000000000000111000000000;
  reg [31:0] andnum  =32'b00000000000000000000111000100000;
  reg [31:0] orreg = 32'b00000000000000000000111000011000;
  reg [31:0] ornum  =32'b00000000000000000000111000111000;
  reg [31:0] strreg = 32'b00000000000000000000111001111000;
  reg [31:0] strnum = 32'b00000000000000000000111001011000;
  reg freqc;
  reg [31:0] wraddr=0;
  always @ (posedge clk)begin
    if(freqc != freq-1 )begin   
      if(wl == 1'b1 && rdl == 1'b0)begin
          sram[addr] <=data;
      end
      if(wl == 1'b0 && rdl == 1'b1)begin
          dout <= sram[addr];
      end


      if(wl == 1'b0 && rdl == 1'b0)begin
        if(opcode == movnum )begin 
            registers[oprand1] <= oprand2;
        end 
        if(opcode == movreg)begin
            registers[oprand1] <= registers[oprand2];
        end
        if(opcode == subnum)begin
            registers[oprand1] <= registers[oprand1] - oprand2;
        end
        if(opcode == subreg)begin
            registers[oprand1] <= registers[oprand1] - registers[oprand2];
        end
        if(opcode == addnum)begin
            registers[oprand1] += oprand2;
        end
        if(opcode == addreg)begin
            registers[oprand1] += registers[oprand2];
        end
        if(opcode == andnum )begin
            registers[oprand1]  <=  registers[oprand1] & oprand2;
        end
        if(opcode == andreg )begin
            registers[oprand1]  <=  registers[oprand1] & registers[oprand2];
        end
        if(opcode == ornum )begin
            registers[oprand1]  <=  registers[oprand1] | oprand2;
        end
        if(opcode == orreg )begin
            registers[oprand1]  <=  registers[oprand1] | registers[oprand2];
        end

        if(opcode == push)
        begin
           sp<=sp-4;
           sram[0] <= registers[oprand1];
        end
        if(opcode == pop)
        begin
           sp<=sp+4;
           sram[0] <=sram[1];

        end
        if(opcode == ldr)begin
             wraddr <= registers[oprand2]+registers[oprand3];
             registers[oprand1] <= sram[wraddr];
        end 

        if(opcode == strreg)begin
          if(oprand2 <8000000)begin
              wraddr <= registers[oprand2]+registers[oprand3];
              sram[wraddr] <=  registers[oprand1];
          end
          else
            $finish;
        end 
        if(opcode == strnum) begin
          if(oprand2 <8000000)begin
              wraddr <= registers[oprand2]+oprand3;
              sram[wraddr ] <=  registers[oprand1];
          end
          else
            $finish;
        end 
      end
       $display("dout: %d  | 0:%d 1:%d 2:%d 3:%d 4:%d 5:%d 6:%d 7:%d 8:%d 9:%d 10:%d 11:%d 12:%d ip:%d",dout,registers[0],registers[1],registers[2],registers[3],registers[4],registers[5],registers[6],registers[7],registers[8],registers[9],registers[10],registers[11],registers[12],ip); 
        ip <= ip+1;


      freqc<=freqc+1;
    
    end
    else
    begin
        freqc <= 0;
          
    end

  end 

  
endmodule
