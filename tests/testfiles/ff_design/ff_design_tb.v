`timescale 1ns/1ps
module ff_design_tb;
    localparam W = 8;

    reg                   clk = 0;
    reg                   rst_n = 0;
    reg                   in_valid = 0;
    reg  [W-1:0]          a, b, c;
    wire                  out_valid;
    wire [(2*W):0]        y;

    ff_design #(.W(W)) dut (
        .clk(clk), .rst_n(rst_n),
        .in_valid(in_valid), .a(a), .b(b), .c(c),
        .out_valid(out_valid), .y(y)
    );

    // 10ns period
    always #5 clk = ~clk;

    initial begin
        // Reset
        rst_n = 0; in_valid = 0; a = 0; b = 0; c = 0;
        repeat (2) @(posedge clk);
        rst_n = 1;

        // Drive a few samples
        @(negedge clk); in_valid = 1; a = 8'd3;  b = 8'd4;  c = 8'd2;  // (3+4)*2 = 14
        @(negedge clk); in_valid = 1; a = 8'd10; b = 8'd1;  c = 8'd5;  // (11)*5 = 55
        @(negedge clk); in_valid = 1; a = 8'd7;  b = 8'd9;  c = 8'd3;  // (16)*3 = 48
        @(negedge clk); in_valid = 0; a = 0;     b = 0;     c = 0;

        // Let pipeline drain
        repeat (3) @(posedge clk);
        $finish;
    end
    
     initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars(0);		//writing the vcd file
	end
	
    
endmodule

