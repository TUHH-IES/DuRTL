`timescale 1us/100ns
module tb_DFFSR();
	reg D;
	reg SET;
	reg CLR;
	reg clk;
	wire Q;
	
	m_0 d1(.CLK(clk), .SET(SET), .CLR(CLR), .D(D), .Q(Q));

	initial begin
  		clk=0;
     		forever #10 clk = ~clk;  
	end 

	initial begin 
		 #40;
		 D <= 'b0;
		 SET <= 'b0;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b0;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #40;
		 D <= 'b0;
		 SET <= 'b1;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b10;
		 #40;
		 D <= 'b1;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b0;
		 SET <= 'b1;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b0;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b1;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #40;
		 D <= 'b1;
		 SET <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #40;
		 D <= 'b0;
		 SET <= 'b1;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b0;
		 SET <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
		 D <= 'b1;
		 SET <= 'b1;
		 #5;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
		 #40;
	
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("dffsr.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
