`timescale 1us/100ns
module tb_SR();
	reg [1:0] SET;
	reg [1:0] CLR;
	wire[1:0] Q;
	

	m_0 d1(.CLR(CLR), .Q(Q), .SET(SET));

	initial begin 
		 #40;
		 SET <= 'b00;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 SET <= 'b01;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 SET <= 'b11;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 SET <= 'b10;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("sr.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
