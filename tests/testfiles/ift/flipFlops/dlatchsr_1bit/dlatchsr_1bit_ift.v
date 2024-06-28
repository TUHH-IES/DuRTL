/* Generated by Yosys 0.20 (git sha1 4fcb95ed087, clang  -fPIC -Os) */

(* dynports =  1  *)
(* src = "dlatch.v:1.1-15.10" *)
module m_0(CLR, CLR_t, D, D_t, EN, EN_t, Q, Q_t, SET, SET_t);
  (* src = "dlatch.v:7.19-7.20" *)
  input CLR;
  wire CLR;
  (* src = "dlatch.v:7.19-7.20" *)
  input D;
  wire D;
  (* src = "dlatch.v:6.7-6.9" *)
  input EN;
  wire EN;
  (* src = "dlatch.v:8.24-8.25" *)
  output Q;
  reg Q;
  (* src = "dlatch.v:7.19-7.20" *)
  input SET;
  wire SET;
   input [31:0] EN_t;
  input [31:0] D_t;
  input [31:0] SET_t;
  input [31:0] CLR_t;
  output reg [31:0] Q_t;
  (* src = "dlatch.v:10.1-13.4" *)

  initial begin
      Q_t = 0;
  end

  always @*
    if (CLR)begin
     Q <= 1'b0;
     Q_t <= CLR_t;
     end
    else if (SET) begin
     Q <= 1'b1;
     Q_t <= SET_t;
     end
    else if (EN)begin
     Q = D;
     Q_t = D_t | EN_t;
     end
endmodule