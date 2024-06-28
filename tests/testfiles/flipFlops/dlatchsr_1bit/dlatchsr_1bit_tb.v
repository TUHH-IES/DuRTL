`timescale 1us/100ns
module tb_DLATCHSR();
	reg D;
	reg  SET;
	reg  CLR;
	reg EN;
	wire Q;
	

	m_0 d1(.CLR(CLR), .D(D), .EN(EN), .Q(Q), .SET(SET));

	initial begin
  		EN=0;
     		forever #10 EN = ~EN;  
	end 

	initial begin 
		 #40;
		 D <= 'b0;
		 SET <= 'b0;
		 CLR <= 'b0;
		 #5;
		 CLR <= 'b1;
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
		//$dumpfile("dlatchsr.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
