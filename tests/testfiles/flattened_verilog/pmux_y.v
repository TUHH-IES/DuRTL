module _90_pmux (A, B, S, Y);
	parameter WIDTH = 1;
	parameter S_WIDTH = 1;

	(* force_downto *)
	input [WIDTH-1:0] A;
	(* force_downto *)
	input [WIDTH*S_WIDTH-1:0] B;
	(* force_downto *)
	input [S_WIDTH-1:0] S;
	(* force_downto *)
	output [WIDTH-1:0] Y;

	(* force_downto *)
	wire [WIDTH-1:0] Y_B;

	genvar i, j;
	generate
		(* force_downto *)
		wire [WIDTH*S_WIDTH-1:0] B_AND_S;
		for (i = 0; i < S_WIDTH; i = i + 1) begin:B_AND
			assign B_AND_S[WIDTH*(i+1)-1:WIDTH*i] = B[WIDTH*(i+1)-1:WIDTH*i] & {WIDTH{S[i]}};
		end:B_AND
		for (i = 0; i < WIDTH; i = i + 1) begin:B_OR
			(* force_downto *)
			wire [S_WIDTH-1:0] B_AND_BITS;
			for (j = 0; j < S_WIDTH; j = j + 1) begin:B_AND_BITS_COLLECT
				assign B_AND_BITS[j] = B_AND_S[WIDTH*j+i];
			end:B_AND_BITS_COLLECT
			assign Y_B[i] = |B_AND_BITS;
		end:B_OR
	endgenerate

	assign Y = |S ? Y_B : A;
endmodule