`timescale 1ns/1ps
module binary_gates_tb();
	reg a, b;
	reg [31:0] a_t; 
	reg [31:0] b_t;
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
	wire [31:0] and_out_t, or_out_t, xor_out_t, xnor_out_t, shl_out_t, shr_out_t, sshl_out_t, sshr_out_t, logic_and_out_t, logic_or_out_t, eqx_out_t, nex_out_t, lt_out_t, le_out_t, eq_out_t, ne_out_t, ge_out_t, gt_out_t, add_out_t, sub_out_t, mul_out_t, div_out_t, mod_out_t, pow_out_t;

	reg [63:0] read_data [0:7];
	integer i;

	m_0 d1(
	    .a(a),
		.a_t(a_t),
	    .b(b),
		.b_t(b_t),
	    .and_out(and_out),
		.and_out_t(and_out_t),
	    .or_out(or_out),
		.or_out_t(or_out_t),
	    .xor_out(xor_out),
		.xor_out_t(xor_out_t),
	    .xnor_out(xnor_out),
	    .xnor_out_t(xnor_out_t),
	    .shl_out(shl_out),
	    .shl_out_t(shl_out_t),
	    .shr_out(shr_out),
	    .shr_out_t(shr_out_t),
	    .sshl_out(sshl_out),
	    .sshl_out_t(sshl_out_t),
	    .sshr_out(sshr_out),
	    .sshr_out_t(sshr_out_t),
	    .logic_and_out(logic_and_out),
	    .logic_and_out_t(logic_and_out_t),
	    .logic_or_out(logic_or_out),
	    .logic_or_out_t(logic_or_out_t),
	    .eqx_out(eqx_out),
	    .eqx_out_t(eqx_out_t),
	    .nex_out(nex_out),
	    .nex_out_t(nex_out_t),
	    .lt_out(lt_out),
	    .lt_out_t(lt_out_t),
	    .le_out(le_out),
	    .le_out_t(le_out_t),
	    .eq_out(eq_out),
	    .eq_out_t(eq_out_t),
	    .ne_out(ne_out),
	    .ne_out_t(ne_out_t),
	    .ge_out(ge_out),
	    .ge_out_t(ge_out_t),
	    .gt_out(gt_out),
	    .gt_out_t(gt_out_t),
	    .add_out(add_out),
	    .add_out_t(add_out_t),
	    .sub_out(sub_out),
	    .sub_out_t(sub_out_t),
	    .mul_out(mul_out),
	    .mul_out_t(mul_out_t),
	    .div_out(div_out),
	    .div_out_t(div_out_t),
	    .mod_out(mod_out),
	    .mod_out_t(mod_out_t),
	    .pow_out(pow_out),
		.pow_out_t(pow_out_t));

	initial begin
		$readmemb(`INPUT_FILE_NAME, read_data);
		for(i = 0; i<=7; i += 1) begin
		{b_t, a_t} =  read_data[i];
		#5;
		end
	end


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
