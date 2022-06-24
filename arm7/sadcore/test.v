`include "cpu.v"
`timescale 10ns/10ps

module tb();
  integer opcode;
  integer oprand1;
  integer oprand2;
  integer oprand3;

  integer addr;
  integer data;
  reg clock=1'b0;
  reg wl=1'b0;
  reg rdl=1'b0;
  wire [31:0] dout;
  //instructions 
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
  //end of instructions 
  core ARM7(
    .clk(clock),
    .opcode(opcode),
    .oprand1(oprand1),
    .oprand2(oprand2),
    .oprand3(oprand3),
    .dout(dout),
    .addr(addr),
    .data(data),
    .wl(wl),
    .rdl(rdl)
  );
  parameter freq = 2500000;
  always #(freq/2) clock<=!clock;
  initial begin
    opcode = movnum;
    oprand1 = 1;
    oprand2 = 5;
    @(posedge clock);
    @(posedge clock);
    
    opcode = strreg;
    oprand1 = 1;
    oprand2 = 1;
    oprand3 = 1;
    @(posedge clock);
    @(posedge clock);
    
    opcode = ldr;
    oprand1 = 0;
    oprand2 = 1;
    oprand3 = 1;

    @(posedge clock);
    @(posedge clock);
    wl = 1'b0;
    rdl = 1'b1;
    addr = oprand1*2;
    @(posedge clock);
        @(posedge clock);
 
    wl = 1'b0;
    rdl = 1'b0;


  end
endmodule
