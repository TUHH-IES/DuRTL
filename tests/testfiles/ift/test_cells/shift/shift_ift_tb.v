`timescale 1ns/1ps
module shift_tb();
	reg [1:0] A;
	reg [1:0] B;
	wire[1:0] Y;
	
	reg[31:0] A_t;
	reg[31:0] B_t;
	wire[31:0] Y_t;

	reg[63:0] read_data [0:3];
	integer i;
	
	m_0 d1(.A(A),.A_t(A_t),.B(B),.B_t(B_t),.Y(Y),.Y_t(Y_t));


	initial begin 
	$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<=3; i+=1) begin
		{ A_t,B_t} = read_data[i]; 
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
		 end
		 
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("shift.vcd");  
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
