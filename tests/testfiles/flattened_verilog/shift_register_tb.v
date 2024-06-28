module shift_register_tb;
  reg clock=0;
  reg shift_in;
  wire shift_out;
  
  shift_register SR(clock, shift_in, shift_out);

  initial begin
    #0
    shift_in = 0;
    #2
    shift_in = 1;
    #2
    shift_in = 0;
    #2
    shift_in = 1;
    #2
    shift_in = 0;
    #2
    shift_in = 1;
    #12
    $finish;
  end
  
    /* Make a regular pulsing clock. */
    always #1 clock = !clock;

  initial
	begin
		$dumpfile(`DUMP_FILE_NAME);  
		$dumpvars();		//writing the vcd file
	end
	
endmodule
