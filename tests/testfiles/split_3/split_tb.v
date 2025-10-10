module split_tb;
	
	reg [3:0] a;
	reg [2:0] b;
	wire [1:0] out;

	// instance of the counter module
	split a1 (.a(a),.b(b),.out(out));

	initial begin
		a = 4'b0000;
		b = 3'b000;
		#1
		a = 4'b0001;
		b = 3'b000;
		#1
		a = 4'b0010;
		b = 3'b101;
		#1
		a = 4'b0101;
		b = 3'b001;
		#1

		$finish;
	end
	
	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);  
			$dumpvars(1);		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, value = %h (%0d)", $time, out, out);
endmodule // test
