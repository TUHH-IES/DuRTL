module counter_tb;

	/* make a reset that pulses once. */
	reg rst = 0;
	initial begin
		# 17 rst = 1;
		# 11 rst = 0;
		# 29 rst = 1;
		# 11 rst = 0;
		# 300 $finish;
	end
	
	/* Make a regular pulsing clock. */
	reg clk = 0;
	always #2 clk = !clk;
	wire [3:0] count;
	reg en = 1;
	
	//timesteps to clock frequency

	always #8 en= $time % 3;
	// instance of the counter module
	counter c1 ( clk, rst, en, count);
	
	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);
			$dumpvars;		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, clk %d, rst %d, en %d, count %d", $time, clk, rst, en, count);
endmodule // test
