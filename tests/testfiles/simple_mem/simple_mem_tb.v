module simple_mem_tb();

reg [7:0] address = 0;
wire [17:0] mem_data;
reg clk = 0;

   initial begin
      #0
      clk = 0;
      #2
      clk = 1;
      address = 1;
      #2
      clk = 0;
      #2
      clk = 1;
      address = 2;
      #2
      $finish;
   end


    simple_mem dut(.address(address), .mem_data(mem_data), .clk(clk));

	initial begin
    	$dumpfile(`DUMP_FILE_NAME);
    	$dumpvars(0);
    end

endmodule