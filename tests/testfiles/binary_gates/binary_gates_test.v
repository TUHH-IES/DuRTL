module m_0(
    a, b,
    and_out, or_out, xor_out, xnor_out, shl_out, shr_out, sshl_out, sshr_out, logic_and_out, logic_or_out, eqx_out, nex_out, lt_out, le_out, eq_out, ne_out, ge_out, gt_out, add_out, sub_out, mul_out, div_out, mod_out, pow_out);
  input wire a;
  input wire b;
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

  assign and_out = a & b;
  assign or_out = a | b;
  assign xor_out = a ^ b;
  assign xnor_out = a ~^ b;

  assign shl_out = a << b;
  assign shr_out = a >> b;
  assign sshl_out = a <<< b;
  assign sshr_out = a >>> b;

  assign logic_and_out = a && b;
  assign logic_or_out = a || b;

  assign eqx_out = a === b;
  assign nex_out = a !== b;

  assign lt_out = a < b;
  assign le_out = a <= b;
  assign eq_out = a == b;
  assign ne_out = a != b;
  assign ge_out = a >= b;
  assign gt_out = a > b;
  assign add_out = a + b;
  assign sub_out = a - b;
  assign mul_out = a * b;
  assign div_out = a / b;
  assign mod_out = a % b;
  assign pow_out = a ** b;

endmodule