module anda_tb;
  reg [4:0] a;
  reg [4:0] b;
  output [4:0] c;

  anda d1( a, b, c);
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
    a = 5'b0000;
    b = 5'b0100;
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
