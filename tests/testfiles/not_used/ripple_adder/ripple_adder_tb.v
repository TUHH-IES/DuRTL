module test;
 reg [3:0] X, Y;
 wire [3:0] S;
 wire Co;

 integer i,j;

	/* make a reset that pulses once. */
	reg reset = 0;
	initial begin
		# 1 X = 0;
        Y = 0;
		# 3 X = 2;
        Y = 1;
		# 5 X = 15;
        Y = 6;
    for (i=0; i<16; i= i+1)
      for (j=0; j<16; j= j+1)
        begin
        #2 X=i; Y=j;
        end
	end
	
	/* Make a regular pulsing clock. */
	
	// instance of the RA module
	ripple_adder A (X, Y, S, Co);
	
	initial
		begin
			$dumpfile("ripple_adder.vcd");  
			$dumpvars;		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, value = X %d  Y %d  S %d  Co %d  ", $time, X , Y, S, Co);
endmodule // test
