module secureInterface (input clk,
                        input rst_n,
                        input [1:0] requestUsecase_i,
                        input [31:0] inputData_i,
                        input [31:0] referenceData_i,
                        output validIsSecure_o,
                        output isSecure_o);

    securityModule sm(clk, rst_n, requestUsecase_i, inputData_i, referenceData_i, validIsSecure_o, isSecure_o);

endmodule

module securityModule(input clk,
                      input rst_n,
                      input [1:0] requestUsecase_i,
                      input [31:0] inputData_i,
                      input [31:0] referenceData_i,
                      output reg validIsSecure_o,
                      output reg isSecure_o);

    wire [31:0]grantAccessUsecase_o;
    wire enable_o;
    wire valid_o;
    wire [31:0] computedData_o;

    assignerCoreSec ac(clk, rst_n, requestUsecase_i, enable_o, grantAccessUsecase_o);
    secureCoreSec sc(clk, rst_n, requestUsecase_i, inputData_i, enable_o, grantAccessUsecase_o, valid_o, computedData_o);

    always @ (*)
    begin
        if (!rst_n)
            begin
                isSecure_o <= 0;
                validIsSecure_o <= 0;
            end
        else
            begin
                if(valid_o)
                    begin
                        isSecure_o <= (computedData_o == referenceData_i);
                        validIsSecure_o <= 1;
                    end
                else
                    begin
                        isSecure_o <= 0;
                        validIsSecure_o <= 0;
                    end
            end
    end

endmodule

module assignerCoreSec (input clk,
                     input rst_n,
                     input [1:0] requestAccessUsecase_i,
                     output reg enable_o,
                     output reg [31:0] grantAccessUsecase_o);

    reg [31:0] grantAccessUsecase_lcl;

    always @ (*)
    begin
        case(requestAccessUsecase_i)
            //AES
            2'b01   : grantAccessUsecase_lcl = 32'hA5A5A5A5;
            //RSA
            2'b10    : grantAccessUsecase_lcl = 32'hDEADBEEF;
            //HMAC
            2'b11    : grantAccessUsecase_lcl = 32'hA3A3A3A3;
            //No access
            default  : grantAccessUsecase_lcl = 32'b0;
        endcase
    end

    always @ (posedge clk)
    begin
        if (!rst_n)
            begin
                enable_o <= 0;
                grantAccessUsecase_o <= 32'b0;
            end
        else
	        begin
                enable_o <= 1;
                grantAccessUsecase_o <= grantAccessUsecase_lcl;
	        end
    end
endmodule

module secureCoreSec(input clk,
                  input rst_n,
                  input [1:0] requestUsecase_i,
                  input [31:0] inputData_i,
                  input valid_i,
                  input [31:0] secretData_i,
                  output reg valid_o,
                  output reg [31:0] computedData_o);

    wire validAes_o, validRsa_o;
    reg readyAes, readyRsa, readyDS;
    wire [31:0] computedDataAes_o;
    wire [31:0] computedDataRsa_o;
    reg [31:0] secretDataRsa,secretDataAes;

    always @ (posedge clk)
    begin
        if (!rst_n)
            begin
                readyAes <= 0;
                readyRsa <= 0;
                readyDS <= 0;
                secretDataAes <= 32'b0;
                secretDataRsa <= 32'b0;
            end
        else
	        begin
                if(valid_i)
                    begin
                        case(requestUsecase_i)
                            //AES
                            2'b01   : begin
                                       readyAes <= 1;
                                       readyRsa <= 0;
                                       readyDS <= 0;
                                       secretDataAes <= secretData_i;
                                       secretDataRsa <= 32'b0;
                                      end
                            //RSA
                            2'b10    : begin
                                        readyAes <= 0;
                                        readyRsa <= 1;
                                        readyDS <= 0;
                                        secretDataAes <= 32'b0;
                                        secretDataRsa <= secretData_i;
                                       end
                            //HMAC
                            2'b11    : begin
                                        readyAes <= 0;
                                        readyRsa <= 0;
                                        readyDS <= 1;
                                        secretDataAes <= 32'b0;
                                        secretDataRsa <= 32'b0;
                                       end
                            //HMAC
                            2'b00    : begin
                                        readyAes <= 0;
                                        readyRsa <= 0;
                                        readyDS <= 0;
                                        secretDataAes <= 32'b0;
                                        secretDataRsa <= 32'b0;
                                       end
                            //No access
                            default  : begin
                                        readyAes <= 0;
                                        readyRsa <= 0;
                                        readyDS <= 0;
                                        secretDataAes <= 32'b0;
                                        secretDataRsa <= 32'b0;
                                       end
                        endcase
                    end
                else
                    begin
                        readyAes <= 0;
                        readyRsa <= 0;
                        readyDS <= 0;
                    end
	        end
    end

    always @ (*)
    begin
        if (!rst_n)
            begin
                valid_o <= 0;
                computedData_o <= 0;
            end
        else
	        begin
                if (validAes_o && readyAes)
                    begin
                        valid_o <= validAes_o;
                        computedData_o <= computedDataAes_o;
                    end
                else if (validRsa_o && readyRsa)
                    begin
                        valid_o <= validRsa_o;
                        computedData_o <= computedDataRsa_o;
                    end
                else
                    begin
                        valid_o <= 0;
                        computedData_o <= 0;
                    end
	        end
    end

    AESCoreSec ec(clk, rst_n, inputData_i, readyAes, secretDataAes, computedDataAes_o, validAes_o);

    RSACoreSec hc(clk, rst_n, inputData_i, readyRsa, secretDataRsa, computedDataRsa_o, validRsa_o);

endmodule

module AESCoreSec(input clk,
                      input rst_n,
                      input [31:0] inputData_i,
                      input readyAes,
                      input [31:0] secretData_i,
                      output reg [31:0] computedDataAes_o,
                      output reg validAes_o);

    always @(posedge clk) begin
        if (!rst_n) begin
            computedDataAes_o <= 32'b0;
            validAes_o <= 0;
        end else begin
            // Simple encryption: XOR
            if(readyAes)
                begin
                    computedDataAes_o <= inputData_i ^ secretData_i;
                    validAes_o <= 1;
                end
            else
                begin
                    computedDataAes_o <= 0;
                    validAes_o <= 0;
                end
        end
    end
endmodule

module RSACoreSec(input clk,
                input rst_n,
                input [31:0] inputData_i,
                input readyRsa,
                input [31:0] secretData_i,
                output reg [31:0] computedDataRsa_o,
                output reg validRsa_o);

    always @(posedge clk) begin
        if (!rst_n) begin
            computedDataRsa_o <= 32'b0;
            validRsa_o <= 0;
        end else begin
            // Simple RSA encryption implementation: XOR and rotate bits
            if(readyRsa)
                begin
                    computedDataRsa_o <= (inputData_i ^ secretData_i) ^ {inputData_i[15:0], inputData_i[31:16]};
                    validRsa_o <= 1;
                end
            else
                begin
                    computedDataRsa_o <= 0;
                    validRsa_o <= 0;
                end
        end
    end
endmodule