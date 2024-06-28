module output_assignment_tb();

reg [31:0] a;
reg [31:0] b;
wire [31:0] y;

output_assignment dut(.a(a), .b(b), .y(y));

initial begin
	a <= 32'h00000000;
	b <= 32'h00000000;
	#5;
	a <= 32'h00000001;
	b <= 32'h00000000;
	#5;
	a <= 32'h00001000;
	b <= 32'h00000000;
	#5;
	a <= 32'h10000000;
	b <= 32'h00000000;
	#5;
	a <= 32'h00000000;
	b <= 32'h00000001;
	#5;
	a <= 32'h00000001;
	b <= 32'h00001000;
	#5;
	a <= 32'h00001000;
	b <= 32'h10000000;
	#5;
	a <= 32'h10000000;
	b <= 32'h00000001;
end

initial begin
	$dumpfile(`DUMP_FILE_NAME);
	$dumpvars();
end

endmodule
