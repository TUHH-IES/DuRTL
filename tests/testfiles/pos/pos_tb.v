module pos__tb;
  reg a;
  output [4:0] c;

  pos_ d1(a, c);
  initial begin
    #0
    a = 1;
    #1
    a = 0;
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
