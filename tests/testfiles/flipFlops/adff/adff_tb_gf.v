`timescale 1ns/1ps
module tb_ADFF();
  parameter WIDTH= 5;
	reg [WIDTH-1:0] D;
	reg clk;
	wire[WIDTH-1:0] Q;
	reg ARST;

	m_0 #(WIDTH) d1(.ARST(ARST),.CLK(clk),.D(D),.Q(Q));

	initial begin
  		clk=0;
     		forever #10 clk = ~clk;  
	end 
	initial begin
  		D=0;
      forever #1 D= D+1;  
	end 


	initial begin 
		 #40;
		 D <= 'b00;
		 #5;
		 ARST <= 1;
		 #5;
		 ARST <= 0;
		 #40;
		 D <= 'b01;
		 #5;
		 ARST <= 1;
		 #5;
		 ARST <= 0;
		 #40;
		 D <= 'b10;
		 #5;
		 ARST <= 1;
		 #5;
		 ARST <= 0;
		 #40;
		 D <= 'b11;
		 #5;
		 ARST <= 1;
		 #5;
		 ARST <= 0;
		 #40;
		 
		 $finish;
	end
	initial
	begin
		$dumpfile("adff_gf.vcd");
		$dumpvars(0,ARST,clk,D,Q);		//writing the vcd file
	end
 	
endmodule 
