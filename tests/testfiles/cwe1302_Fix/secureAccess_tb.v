module TopModule_tb;

    reg clk;
    reg rst_n;
    reg [31:0] inputData_i;
    reg [1:0] requestUsecase_i;
    reg [31:0] referenceData_i;
    wire isSecure_o;

    secureInterface uut (
        .clk(clk),
        .rst_n(rst_n),
        .requestUsecase_i(requestUsecase_i),
        .inputData_i(inputData_i),
        .referenceData_i(referenceData_i),
        .validIsSecure_o(validIsSecure_o),
        .isSecure_o(isSecure_o)
    );

        initial begin
            clk = 1;
            // Initialize inputs
            rst_n = 0;
            inputData_i = 32'h00000000;
            referenceData_i = 32'h00000000;
            requestUsecase_i = 2'b0;

            // reset
            #10;
            rst_n = 1;

            // Test case 1: Valid data
            inputData_i = 32'h10010011;
            referenceData_i = 32'hcebdaeff; // Expected hash value based on the hash function, intermediate value - ceacbefe
            requestUsecase_i = 2'b10;
            #70;

            // end - change based on number of simulations
            #100;
            $finish;
        end

always clk = #5 ~clk;

initial
    begin
        $dumpfile(`DUMP_FILE_NAME);
        $dumpvars; //writing the vcd file
    end
endmodule