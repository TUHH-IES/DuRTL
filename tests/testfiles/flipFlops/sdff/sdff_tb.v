`timescale 1ns/1ps
module tb_SDFF();
	reg [1:0] D;
	reg clk;
	wire[1:0] Q;
	reg SRST;

	m_0 d1(.CLK(clk), .D(D), .SRST(SRST), .Q(Q));

	initial begin
  		clk=0;
     		forever #5 clk = ~clk;  
	end 

	initial begin 
		 #20;
		 D <= 'b00;
		 SRST <= 0;
		 #20;
		 D <= 'b00;
		 SRST <= 1;
		 #20;
		 D <= 'b01;
		 SRST <= 0;
		 #20;
		 D <= 'b01;
		 SRST <= 1;
		 #20;
		 D <= 'b10;
		 SRST <= 0;
		 #20;
		 D <= 'b10;
		 SRST <= 1;
		 #20;
		 D <= 'b11;
		 SRST <= 0;
		 #20;
		 D <= 'b11;
		 SRST <= 1;
		 #20;
		 
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("../../examples/flipFlops/sdff/sdffExport.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
