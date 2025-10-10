// add_then_mul_pipeline.v
module ff_design #(
    parameter integer W = 8  // bit-width of a, b, c
)(
    input  wire                  clk,
    input  wire                  rst_n,     // active-low synchronous reset

    input  wire                  in_valid,  // marks inputs a,b,c as valid this cycle
    input  wire [W-1:0]          a,
    input  wire [W-1:0]          b,
    input  wire [W-1:0]          c,

    output reg                   out_valid, // valid one cycle after in_valid
    output reg  [(2*W):0]        y          // (W+1) + W bits = 2*W+1 bits
);

    // Stage 1: add (combinational), then register results
    wire [W:0] sum_wide = a + b; // W+1 bits to hold carry

    reg  [W:0] sum_r;            // pipeline register for sum
    reg  [W-1:0] c_r;            // pipeline register for c
    reg          valid_r;        // pipeline register for valid

    always @(posedge clk) begin
        if (!rst_n) begin
            sum_r   <= 0;
            c_r     <= 0;
            valid_r <= 1'b0;
        end else begin
            sum_r   <= sum_wide;
            c_r     <= c;
            valid_r <= in_valid;
        end
    end

    // Stage 2: multiply registered operands, register output
    wire [(2*W):0] prod_wide = sum_r * c_r; // (W+1) * W -> 2*W+1 bits

    always @(posedge clk) begin
        if (!rst_n) begin
            y         <= 0;
            out_valid <= 1'b0;
        end else begin
            y         <= prod_wide;
            out_valid <= valid_r;
        end
    end

    // Timing summary (per rising edge at time t):
    //   sum_r[t+1]   = a[t] + b[t]
    //   c_r[t+1]     = c[t]
    //   y[t+1]       = sum_r[t+1] * c_r[t+1] = (a[t] + b[t]) * c[t]
    //   out_valid[t+1] = in_valid[t]
endmodule

