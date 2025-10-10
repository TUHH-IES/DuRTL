// pipelined_mac.v
module pipelined_mac #(
    parameter integer WIDTH = 8
)(
    input  wire                     clk,
    input  wire                     rst_n,      // active-low synchronous reset
    input  wire                     valid_in,
    input  wire                     clear_acc,  // synchronous clear of accumulator
    input  wire [WIDTH-1:0]         a,
    input  wire [WIDTH-1:0]         b,
    input  wire [WIDTH-1:0]         c,
    output reg  [(2*WIDTH):0]       y,          // accumulator width: 2*WIDTH+1
    output wire                     valid_out
);
    localparam integer SUMW  = WIDTH + 1;
    localparam integer PRODW = (2*WIDTH) + 1;

    // Pipeline registers (5+ flip-flops across these regs)
    reg [WIDTH-1:0]   a_r, b_r, c_r;      // stage 0 inputs
    reg [SUMW-1:0]    sum_r;              // stage 1 sum = a+b
    reg [PRODW-1:0]   mult_r;             // stage 2 mult = sum*c
    reg [PRODW-1:0]   acc_r;              // stage 3 accumulator
    reg [3:0]         valid_sr;           // valid shift register

    assign valid_out = valid_sr[3];

    always @(posedge clk) begin
        if (!rst_n) begin
            a_r      <= {WIDTH{1'b0}};
            b_r      <= {WIDTH{1'b0}};
            c_r      <= {WIDTH{1'b0}};
            sum_r    <= {SUMW{1'b0}};
            mult_r   <= {PRODW{1'b0}};
            acc_r    <= {PRODW{1'b0}};
            valid_sr <= 4'b0000;
            y        <= {PRODW{1'b0}};
        end else begin
            // Stage 0: capture inputs
            a_r      <= a;
            b_r      <= b;
            c_r      <= c;

            // Valid pipeline
            valid_sr <= {valid_sr[2:0], valid_in};

            // Stage 1: sum
            sum_r    <= a_r + b_r;

            // Stage 2: multiply
            mult_r   <= sum_r * c_r;

            // Stage 3: accumulate when the corresponding data reaches stage 3
            if (clear_acc) begin
                acc_r <= {PRODW{1'b0}};
            end else if (valid_sr[2]) begin
                acc_r <= acc_r + mult_r;
            end

            // Output register (aligned with valid_out, one stage after accumulate)
            y <= acc_r;
        end
    end
endmodule
