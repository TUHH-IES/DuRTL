module eth_fifo_tb();

parameter DATA_WIDTH    = 32;
parameter DEPTH         = 8;
parameter CNT_WIDTH     = 4;

	reg clk = 0;
	reg reset = 0;
	reg write = 0;
	reg read = 0;
	reg clear = 0;
	reg [DATA_WIDTH-1:0] data_in = 0;

    reg [31:0] clk_t = 0;
    reg [31:0] reset_t = 0;
    reg [31:0] write_t = 0;
    reg [31:0] read_t = 0;
    reg [31:0] clear_t = 0;
    reg [31:0] data_in_t = 0;

	wire  [DATA_WIDTH-1:0]  data_out;
    wire                    almost_full;
    wire                    full;
    wire                    almost_empty;
    wire                    empty;
    wire  [CNT_WIDTH-1:0]   cnt;

    wire [31:0] data_out_t;
    wire [31:0] almost_full_t;
    wire [31:0] full_t;
    wire [31:0] almost_empty_t;
    wire [31:0] empty_t;
    wire [31:0] cnt_t;

	   initial begin
          #0
          clk = 0;
          #2
          clk = 1;
          clear = 1;
          #2
          clk = 0;
          #2
          clk = 1;
          clear = 0;
          #2
          clk = 0;
          #2
          clk = 1;
          write = 1;
          #2
          clk = 0;
          #2
          clk = 1;
          write = 0;
          #2
            clk = 0;
            #2
            clk = 1;
            #2
            clk = 0;
            #2
            clk = 1;
            #2
            clk = 0;
            #2
            clk = 1;
            #2
            clk = 0;
            #2
            clk = 1;
            #2
          $finish;
       end

eth_fifo dut(.data_in(data_in), .data_out(data_out), .clk(clk), .reset(reset), .write(write), .read(read), .clear(clear),
             .almost_full(almost_full), .full(full), .almost_empty(almost_empty), .empty(empty), .cnt(cnt),
             .data_in_t(data_in_t), .data_out_t(data_out_t), .clk_t(clk_t), .reset_t(reset_t), .write_t(write_t), .read_t(read_t), .clear_t(clear_t),
                          .almost_full_t(almost_full_t), .full_t(full_t), .almost_empty_t(almost_empty_t), .empty_t(empty_t), .cnt_t(cnt_t)
                );

   	initial begin
       	$dumpfile(`DUMP_FILE_NAME);
       	$dumpvars(0);
       end


endmodule