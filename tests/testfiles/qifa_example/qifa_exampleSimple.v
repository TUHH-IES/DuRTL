module qifa_exampleSimple (
    input logic clk,
    input logic rst,
    input logic [2:0] secret,
    input logic [1:0] inp,
    output logic [5:0] out
);
    logic [5:0] state;

    always_ff @(posedge clk) begin
        if (rst)
            state <= 6'b000000;
        else
            if (inp==0) begin
              if (secret<3)
                state <= 2;
              else 
                state <= 7;
            end else
              state <= state+inp;
    end

    assign out = state;

endmodule
