module ScalableModule_tb;
reg [6:0] in_0, in_1, in_2, in_3, in_4, in_5;
output [6:0]out_0,out_1,out_2,out_3,out_4, out_5;
M0 test(in_0,in_1,in_2,in_3,in_4,in_5,out_0,out_1,out_2,out_3,out_4, out_5);
initial begin
in_0 = 6'b000000;
in_1 = 6'b000000;
in_2 = 6'b000000;
in_3 = 6'b000000;
in_4 = 6'b000000;
in_5 = 6'b000000;
#2
in_0 = 6'b111111;
in_1 = 6'b111111;
in_2 = 6'b111111;
in_3 = 6'b111111;
in_4 = 6'b111111;
in_5 = 6'b111111;
#4
in_0 = 6'b000000;
in_1 = 6'b000000;
in_2 = 6'b000000;
in_3 = 6'b111111;
in_4 = 6'b000000;
in_5 = 6'b000000;
#6
in_0 = 6'b000000;
in_1 = 6'b000000;
in_2 = 6'b000000;
in_3 = 6'b111111;
in_4 = 6'b000000;
in_5 = 6'b000000;
#8
in_0 = 6'b111111;
in_1 = 6'b111111;
in_2 = 6'b111111;
in_3 = 6'b111111;
in_4 = 6'b111111;
in_5 = 6'b111111;
#10
in_0 = 6'b000000;
in_1 = 6'b000000;
in_2 = 6'b000000;
in_3 = 6'b111111;
in_4 = 6'b000000;
in_5 = 6'b000000;
#12
in_0 = 6'b000000;
in_1 = 6'b000000;
in_2 = 6'b000000;
in_3 = 6'b111111;
in_4 = 6'b110000;
in_5 = 6'b111000;
#14
in_0 = 6'b111111;
in_1 = 6'b111111;
in_2 = 6'b111111;
in_3 = 6'b111111;
in_4 = 6'b111111;
in_5 = 6'b111111;
end
initial
begin
	$dumpfile("../../examples/SixBySix/SixBySixScalable1.vcd");
	$dumpvars();
end
endmodule
