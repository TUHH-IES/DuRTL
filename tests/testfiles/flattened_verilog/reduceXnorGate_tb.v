module reduceXnorGate_tb;
	
	reg [2:0] a;
	wire out;

	// instance of the counter module
	reduceXnorGate a1 (a, out);

	initial begin
		a = 0;
		#2
		a = 2;
		#2
		a = 7;
		#2
		a = 1;
		#2
		a = 3;
	end
	
	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);  
			$dumpvars();		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, value = %h (%0d)", $time, out, out);
endmodule // test
