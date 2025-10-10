// Benchmark "and" written by ABC on Wed Sep 22 12:21:13 2021

module and ( 
    a1, a2, b1, b2,
    c1, c2  );
  input  a1, a2, b1, b2;
  output c1, c2;
  wire new_n7_, new_n8_, new_n10_, new_n11_, new_n12_;
  assign new_n7_ = a1 & ~a2;
  assign new_n8_ = b1 & ~b2;
  assign c1 = new_n7_ & new_n8_;
  assign new_n10_ = ~b1 & ~b2;
  assign new_n11_ = a2 & ~new_n10_;
  assign new_n12_ = new_n7_ & ~new_n8_;
  assign c2 = new_n11_ | new_n12_;
endmodule


