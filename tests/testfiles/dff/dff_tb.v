module dff_tb;
  reg d;
  reg clk;
  output q;

  dff d1( d, clk, q);
  initial begin
    #0
    clk = 0;
    d = 0;
    #1
    clk = 1;
    d = 0;
    #1
    clk = 0;
    d = 1;
    #1
    clk = 1;
    d = 1;
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
