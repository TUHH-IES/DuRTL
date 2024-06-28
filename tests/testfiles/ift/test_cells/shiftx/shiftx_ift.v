module m_0 (A,A_t, B,B_t, Y,Y_t);
    
    parameter A_SIGNED = 1;
    parameter B_SIGNED = 1;
    parameter A_WIDTH = 2;
    parameter B_WIDTH = 2;
    parameter Y_WIDTH = 2;
    
    input [A_WIDTH-1:0] A;
    input [B_WIDTH-1:0] B;
    output [Y_WIDTH-1:0] Y;
    
      
  input wire [31:0] A_t;
  input wire [31:0] B_t;
  output wire [31:0] Y_t;
  
    generate
        if (Y_WIDTH > 0)
            if (B_SIGNED) begin:BLOCK1
                assign Y = A[$signed(B) +: Y_WIDTH];
            end else begin:BLOCK2
                assign Y = A[B +: Y_WIDTH];
            end
    endgenerate
    assign Y_t = A_t | B_t;
    
endmodule

