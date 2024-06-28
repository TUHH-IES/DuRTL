module aes_module_t (in, out);
    input  [7:0]  in;
    output [31:0] out;

    S s0 (in, out[31:24]);
    assign out[23:16] = out[31:24];

    S s4 (in, out[7:0]);
    assign out[15:8] = out[23:16] ^ out[7:0];
endmodule

module S (in, out);

input [7:0] in;
output [7:0] out;

assign out = in;

endmodule
