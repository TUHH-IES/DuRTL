module pass_through_tb;
  reg [2:0] a;
  wire [2:0] c;
  wire [2:0] d;

  reg clk=0;
	always #2 clk = !clk;

  pass_through d1(clk, a, c, d);
  
  initial begin
    #1
    a = 0;
    #4
    a = 1;
    #4
    a = 2;
    #4
    a = 3;
    #4
    a = 4;
    #4
    a = 5;
    #4
    $finish;
  end


  initial
	begin
		//$dumpfile("`DUMP_FILE_NAME");
		$dumpfile("test.vcd");
		$dumpvars(0);		//writing the vcd file
	end
	
	//initial
		//$monitor("At time %t, value a = %b value a = %b value a = %b", $time, a, b, c);
endmodule
