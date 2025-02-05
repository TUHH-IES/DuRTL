`timescale 1us/100ns
module tb_DLATCHSR();
	reg [1:0] D;
	reg [1:0] SET;
	reg [1:0] CLR;
	reg EN;
	wire[1:0] Q;
	

	m_0 d1(.CLR(CLR), .D(D), .EN(EN), .Q(Q), .SET(SET));

	initial begin
  		EN=0;
     		forever #10 EN = ~EN;  
	end 

	initial begin 
		 #40;
		 D <= 'b00;
		 SET <= 'b00;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b01;
		 SET <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b10;
		 SET <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b11;
		 SET <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b00;
		 SET <= 'b01;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b01;
		 SET <= 'b01;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b10;
		 SET <= 'b01;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b11;
		 SET <= 'b01;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b00;
		 SET <= 'b11;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b01;
		 SET <= 'b11;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b10;
		 SET <= 'b11;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b11;
		 SET <= 'b11;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b00;
		 SET <= 'b10;
		 CLR <= 'b00;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b01;
		 SET <= 'b10;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b10;
		 SET <= 'b10;
		 #5;
		 CLR <= 'b10;
		 #5;
		 CLR <= 'b11;
		 #40;
		 D <= 'b11;
		 SET <= 'b10;
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
		//$dumpfile("dlatchsr.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
