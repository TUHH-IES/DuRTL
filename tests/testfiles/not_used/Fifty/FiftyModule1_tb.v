module FiftyModule_tb;
reg [16:0] in_0, in_1, in_2, in_3, in_4, in_5;
output [16:0]out_0,out_1,out_2,out_3,out_4;
M0 test(in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4);
initial begin
in_0 = 16'b0000000000000000;
in_1 = 16'b0000000000000000;
in_2 = 16'b0000000000000000;
in_3 = 16'b0000000000000000;
in_4 = 16'b0000000000000000;
in_5 = 16'b0000000000000000;
#2
in_0 = 16'b0000000001111111;
in_1 = 16'b0000000011111111;
in_2 = 16'b0000000111111111;
in_3 = 16'b0000001111111111;
in_4 = 16'b0000011111111111;
in_5 = 16'b0000111111111111;
#4
in_0 = 16'b0000000000000000;
in_1 = 16'b0000000000000000;
in_2 = 16'b0000000000000000;
in_3 = 16'b1111111111111111;
in_4 = 16'b0000000000000000;
in_5 = 16'b0000000000000000;
#6
in_0 = 16'b1111111100000000;
in_1 = 16'b1111111100000000;
in_2 = 16'b1111111100000000;
in_3 = 16'b1111111111111111;
in_4 = 16'b1111111100000000;
in_5 = 16'b1111111100000000;
#8
in_0 = 16'b0000000001111111;
in_1 = 16'b0000000001111111;
in_2 = 16'b0000000001111111;
in_3 = 16'b1111111111111111;
in_4 = 16'b0000000001111111;
in_5 = 16'b0000000001111111;
#10
in_0 = 16'b0000000000000000;
in_1 = 16'b0000000000000000;
in_2 = 16'b0000000000000000;
in_3 = 16'b0000000011111111;
in_4 = 16'b0000000000000000;
in_5 = 16'b0000000000000000;
#12
in_0 = 16'b1111111100000000;
in_1 = 16'b1111111110000000;
in_2 = 16'b1111111111000000;
in_3 = 16'b1111111111111111;
in_4 = 16'b1111111111110000;
in_5 = 16'b1111111111111000;
#14
in_0 = 16'b1111111111111111;
in_1 = 16'b1111111111111111;
in_2 = 16'b1111111111111111;
in_3 = 16'b1111111111111111;
in_4 = 16'b1111111111111111;
in_5 = 16'b1111111111111111;
end
initial
begin
	$dumpfile("../../examples/PathMaker/../Fifty/FiftyModule.vcd");
	$dumpvars();
end
endmodule
