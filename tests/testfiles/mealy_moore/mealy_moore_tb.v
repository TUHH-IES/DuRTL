module mealy_moore_tb;
  reg [2:0] a;
  reg [2:0] b;
  wire [2:0] c;
  wire [2:0] d;

  reg clk=0;
	always #2 clk = !clk;

  mealy_moore d1(clk, a, b, c, d);
  
  initial begin
    #1
    a = 0;
    b = 0;
    #4
    a = 1;
    b = 0;
    #4
    a = 0;
    b = 1;
    #4
    a = 1;
    b = 1;
    #4
    a = 0;
    b = 0;
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
