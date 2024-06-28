module constants(a, b, y);
input [31:0] a;
input [31:0] b;
output [31:0] y;

wire [31:0] constant;

assign constant = 32'b11000000000000000000000000000011;
assign y = (a | b) ^ constant;

endmodule
