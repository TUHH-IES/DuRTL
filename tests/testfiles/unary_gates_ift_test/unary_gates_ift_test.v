module m_0( 
    a, a_t,
    not_out, not_out_t, pos_out, pos_out_t, neg_out, neg_out_t, reduce_and_out, reduce_and_out_t, reduce_or_out, reduce_or_out_t, reduce_xor_out, reduce_xor_out_t, reduce_xnor_out, reduce_xnor_out_t, logic_not_out, logic_not_out_t);
  input wire a;
  input wire [31:0] a_t;
  output wire not_out;
  output wire pos_out;
  output wire neg_out;
  output wire reduce_and_out;
  output wire reduce_or_out;
  output wire reduce_xor_out;
  output wire reduce_xnor_out;
  output wire logic_not_out;
  output wire [31:0] not_out_t, pos_out_t, neg_out_t, reduce_and_out_t, reduce_or_out_t, reduce_xor_out_t, reduce_xnor_out_t, logic_not_out_t;
  
  assign not_out = ~a;
  assign not_out_t = a_t;
  assign pos_out = +a;
  assign pos_out_t = a_t;
  assign neg_out = -a;
  assign neg_out_t = a_t;
  assign reduce_and_out = &a;
  assign reduce_and_out_t = a_t;
  assign reduce_or_out = |a;
  assign reduce_or_out_t = a_t;
  assign reduce_xor_out = ^a;
  assign reduce_xor_out_t = a_t;
  assign reduce_xnor_out = ~^a;
  assign reduce_xnor_out_t = a_t;
  assign logic_not_out = !a;
  assign logic_not_out_t = a_t;
  
endmodule