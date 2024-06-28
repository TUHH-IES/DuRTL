module counter_tb;

	/* make a reset that pulses once. */
	reg rst = 0;
	reg [31:0] rst_t = 0;
	reg [31:0] en_t = 0;
	reg [31:0] clk_t = 0;
	
	reg [95:0] read_data [0:95];
   	integer i, t;
   
	initial begin
		# 12 rst = 1;
		# 16 rst = 0;
		# 20 rst = 1;
		# 12 rst = 0;
		# 300 $finish;
	end

  initial begin
      $readmemb(`INPUT_FILE_NAME, read_data);
      for(i = 0; i<=4; i++) begin
      {rst_t, en_t, t} =  read_data[i];
      #t;
      end
   end

	/* Make a regular pulsing clock. */
	reg clk = 0;
	always #2 clk = !clk;
	wire [3:0] count;
	wire [31:0] count_t;
	reg en = 1;

	//timesteps to clock frequency

	always #8 en= $time % 3;
	// instance of the counter module
	counter c1 ( .clk(clk), .clk_t(clk_t), .rst(rst), .rst_t(rst_t), .en(en), .en_t(en_t), .count(count), .count_t(count_t));
	
	initial
		begin
			$dumpfile(`DUMP_FILE_NAME);
			$dumpvars;		//writing the vcd file
		end
	
	//initial
		//$monitor("At time %t, clk %d, rst %d, en %d, count %d", $time, clk, rst, en, count);
endmodule // test
