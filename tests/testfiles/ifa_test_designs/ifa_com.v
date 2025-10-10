module ifa_com(
    a,b,c,
    d,e);
  input wire a,b,c;
  output wire d,e;
  
  assign d = a & b & c;
  assign e = a | b | c;
  

endmodule