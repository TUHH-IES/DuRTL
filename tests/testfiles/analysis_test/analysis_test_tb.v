module analysis_test_tb;
  reg clk = 0;
  reg a;
  reg b;
  output c, d;

  analysis_test dut(clk, a, b, c, d);
  initial begin
    #0
    a = 0;
    b = 0;
    #1
    a = 1;
    b = 0;
    #1
    a = 0;
    b = 1;
    #1
    a = 1;
    b = 1;
    #2
    $finish;
  end

  always #1 clk = ~clk;

  initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars(0);		//writing the vcd file
	end
	
	//initial
		//$monitor("At time %t, value a = %b value a = %b value a = %b", $time, a, b, c);
endmodule
