module yadmc_sdram16_tb  #(
                        	parameter sdram_depth = 25,
                        	parameter sdram_columndepth = 9,
                        	parameter cache_depth = 10,
                        	
                        	/*
                        	 * Cache line depth is always 2 for 16-bit SDRAM:
                        	 * burst length is 8, which means 16 bytes are transferred per burst.
                        	 * with a cache word size of 4 bytes, this yields 4 cache words per burst.
                        	 * Thus the depth of 2 (to make FSMs simpler, we use cache line length = burst length).
                        	 */
                        	parameter cache_linedepth = 2,
                        	
                        	parameter sdram_rowdepth = sdram_depth-1-sdram_columndepth-2 /* -2 for the banks */
                        );

	reg sdram_clk = 0;
	reg sdram_rst = 0;
	
	/* Command port */
	reg command_evict = 0;
	reg command_refill = 0;
	wire command_ack;
	reg [sdram_depth-cache_linedepth-2-1:0] evict_adr = 0;  /* unregistered */
	reg [sdram_depth-cache_linedepth-2-1:0] refill_adr = 0; /* unregistered */
	
	/* Cache interface */
	wire [cache_depth+cache_linedepth-1:0] cache_adr;
	wire [31:0] cache_dat_o;
	wire [3:0] cache_we;
	reg [31:0] cache_dat_i = 0;
	
	/* SDRAM interface */
	wire sdram_cke;
	wire sdram_cs_n;
	wire sdram_we_n;
	wire sdram_cas_n;
	wire sdram_ras_n;
	wire [1:0] sdram_dqm;
	wire [12:0] sdram_adr;
	wire [1:0] sdram_ba;
	wire [15:0] sdram_dq;

    reg [15:0] inout_drive = 0;
    wire [15:0] inout_recv;
    
    assign sdram_dq = inout_drive;
    assign inout_recv = sdram_dq;

yadmc_sdram16 dut(
    .sdram_clk(sdram_clk),
    .sdram_rst(sdram_rst),
    .command_evict(command_evict),
    .command_refill(command_refill),
    .command_ack(command_ack),
    .evict_adr(evict_adr),
    .refill_adr(refill_adr),
    .cache_adr(cache_adr),
    .cache_dat_o(cache_dat_o),
    .cache_we(cache_we),
    .cache_dat_i(cache_dat_i),
    .sdram_cke(sdram_cke),
    .sdram_cs_n(sdram_cs_n),
    .sdram_we_n(sdram_we_n),
    .sdram_cas_n(sdram_cas_n),
    .sdram_ras_n(sdram_ras_n),
    .sdram_dqm(sdram_dqm),
    .sdram_adr(sdram_adr),
    .sdram_ba(sdram_ba),
    .sdram_dq(sdram_dq)
    );

    always #10 sdram_clk=~sdram_clk;

	initial begin
#10
#10
#10
#10
#10
#10
#10
#10
#10
#10
#10
#10
#10
#10
#10
#10
#10
#10
#10
#10
		 $finish;
	end

	initial begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars(0);
	end

endmodule

