module testbench;

    reg clk;
    reg reset;

    // Bus signals
    wire [7:0] bus_data;
    reg client_write_enable, server_write_enable;

    // Instantiate the top module
    TopModule top_module_inst (
        .clk(clk),
        .bus_data(bus_data),
        .client_write_enable(client_write_enable),
        .server_write_enable(server_write_enable),
        .reset(reset)
    );

    // Clock generation
    always #5 clk = ~clk;

    // Reset generation
    initial begin
        client_write_enable = 0;
        server_write_enable = 0;
        clk = 0;
        reset = 1;
        #10 reset = 0;
        server_write_enable = 1;
        #20 reset = 0;
        server_write_enable = 0;
        #20 reset = 0;
        client_write_enable = 1;
        #10 reset = 0;
        client_write_enable = 0;
        server_write_enable = 1;
        #20 reset = 0;
        server_write_enable = 0;
        #20 reset = 0;
        client_write_enable = 1;
    end

        // Dump VCD file
    initial begin
        $dumpfile("trace.vcd");
        $dumpvars(0, testbench);
        #1000 $finish;
    end

endmodule
