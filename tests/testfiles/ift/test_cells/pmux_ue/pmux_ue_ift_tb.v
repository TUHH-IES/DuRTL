`timescale 1us/100ns
module pmux_ue_tb();
	reg [1:0] A;
	reg [5:0] B;
	reg [2:0] S;
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
		#5;
		 A <= 0;
		 B <= 0;
		 S <= 0;
		 #5;
		 A <= 0;
		 B <= 0;
		 S <= 1;
		 #5;
		 A <= 0;
		 B <= 0;
		 S <= 2;
		 #5;
		 A <= 0;
		 B <= 0;
		 S <= 3;
		 #5;
		 A <= 0;
		 B <= 0;
		 S <= 4;
		 #5;
		 A <= 0;
		 B <= 0;
		 S <= 5;
		 #5;
		 A <= 0;
		 B <= 0;
		 S <= 6;
		 #5;
		 A <= 0;
		 B <= 19;
		 S <= 0;
		 #5;
		 A <= 0;
		 B <= 19;
		 S <= 1;
		 #5;
		 A <= 0;
		 B <= 19;
		 S <= 2;
		 #5;
		 A <= 0;
		 B <= 19;
		 S <= 3;
		 #5;
		 A <= 0;
		 B <= 19;
		 S <= 4;
		 #5;
		 A <= 0;
		 B <= 19;
		 S <= 5;
		 #5;
		 A <= 0;
		 B <= 19;
		 S <= 6;
		 #5;
		 A <= 0;
		 B <= 38;
		 S <= 0;
		 #5;
		 A <= 0;
		 B <= 38;
		 S <= 1;
		 #5;
		 A <= 0;
		 B <= 38;
		 S <= 2;
		 #5;
		 A <= 0;
		 B <= 38;
		 S <= 3;
		 #5;
		 A <= 0;
		 B <= 38;
		 S <= 4;
		 #5;
		 A <= 0;
		 B <= 38;
		 S <= 5;
		 #5;
		 A <= 0;
		 B <= 38;
		 S <= 6;
		 #5;
		 A <= 0;
		 B <= 57;
		 S <= 0;
		 #5;
		 A <= 0;
		 B <= 57;
		 S <= 1;
		 #5;
		 A <= 0;
		 B <= 57;
		 S <= 2;
		 #5;
		 A <= 0;
		 B <= 57;
		 S <= 3;
		 #5;
		 A <= 0;
		 B <= 57;
		 S <= 4;
		 #5;
		 A <= 0;
		 B <= 57;
		 S <= 5;
		 #5;
		 A <= 0;
		 B <= 57;
		 S <= 6;
		 #5;
		 A <= 1;
		 B <= 0;
		 S <= 0;
		 #5;
		 A <= 1;
		 B <= 0;
		 S <= 1;
		 #5;
		 A <= 1;
		 B <= 0;
		 S <= 2;
		 #5;
		 A <= 1;
		 B <= 0;
		 S <= 3;
		 #5;
		 A <= 1;
		 B <= 0;
		 S <= 4;
		 #5;
		 A <= 1;
		 B <= 0;
		 S <= 5;
		 #5;
		 A <= 1;
		 B <= 0;
		 S <= 6;
		 #5;
		 A <= 1;
		 B <= 19;
		 S <= 0;
		 #5;
		 A <= 1;
		 B <= 19;
		 S <= 1;
		 #5;
		 A <= 1;
		 B <= 19;
		 S <= 2;
		 #5;
		 A <= 1;
		 B <= 19;
		 S <= 3;
		 #5;
		 A <= 1;
		 B <= 19;
		 S <= 4;
		 #5;
		 A <= 1;
		 B <= 19;
		 S <= 5;
		 #5;
		 A <= 1;
		 B <= 19;
		 S <= 6;
		 #5;
		 A <= 1;
		 B <= 38;
		 S <= 0;
		 #5;
		 A <= 1;
		 B <= 38;
		 S <= 1;
		 #5;
		 A <= 1;
		 B <= 38;
		 S <= 2;
		 #5;
		 A <= 1;
		 B <= 38;
		 S <= 3;
		 #5;
		 A <= 1;
		 B <= 38;
		 S <= 4;
		 #5;
		 A <= 1;
		 B <= 38;
		 S <= 5;
		 #5;
		 A <= 1;
		 B <= 38;
		 S <= 6;
		 #5;
		 A <= 1;
		 B <= 57;
		 S <= 0;
		 #5;
		 A <= 1;
		 B <= 57;
		 S <= 1;
		 #5;
		 A <= 1;
		 B <= 57;
		 S <= 2;
		 #5;
		 A <= 1;
		 B <= 57;
		 S <= 3;
		 #5;
		 A <= 1;
		 B <= 57;
		 S <= 4;
		 #5;
		 A <= 1;
		 B <= 57;
		 S <= 5;
		 #5;
		 A <= 1;
		 B <= 57;
		 S <= 6;
		 #5;
		 A <= 2;
		 B <= 0;
		 S <= 0;
		 #5;
		 A <= 2;
		 B <= 0;
		 S <= 1;
		 #5;
		 A <= 2;
		 B <= 0;
		 S <= 2;
		 #5;
		 A <= 2;
		 B <= 0;
		 S <= 3;
		 #5;
		 A <= 2;
		 B <= 0;
		 S <= 4;
		 #5;
		 A <= 2;
		 B <= 0;
		 S <= 5;
		 #5;
		 A <= 2;
		 B <= 0;
		 S <= 6;
		 #5;
		 A <= 2;
		 B <= 19;
		 S <= 0;
		 #5;
		 A <= 2;
		 B <= 19;
		 S <= 1;
		 #5;
		 A <= 2;
		 B <= 19;
		 S <= 2;
		 #5;
		 A <= 2;
		 B <= 19;
		 S <= 3;
		 #5;
		 A <= 2;
		 B <= 19;
		 S <= 4;
		 #5;
		 A <= 2;
		 B <= 19;
		 S <= 5;
		 #5;
		 A <= 2;
		 B <= 19;
		 S <= 6;
		 #5;
		 A <= 2;
		 B <= 38;
		 S <= 0;
		 #5;
		 A <= 2;
		 B <= 38;
		 S <= 1;
		 #5;
		 A <= 2;
		 B <= 38;
		 S <= 2;
		 #5;
		 A <= 2;
		 B <= 38;
		 S <= 3;
		 #5;
		 A <= 2;
		 B <= 38;
		 S <= 4;
		 #5;
		 A <= 2;
		 B <= 38;
		 S <= 5;
		 #5;
		 A <= 2;
		 B <= 38;
		 S <= 6;
		 #5;
		 A <= 2;
		 B <= 57;
		 S <= 0;
		 #5;
		 A <= 2;
		 B <= 57;
		 S <= 1;
		 #5;
		 A <= 2;
		 B <= 57;
		 S <= 2;
		 #5;
		 A <= 2;
		 B <= 57;
		 S <= 3;
		 #5;
		 A <= 2;
		 B <= 57;
		 S <= 4;
		 #5;
		 A <= 2;
		 B <= 57;
		 S <= 5;
		 #5;
		 A <= 2;
		 B <= 57;
		 S <= 6;
		 end

		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		//$dumpfile("pmux_ue.vcd");
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
