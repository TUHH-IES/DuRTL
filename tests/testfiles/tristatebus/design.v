module TriStateBuffer (
    input wire enable,
    input wire [7:0] data_in,
    output reg [7:0] data_out
);
    always @ (enable)
    begin
        if (enable)
            data_out <= data_in;
        else
            data_out <= 8'bz;
    end
endmodule

module Client (
    input wire clk,
    input wire [7:0] bus_data_in,
    output wire [7:0] bus_data_out,
    input wire client_write_enable,
    input wire server_write_enable,
    input wire reset
);
    reg [7:0] reg_data;

    TriStateBuffer buffer (
        .enable(client_write_enable),
        .data_in(reg_data),
        .data_out(bus_data_out)
    );

    always @(posedge clk or posedge reset)
    begin
        if (reset)
            reg_data <= 8'h00;
        else if (server_write_enable)
            reg_data <= bus_data_in;
    end
endmodule

module Server (
    input wire clk,
    output wire [7:0] bus_data_out,
    input wire server_write_enable,
    input wire reset
);
    reg [7:0] reg_data;

    always @(posedge clk or posedge reset)
    begin
        if (reset)
            reg_data <= 8'h00;
        else
            reg_data <= reg_data + 1;
    end

    TriStateBuffer buffer (
        .enable(server_write_enable),
        .data_in(reg_data),
        .data_out(bus_data_out)
    );
endmodule

module TopModule(
    // Clock signal
    input wire clk,
    // Bus signals
    output wire [7:0] bus_data,
    input wire client_write_enable,
    input wire server_write_enable,
    // Reset signal
    input wire reset
  );


    // Instantiate Server module
    Server server_inst (
        .clk(clk),
        .bus_data_out(bus_data),
        .server_write_enable(server_write_enable),
        .reset(reset)
    );

    // Instantiate Client module
    Client client_inst (
        .clk(clk),
        .bus_data_out(bus_data),
        .bus_data_in(bus_data),
        .client_write_enable(client_write_enable),
        .server_write_enable(server_write_enable),
        .reset(reset)
    );


endmodule
