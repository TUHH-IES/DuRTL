module qifa_exampleSimple (
    input logic [1:0] secret,
    input logic [1:0] inp,
    output logic [1:0] out
);

    assign out = inp+secret;

endmodule
