module qifa_exampleSimple (
    input logic [1:0] secret,
    input logic [1:0] inp,
    output logic [1:0] out
);

    assign out = (inp==0? ( secret<2? 0 : 3 ) : 2 );

endmodule
