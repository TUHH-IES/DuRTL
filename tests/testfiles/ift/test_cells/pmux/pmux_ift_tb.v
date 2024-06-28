`timescale 1us/100ns
module pmux_tb();
	reg [1:0] A;
	reg [1:0] B;
	reg S;
	wire[1:0] Y;
	
	reg[31:0] A_t;
	reg[31:0] B_t;
	reg[31:0] S_t;
	wire[31:0] Y_t;
	
	reg[95:0] read_data [0:7];
	integer i;

	m_0 d1(.A(A),.A_t(A_t),.B(B),.B_t(B_t),.S(S),.S_t(S_t),.Y(Y),.Y_t(Y_t));

	initial begin 
		$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<=7; i+=1) begin
		{ A_t,B_t,S_t} = read_data[i]; 
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
		 end
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("pmux.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 

