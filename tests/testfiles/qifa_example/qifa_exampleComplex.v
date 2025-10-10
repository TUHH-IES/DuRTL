module seq_example (
    input logic clk,
    input logic rst,
    input logic [2:0] secret,
    input logic [1:0] inp,
    output logic [1:0] out,
    output logic [5:0] state
);

    logic [5:0] temp;
    //logic [5:0] state;

    always_ff @(posedge clk or posedge rst) begin
        if (rst)
            state <= 6'b000000;
        else
            state <= state + secret + inp;
    end

    assign temp= (state ^ secret);

    assign out = {temp[5], temp[3]};

endmodule
