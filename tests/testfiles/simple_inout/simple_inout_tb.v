module simple_inout_tb();

reg [7:0] address = 0;
reg MEM_RW = 0;
reg MEM_OE = 0;
wire [17:0] MEM_DATA_bidir;
reg clk = 0;

   initial begin
      #0
      clk = 0;
      #2
      clk = 1;
      MEM_RW = 1;
      MEM_OE = 1;
      #2
      clk = 0;
      #2
      clk = 1;
      MEM_RW = 0;
      #2
      $finish;
   end


    simple_inout dut(.address(address), .MEM_RW(MEM_RW), .MEM_OE(MEM_OE), .MEM_DATA_bidir(MEM_DATA_bidir), .clk(clk));

	initial begin
    	$dumpfile(`DUMP_FILE_NAME);
    	$dumpvars(0);
    end

endmodule