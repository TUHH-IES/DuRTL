`timescale 1ns/1ps
module tb_ADLATCH();
	reg [1:0] D;
	reg EN;
	wire[1:0] Q;
	reg ARST;

	m_0 d1(.EN(EN),.ARST(ARST),.D(D),.Q(Q));

	initial begin
  		EN=0;
     		forever #10 EN = ~EN;  
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
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("adlatch.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
