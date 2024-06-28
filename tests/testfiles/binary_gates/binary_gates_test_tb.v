`timescale 1ns/1ps
module binary_gates_tb();
	reg a;
	reg  b;
	wire and_out;
    wire or_out;
    wire xor_out;
    wire xnor_out;
    wire shl_out;
    wire shr_out;
    wire sshl_out;
    wire sshr_out;
    wire logic_and_out;
    wire logic_or_out;
    wire eqx_out;
    wire nex_out;
    wire lt_out;
    wire le_out;
    wire eq_out;
    wire ne_out;
    wire ge_out;
    wire gt_out;
    wire add_out;
    wire sub_out;
    wire mul_out;
    wire div_out;
    wire mod_out;
    wire pow_out;

	m_0 d1(
	    .a(a),
	    .b(b),
	    .and_out(and_out),
	    .or_out(or_out),
	    .xor_out(xor_out),
	    .xnor_out(xnor_out),
	    .shl_out(shl_out),
	    .shr_out(shr_out),
	    .sshl_out(sshl_out),
	    .sshr_out(sshr_out),
	    .logic_and_out(logic_and_out),
	    .logic_or_out(logic_or_out),
	    .eqx_out(eqx_out),
	    .nex_out(nex_out),
	    .lt_out(lt_out),
	    .le_out(le_out),
	    .eq_out(eq_out),
	    .ne_out(ne_out),
	    .ge_out(ge_out),
	    .gt_out(gt_out),
	    .add_out(add_out),
	    .sub_out(sub_out),
	    .mul_out(mul_out),
	    .div_out(div_out),
	    .mod_out(mod_out),
	    .pow_out(pow_out));

	initial begin
		 a <= 'b0;
		 b <= 'b0;
		 #10;
		 a <= 'b0;
         b <= 'b1;
		 #10;
		 a <= 'b1;
         b <= 'b0;
		 #10;
		 a <= 'b1;
         b <= 'b1;
		 #10;
		 
		 $finish;
	end
	initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars(1);		//writing the vcd file
	end
 	
endmodule 
