`timescale 1ns/1ps
module tb_shift();
	reg [1:0] A;
	reg [1:0] B;
	wire[1:0] Y;

	m_0 d1(.A(A),.B(B),.Y(Y));


	initial begin 
		 #100;
		 A <= 'b00;
		 B <= 'b00;
		 #10;
		 B <= 'b01;
		 #10;
		 B <= 'b10;
		 #10;
		 B <= 'b11;
		 #10;
		 #100;
		 A <= 'b01;
		 B <= 'b00;
		 #10;
		 B <= 'b01;
		 #10;
		 B <= 'b10;
		 #10;
		 B <= 'b11;
		 #10;
		 #100;
		 A <= 'b10;
		 B <= 'b00;
		 #10;
		 B <= 'b01;
		 #10;
		 B <= 'b10;
		 #10;
		 B <= 'b11;
		 #10;
		 #100;
		 A <= 'b11;
		 B <= 'b00;
		 #10;
		 B <= 'b01;
		 #10;
		 B <= 'b10;
		 #10;
		 B <= 'b11;
		 #10;
		 #100;
		 
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("shift.vcd");  
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
