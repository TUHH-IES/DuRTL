module not__tb;
  reg [4:0] a;
  output [4:0] c;

  not_ d1( a, c);
  initial begin
    #0
    a = 5'b00000;
    #1
    a = 5'b00001;
    #1
    $finish;
  end


  initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars(0);		//writing the vcd file
	end
	
	//initial
		//$monitor("At time %t, value a = %b value a = %b value a = %b", $time, a, b, c);
endmodule
