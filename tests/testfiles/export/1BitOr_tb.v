module OneBitOr_tb;
  reg a;
  reg b;
  output c;
  reg[2:0] read_data [0:15];
  integer i;

  OneBitOr d1( a, b, c);

  initial begin
    $readmemb(`INPUT_FILE_NAME, read_data);
      for(i = 0; i<=15; i = i + 1) begin
      {a, b} =  read_data[i];
      #2;
    end
  end


  initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars(1,  a, b, c);		//writing the vcd file
	end
	//always@(a,b,c)
		//$monitor("At time %t, value_a = %b (%0d) value_b = %b (%0d) value_b = %b", $time, a, a, b, b, c, c);
endmodule
