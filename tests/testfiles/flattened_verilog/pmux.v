module pmux( a, b, s, out);

	input [3:0] a;
	input [11:0] b;
	input [2:0] s;
	output [3:0] out;
	//reg [3:0] out;
	wire [3:0] out;
	
	assign out = s==3'b001 ? b[3:0] : s==3'b010 ? b[7:4] : s==3'b100 ? b[11:8] : a;

	// always@(*)
	// begin
	// 	case (s)
	// 		3'b000 : out <= a;
	// 		3'b001 : out <= b[3:0];
	// 		3'b010 : out <= b[7:4];
	// 		3'b100 : out <= b[11:8];
	// 	endcase
	// end

	//assign out1 = out;

endmodule
