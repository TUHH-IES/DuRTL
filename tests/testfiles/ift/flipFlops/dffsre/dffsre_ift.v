/* Generated by Yosys 0.20 (git sha1 4fcb95ed087, clang  -fPIC -Os) */

(* dynports =  1  *)
(* src = "dffsre.v:1.1-30.10" *)
module m_0( CLK, CLK_t,CLR, CLR_t, D, D_t, EN, EN_t, Q, Q_t, SET, SET_t);
  (* src = "dffsre.v:9.7-9.10" *)
  input CLK;
  wire CLK;
  (* src = "dffsre.v:10.24-10.27" *)
  input CLR;
  wire CLR;
  (* src = "dffsre.v:10.29-10.30" *)
  input D;
  wire D;
  (* src = "dffsre.v:9.12-9.14" *)
  input EN;
  wire EN;
  (* src = "dffsre.v:11.24-11.25" *)
  output Q;
  reg Q;
  (* src = "dffsre.v:10.19-10.22" *)
  input SET;
  wire SET;
  
  input [31:0] CLK_t;
  input [31:0] D_t;
  input [31:0] EN_t;
  input [31:0] SET_t;
  input [31:0] CLR_t;
  output reg [31:0] Q_t;

  initial begin
      Q_t = 0;
  end

  (* src = "dffsre.v:20.3-26.18" *)
  always @(posedge CLK, posedge SET, posedge CLR)
    if (CLR) begin
    Q <= 1'b0;
    Q_t <= CLR_t;
    end
    else if (SET) begin
     Q <= 1'b1;
     Q_t <= (^D === 1'bx) ? 0 : ((D == SET) ? (D_t | SET_t) : SET_t);
    end
    else if (EN) begin
     Q <= D;
     Q_t <= D_t | EN_t;
     end
endmodule
