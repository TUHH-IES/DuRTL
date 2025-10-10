module dff_design_tb;
  reg a,b,c;
  reg clk;
  output d,e;

  dff_hierarchy_design d1(.clk(clk), .a(a), .b(b), .c(c), .d(d), .e(e));
  initial begin
    #0
    clk = 0;
    a = 0;
    b = 0;
    c = 0;
    #1
    clk = 1;
    #1
    clk = 0;
    a = 0;
    b = 1;
    c = 1;
    #1
    clk = 1;
    #1
     clk = 0;
    a = 1;
    b = 1;
    c = 0;
    #1
    clk = 1;
    #1
    clk = 0;
    a = 1;
    b = 1;
    c = 1;
    #1
    clk = 1;
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
