module ScalableModule_tb;
reg [4:0] in_0, in_1;
output [4:0]out_0;
M0 test(in_0,in_1,out_0);
initial begin
in_0 = 4'b0000;
in_1 = 4'b0000;
#2
in_0 = 4'b1111;
in_1 = 4'b1111;
#4
in_0 = 4'b0000;
in_1 = 4'b0000;
#6
in_0 = 4'b0000;
in_1 = 4'b0000;
#8
in_0 = 4'b1111;
in_1 = 4'b1111;
#10
in_0 = 4'b0000;
in_1 = 4'b0000;
#12
in_0 = 4'b0000;
in_1 = 4'b0000;
#14
in_0 = 4'b1111;
in_1 = 4'b1111;
end
initial
begin
	$dumpfile("../../examples/Scalable/ScalableModule.vcd");
	$dumpvars();
end
endmodule
