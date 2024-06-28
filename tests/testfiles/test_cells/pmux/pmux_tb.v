`timescale 1us/100ns
module tb_SR();
	reg [1:0] A;
	reg [1:0] B;
	reg S;
	wire[1:0] Y;
	

	m_0 d1(.A(A),.B(B),.S(S),.Y(Y));

	initial begin 
		 #40;
		 A <= 'b00;
		 B <= 'b00;
		 S <= 'b1;
		 #5;
		 B <= 'b10;
		 #5;
		 B <= 'b11;
		 #40;
		 A <= 'b01;
		 B <= 'b00;
		 S <= 'b1;
		 #5;
		 B <= 'b10;
		 #5;
		 B <= 'b11;
		 #40;
		 A <= 'b11;
		 B <= 'b00;
		 S <= 'b1;
		 #5;
		 B <= 'b10;
		 #5;
		 B <= 'b11;
		 #40;
		 A <= 'b10;
		 B <= 'b00;
		 S <= 'b1;		 
		 #5;
		 B <= 'b10;
		 #5;
		 B <= 'b11;
		 #40;
		 A <= 'b00;
		 B <= 'b00;
		 S <= 'b0;
		 #5;
		 B <= 'b10;
		 #5;
		 B <= 'b11;
		 #40;
		 A <= 'b01;
		 B <= 'b00;
		 S <= 'b0;
		 #5;
		 B <= 'b10;
		 #5;
		 B <= 'b11;
		 #40;
		 A <= 'b11;
		 B <= 'b00;
		 S <= 'b0;
		 #5;
		 B <= 'b10;
		 #5;
		 B <= 'b11;
		 #40;
		 A <= 'b10;
		 B <= 'b00;
		 S <= 'b0;		 
		 #5;
		 B <= 'b10;
		 #5;
		 B <= 'b11;
		 #40;
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("pmux.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 

