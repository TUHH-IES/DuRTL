module unary_gates(
    a,
    not_out, pos_out, neg_out, reduce_and_out, reduce_or_out, reduce_xor_out, reduce_xnor_out, logic_not_out);
  input wire a;
  output wire not_out;
  output wire pos_out;
  output wire neg_out;
  output wire reduce_and_out;
  output wire reduce_or_out;
  output wire reduce_xor_out;
  output wire reduce_xnor_out;
  output wire logic_not_out;
  
  assign not_out = ~a;
  assign pos_out = +a;
  assign neg_out = -a;
  assign reduce_and_out = &a;
  assign reduce_or_out = |a;
  assign reduce_xor_out = ^a;
  assign reduce_xnor_out = ~^a;
  assign logic_not_out = !a;
  
endmodule