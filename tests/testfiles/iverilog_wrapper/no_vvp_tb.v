module d1_tb();

wire b;
reg a;  

d1 dut(a,b);

initial begin
	a = 1;
	a = 0;
end

endmodule
