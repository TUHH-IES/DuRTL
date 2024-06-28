module ripple_adder(X, Y, S, Co);
 input [3:0] X, Y;
 output [3:0] S;
 output Co;
 wire w1, w2, w3;

 fulladder u1(.X(X[0]), .Y(Y[0]), .Ci(1'b0), .S(S[0]), .Co(w1));
 fulladder u2(.X(X[1]), .Y(Y[1]), .Ci(w1), .S(S[1]), .Co(w2));
 fulladder u3(.X(X[2]), .Y(Y[2]), .Ci(w2), .S(S[2]), .Co(w3));
 fulladder u4(.X(X[3]), .Y(Y[3]), .Ci(w3), .S(S[3]), .Co(Co));
endmodule

module fulladder(X, Y, Ci, S, Co);
  input X, Y, Ci;
  output S, Co;
  wire w1,w2,w3;


  xor G1(w1, X, Y);
  xor G2(S, w1, Ci);
  and G3(w2, w1, Ci);
  and G4(w3, X, Y);
  or G5(Co, w2, w3);
endmodule
