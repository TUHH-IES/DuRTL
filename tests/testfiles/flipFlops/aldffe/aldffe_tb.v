`timescale 1ns/1ps
module tb_ALDFFE();
	reg [1:0] D;
	reg [1:0] AD;
	reg clk;
	reg aload;
	reg EN;
	wire[1:0] Q;
	

	m_0 d1(.AD(AD), .ALOAD(aload), .CLK(clk), .D(D), .EN(EN) ,.Q(Q));

	initial begin
  		clk=0;
     		forever #10 clk = ~clk;  
	end 

	initial begin 
		 #40;
		 EN <= 1;
		 D <= 'b00;
		 AD <= 'b11;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 D <= 'b01;
		 AD <= 'b1;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 D <= 'b10;
		 AD <= 'b01;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 D <= 'b11;
		 AD <= 'b00;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 
		 EN <= 0;
		 D <= 'b00;
		 AD <= 'b11;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 D <= 'b01;
		 AD <= 'b1;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 D <= 'b10;
		 AD <= 'b01;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 D <= 'b11;
		 AD <= 'b00;
		 #5;
		 aload <= 1;
		 #5;
		 aload <= 0;
		 #40;
		 
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("../../examples/flipFlops/aldffe/aldffeExport.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
