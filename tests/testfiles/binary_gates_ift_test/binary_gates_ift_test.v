module m_0(
    a, a_t, b, b_t,
    and_out, and_out_t, or_out, or_out_t, xor_out, xor_out_t, xnor_out, xnor_out_t, shl_out, shl_out_t, shr_out, shr_out_t, sshl_out, sshl_out_t, sshr_out, sshr_out_t, logic_and_out, logic_and_out_t, logic_or_out, logic_or_out_t, eqx_out, eqx_out_t, nex_out, nex_out_t, lt_out, lt_out_t, le_out, le_out_t, eq_out, eq_out_t, ne_out, ne_out_t, ge_out, ge_out_t, gt_out, gt_out_t, add_out, add_out_t, sub_out, sub_out_t, mul_out, mul_out_t, div_out, div_out_t, mod_out, mod_out_t, pow_out, pow_out_t);
  input wire a, b;
  input wire [31:0] a_t, b_t;
  output wire and_out;
  output wire or_out;
  output wire xor_out;
  output wire xnor_out;
  output wire shl_out;
  output wire shr_out;
  output wire sshl_out;
  output wire sshr_out;
  output wire logic_and_out;
  output wire logic_or_out;
  output wire eqx_out;
  output wire nex_out;
  output wire lt_out;
  output wire le_out;
  output wire eq_out;
  output wire ne_out;
  output wire ge_out;
  output wire gt_out;
  output wire add_out;
  output wire sub_out;
  output wire mul_out;
  output wire div_out;
  output wire mod_out;
  output wire pow_out;
  output wire [31:0] and_out_t, or_out_t, xor_out_t, xnor_out_t, shl_out_t, shr_out_t, sshl_out_t, sshr_out_t, logic_and_out_t, logic_or_out_t, eqx_out_t, nex_out_t, lt_out_t, le_out_t, eq_out_t, ne_out_t, ge_out_t, gt_out_t, add_out_t, sub_out_t, mul_out_t, div_out_t, mod_out_t, pow_out_t;

  assign and_out = a & b;
  assign and_out_t = ((a > 0 ? b_t : 0) | (b > 0 ? a_t : 0) | (a_t & b_t ));
  assign or_out = a | b;
  assign or_out_t = ((a == 0 ? b_t : 0) | ( b == 0 ? a_t : 0) | (a_t | b_t));
  assign xor_out = a ^ b;
  assign xor_out_t = a_t | b_t;
  assign xnor_out = a ~^ b;
  assign xnor_out_t = a_t | b_t;

  // if b changes then the output also changes
  assign shl_out = a << b;
  assign shl_out_t = a_t | b_t;
  assign shr_out = a >> b;
  assign shr_out_t = a_t | b_t;
  assign sshl_out = a <<< b;
  assign sshl_out_t = a_t | b_t;
  assign sshr_out = a >>> b;
  assign sshr_out_t = a_t | b_t;

  //same as bitwise and/or?? for 1 bit signals should be but for vectors?
  assign logic_and_out = a && b; 
  assign logic_and_out_t = ((a > 0 ? b_t : 0) | (b > 0 ? a_t : 0) | (a_t & b_t ));
  assign logic_or_out = a || b;
  assign logic_or_out_t = ((a == 0 ? b_t : 0) | ( b == 0 ? a_t : 0) | (a_t | b_t));
 
  // equivalence operators retrurning true/false value -> propagate both tags

  assign eqx_out = a === b;
  assign eqx_out_t = a_t | b_t;
  assign nex_out = a !== b;
  assign nex_out_t = a_t | b_t;

  assign lt_out = a < b;
  assign lt_out_t = a_t | b_t;
  assign le_out = a <= b;
  assign le_out_t = a_t | b_t;
  assign eq_out = a == b;
  assign eq_out_t = a_t | b_t;
  assign ne_out = a != b;
  assign ne_out_t = a_t | b_t;
  assign ge_out = a >= b;
  assign ge_out_t = a_t | b_t;
  assign gt_out = a > b;
  assign gt_out_t = a_t | b_t;

  // arithmetic operations -> propagate both tags
  assign add_out = a + b;
  assign add_out_t = a_t | b_t;
  assign sub_out = a - b;
  assign sub_out_t = a_t | b_t;
  assign mul_out = a * b;
  assign mul_out_t = a_t | b_t;
  assign div_out = a / b;
  assign div_out_t = a_t | b_t;
  assign mod_out = a % b;
  assign mod_out_t = a_t | b_t;
  assign pow_out = a ** b;
  assign pow_out_t = a_t | b_t;

endmodule