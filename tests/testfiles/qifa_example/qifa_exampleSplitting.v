module qifa_exampleSimple (
    input logic [2:0] secret,
    input logic [5:0] inp,
    output logic [5:0] out
);

    assign out= (inp ^ secret);


endmodule
