module Environmental_Monitor (
    input wire [7:0] vibration_level,
    input wire [7:0] temperature,
    input wire storm_detected,
    input wire clk,
    input wire reset,
    output reg [1:0] env_action,
    output reg warning,
    output reg shutdown
);

    // State Definitions
    localparam IDLE    = 2'b00;
    localparam WARNING = 2'b01;
    localparam ALERT   = 2'b10;

    // State and Next State Signals
    reg [1:0] state, next_state;

    // State Transition Logic
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            state <= IDLE;
        end else begin
            state <= next_state;
        end
    end

    // Next State Logic
    always @(*) begin
        case (state)
            IDLE: begin
                if ((vibration_level > 8'd100) || (temperature > 8'd70)) begin
                    next_state = WARNING;
                end else if (storm_detected) begin
                    next_state = ALERT;
                end else begin
                    next_state = IDLE;
                end
            end

            WARNING: begin
                if (storm_detected || (vibration_level > 8'd120)) begin
                    next_state = ALERT;
                end else if ((vibration_level <= 8'd100) && (temperature <= 8'd70)) begin
                    next_state = IDLE;
                end else begin
                    next_state = WARNING;
                end
            end

            ALERT: begin
                if (!storm_detected && (vibration_level <= 8'd100) && (temperature <= 8'd70)) begin
                    next_state = IDLE;
                end else begin
                    next_state = ALERT;
                end
            end

            default: next_state = IDLE;
        endcase
    end

    // Output Logic
    always @(*) begin
        case (state)
            IDLE: begin
                env_action = 2'b00;
                warning = 1'b0;
                shutdown = 1'b0;
            end

            WARNING: begin
                env_action = 2'b01;
                warning = 1'b1;
                shutdown = 1'b0;
            end

            ALERT: begin
                env_action = 2'b10;
                warning = 1'b1;
                shutdown = 1'b1;
            end

            default: begin
                env_action = 2'b00;
                warning = 1'b0;
                shutdown = 1'b0;
            end
        endcase
    end

endmodule

