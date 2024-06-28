/* Generated by Yosys 0.20 (git sha1 4fcb95ed087, clang  -fPIC -Os) */

(* dynports =  1  *)
(* src = "mux.v:1.1-16.10" *)
module m_0 (A,A_t, B, B_t, S,S_t, Y,Y_t);
  (* src = "mux.v:5.23-5.24" *)
  input [1:0] A;
  wire [1:0] A;
  (* src = "mux.v:5.26-5.27" *)
  input [1:0] B;
  wire [1:0] B;
  (* src = "mux.v:6.11-6.12" *)
  input S;
  wire S;
  (* src = "mux.v:7.28-7.29" *)
  output [1:0] Y;
  wire [1:0] Y;
  input wire [31:0] A_t;
  input wire [31:0] B_t;
  input wire [31:0] S_t;
  output wire [31:0] Y_t;
  
  function [1:0] _0_;
    input [1:0] a;
    input [1:0] b;
    input [0:0] s;
    (* full_case = 32'd1 *)
    (* parallel_case = 32'd1 *)
    (* src = "mux.v:10.13-10.14|mux.v:10.9-13.19" *)
    (* parallel_case *)
    casez (s)
      1'b1:
        _0_ = b[1:0];
      default:
        _0_ = a;
    endcase
  endfunction
  
    function [31:0] _1_;
    input [31:0] a_t;
    input [31:0] b_t;
    input [31:0] s_t;
    input [0:0] s;
    (* full_case = 32'd1 *)
    (* parallel_case = 32'd1 *)
    (* src = "mux.v:10.13-10.14|mux.v:10.9-13.19" *)
    (* parallel_case *)
    casez (s)
      1'b1:
        _1_ = (^s === 1'bx) ? 0 : (b_t | s_t);
      default:
        _1_ = (^s === 1'bx) ? 0 : (a_t | s_t);
    endcase
  endfunction
  assign Y = _0_(A,B,S);
  assign Y_t = _1_(A_t, B_t, S_t,S);
endmodule
