`timescale 1ns/1ps
module tb_DFFE();
	reg [1:0] D;
	reg clk;
	reg EN;
	wire[1:0] Q;

	m_0 d1(.CLK(clk),.D(D),.EN(EN),.Q(Q));

	initial begin
  		clk=0;
     		forever #10 clk = ~clk;  
	end 

	initial begin 
	  	 #100;
	  	 EN <= 'b0;	
		 #100;
		 D <= 'b00;
		 #100;
		 D <= 'b01;
		 #100;
		 D <= 'b10;
		 #100;
		 D <= 'b11;
		 #100;
	  	 EN <= 'b1;	
		 #100;
		 D <= 'b00;
		 #100;
		 D <= 'b01;
		 #100;
		 D <= 'b10;
		 #100;
		 D <= 'b11;
		 #100;
		 
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("../../examples/flipFlops/dffe/dffeExport.vcd");
		$dumpvars(1,clk,D,Q);		//writing the vcd file
	end
 	
endmodule 
