`timescale 1ns/1ps
module unary_gates_tb();
	reg a;
	wire not_out;
    wire pos_out;
    wire neg_out;
    wire reduce_and_out;
    wire reduce_or_out;
    wire reduce_xor_out;
    wire reduce_xnor_out;
    wire logic_not_out;

	unary_gates dut(
	    .a(a),
	    .not_out(not_out),
	    .pos_out(pos_out),
	    .neg_out(neg_out),
	    .reduce_and_out(reduce_and_out),
	    .reduce_or_out(reduce_or_out),
	    .reduce_xor_out(reduce_xor_out),
	    .reduce_xnor_out(reduce_xnor_out),
	    .logic_not_out(logic_not_out)
		);

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
		$dumpvars(1);
	end
 	
endmodule 
