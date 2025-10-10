// tb_pipelined_mac.v
`timescale 1ns/1ps

module tb_pipelined_mac;
    localparam integer WIDTH = 8;
    localparam integer SUMW  = WIDTH + 1;
    localparam integer PRODW = (2*WIDTH) + 1;

    reg                     clk;
    reg                     rst_n;
    reg                     valid_in;
    reg                     clear_acc;
    reg  [WIDTH-1:0]        a, b, c;
    wire [PRODW-1:0]        y;
    wire                    valid_out;

    // DUT
    pipelined_mac #(.WIDTH(WIDTH)) dut (
        .clk(clk),
        .rst_n(rst_n),
        .valid_in(valid_in),
        .clear_acc(clear_acc),
        .a(a), .b(b), .c(c),
        .y(y),
        .valid_out(valid_out)
    );

    // Clock
    initial clk = 1'b0;
    always #5 clk = ~clk;  // 100 MHz

    // Reference model state (mirrors DUT pipeline timing)
    reg [3:0]              vld_sr;
    reg [PRODW-1:0]        prod_p0, prod_p1, prod_p2;
    reg [PRODW-1:0]        acc_ref;
    reg                    vout_ref;

    integer cycle;
    integer errors;

    // Drive stimulus and check
    initial begin
        errors   = 0;
        rst_n    = 1'b0;
        valid_in = 1'b0;
        clear_acc= 1'b0;
        a        = {WIDTH{1'b0}};
        b        = {WIDTH{1'b0}};
        c        = {WIDTH{1'b0}};

        vld_sr   = 4'b0;
        prod_p0  = {PRODW{1'b0}};
        prod_p1  = {PRODW{1'b0}};
        prod_p2  = {PRODW{1'b0}};
        acc_ref  = {PRODW{1'b0}};
        vout_ref = 1'b0;

        // Reset
        repeat (5) @(posedge clk);
        rst_n = 1'b1;

        // Run randomized sequence
        for (cycle = 0; cycle < 500; cycle = cycle + 1) begin
            // Drive next inputs before the clock edge
            valid_in = ($random % 100) < 70;        // ~70% valid
            clear_acc= (($random % 100) < 5);       // ~5% clear
            a        = $random;                     // truncated to WIDTH
            b        = $random;
            c        = $random;

            @(posedge clk);

            // Reference pipeline update (matches DUT timing)
            vld_sr  <= {vld_sr[2:0], valid_in};
            prod_p0 <= (a + b) * c;                 // product for this input
            prod_p1 <= prod_p0;
            prod_p2 <= prod_p1;

            if (clear_acc) begin
                acc_ref <= {PRODW{1'b0}};
            end else if (vld_sr[2]) begin
                acc_ref <= acc_ref + prod_p2;       // accumulate on stage 3 valid
            end

            vout_ref <= vld_sr[3];

            // Small delay for stable signals then check
            #1;
            if (valid_out !== vout_ref) begin
                $display("Mismatch valid_out at cycle %0d: DUT=%0b REF=%0b", cycle, valid_out, vout_ref);
                errors = errors + 1;
            end
            if (valid_out && (y !== acc_ref)) begin
                $display("Mismatch y at cycle %0d: DUT=%0d REF=%0d", cycle, y, acc_ref);
                errors = errors + 1;
            end
        end

        // Wrap up
        if (errors == 0) begin
            $display("PASS: No mismatches detected.");
        end else begin
            $display("FAIL: %0d mismatches detected.", errors);
            $fatal;
        end

        $finish;
    end

    initial
	begin
		$dumpfile(`DUMP_FILE_NAME);
		$dumpvars(0);		//writing the vcd file
	end
endmodule
