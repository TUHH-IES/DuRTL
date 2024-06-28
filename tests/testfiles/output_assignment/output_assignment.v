module output_assignment(a, b, y);
input [31:0] a;
input [31:0] b;
output [31:0] y;

wire [31:0] constant;
wire [31:0] test_wire;
wire [31:0] test_wire2;
wire [31:0] test_wire3;
wire [31:0] test_wire4;

assign constant = 32'b11000000000000000000000000000011;
assign y = (a | b) ^ constant;
assign test_wire = constant;
assign test_wire2 = a;
assign test_wire3 = y;
assign test_wire4 = test_wire3;

endmodule
