module yadmc_dpram_tb #(
                     	parameter address_depth = 10,
                     	parameter data_width = 8
                     ) ();

	reg clk0;
	reg [address_depth-1:0] adr0;
	reg we0;
	reg [data_width-1:0] di0;
	wire [data_width-1:0] do0;

	reg clk1;
	reg [address_depth-1:0] adr1;
	reg we1;
	reg [data_width-1:0] di1;
	wire [data_width-1:0] do1;

	   initial begin
          #0
          clk0 = 0;
          clk1 = 0;
          #2
          clk0 = 1;
          clk1 = 0;
          adr1 = 1;
          #2
          clk0 = 0;
          clk1 = 1;
          #2
          clk0 = 1;
          clk1 = 1;
          adr0 = 2;
          #2
          clk0 = 0;
          clk1 = 0;
          #2
          clk0 = 1;
          clk1 = 0;
          adr1 = 1;
          #2
          clk0 = 0;
          clk1 = 1;
          #2
          clk0 = 1;
          clk1 = 1;
          adr0 = 2;
          #2
          $finish;
       end

yadmc_dpram dut(.clk0(clk0),
                .adr0(adr0),
                .we0(we0),
                .di0(di0),
                .do0(do0),
                .clk1(clk1),
                .adr1(adr1),
                .we1(we1),
                .di1(di1),
                .do1(do1)
                );

   	initial begin
       	$dumpfile(`DUMP_FILE_NAME);
       	$dumpvars(0);
       end


endmodule