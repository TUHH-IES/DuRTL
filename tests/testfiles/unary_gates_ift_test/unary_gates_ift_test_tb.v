`timescale 1ns/1ps
module unary_gates_tb();
	reg a;
	reg [31:0] a_t;
	wire not_out;
    wire pos_out;
    wire neg_out;
    wire reduce_and_out;
    wire reduce_or_out;
    wire reduce_xor_out;
    wire reduce_xnor_out;
    wire logic_not_out;
	wire [31:0] not_out_t, pos_out_t, neg_out_t, reduce_and_out_t, reduce_or_out_t, reduce_xor_out_t, reduce_xnor_out_t, logic_not_out_t;

	reg [31:0] read_data [0:3];
	integer i;

	m_0 d1(
	    .a(a),
		.a_t(a_t),
	    .not_out(not_out),
		.not_out_t(not_out_t),
	    .pos_out(pos_out),
		.pos_out_t(pos_out_t),
	    .neg_out(neg_out),
		.neg_out_t(neg_out_t),
	    .reduce_and_out(reduce_and_out),
		.reduce_and_out_t(reduce_and_out_t),
	    .reduce_or_out(reduce_or_out),
		.reduce_or_out_t(reduce_or_out_t),
	    .reduce_xor_out(reduce_xor_out),
		.reduce_xor_out_t(reduce_xor_out_t),
	    .reduce_xnor_out(reduce_xnor_out),
		.reduce_xnor_out_t(reduce_xnor_out_t),
	    .logic_not_out(logic_not_out),
		.logic_not_out_t(logic_not_out_t)
		);

 	initial begin
		$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<=3; i += 1) begin
		{a_t} =  read_data[i];
		#5;
		end
	end

	initial begin
		 a <= 'b0;
		 #10;
		 a <= 'b1;
		 #10
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
