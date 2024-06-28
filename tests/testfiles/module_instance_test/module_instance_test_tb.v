module Test_tb;
  reg a, b;
  wire c;

  B ins(a, b, c);

  initial begin
    #0
    a=0;
    b=0;
    #2
    a=1;
    b=1;
    #2;
    $finish;
  end
  
  initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars();		//writing the vcd file
	end
	
endmodule
