`timescale 1ns/1ps
module ComposedSystem (
    input clk,
    input rst,
    input [7:0] wind_speed,
    input [7:0] power,
    input [7:0] power_need,
    output [7:0] turbine_state,
    output [7:0] park_command
);

    wire [7:0] turbine_state_internal;
    wire [7:0] park_command_internal;

    WindTurbine turbine (
        .clk(clk),
        .rst(rst),
        .wind_speed(wind_speed),
        .power(power),
        .park_command(park_command_internal),
        .turbine_state(turbine_state_internal)
    );

    WindPark park (
        .clk(clk),
        .rst(rst),
        .power_need(power_need),
        .park_command(park_command_internal)
    );

    assign turbine_state = turbine_state_internal;
    assign park_command = park_command_internal;

endmodule

module WindTurbine (
    input clk,
    input rst,
    input [7:0] wind_speed,
    input [7:0] power,
    input [7:0] park_command,
    output reg [7:0] turbine_state
);

    parameter NORMAL_OPERATION    = 8'b00000000,
              POWER_RAMP_UP_DOWN  = 8'b00000001,
              SYNC_CONNECT_DISCONNECT = 8'b00000010,
              ACCEL_DECEL         = 8'b00000011,
              REDUCED_SPEED       = 8'b00000100,
              BLADE_ADJUSTMENT    = 8'b00000101,
              YAW_ALIGNMENT       = 8'b00000110,
              SHUTDOWN            = 8'b00000111;

    reg [7:0] env;

    always @(posedge clk or posedge rst) begin
        if (rst)
            turbine_state <= SHUTDOWN;
        else begin
        if (power == 8'b00000000)
            env <= SHUTDOWN;
        else if (power <= 8'b00001010)
            env <= REDUCED_SPEED;
        else if (wind_speed > 8'b01100100)
            env <= BLADE_ADJUSTMENT;
        case (turbine_state)
            NORMAL_OPERATION: begin
                if (env == BLADE_ADJUSTMENT || env == REDUCED_SPEED || env == SHUTDOWN || park_command == REDUCED_SPEED || park_command == SHUTDOWN)
                    turbine_state <= POWER_RAMP_UP_DOWN;
                else turbine_state <= NORMAL_OPERATION;
            end
            POWER_RAMP_UP_DOWN: begin
                if (env == BLADE_ADJUSTMENT || env == REDUCED_SPEED || env == SHUTDOWN || park_command == REDUCED_SPEED || park_command == SHUTDOWN)
                    turbine_state <= SYNC_CONNECT_DISCONNECT;
                else if ((env != BLADE_ADJUSTMENT && env != REDUCED_SPEED && env != SHUTDOWN) && (park_command == NORMAL_OPERATION))
                    turbine_state <= NORMAL_OPERATION;
            end
            SYNC_CONNECT_DISCONNECT: begin
                if (env == BLADE_ADJUSTMENT || env == REDUCED_SPEED || env == SHUTDOWN || park_command == REDUCED_SPEED || park_command == SHUTDOWN)
                    turbine_state <= ACCEL_DECEL;
                else if ((env != BLADE_ADJUSTMENT && env != REDUCED_SPEED && env != SHUTDOWN) && (park_command == NORMAL_OPERATION))
                    turbine_state <= POWER_RAMP_UP_DOWN;
            end
            ACCEL_DECEL: begin
                if (env == BLADE_ADJUSTMENT || env == REDUCED_SPEED || env == SHUTDOWN || park_command == REDUCED_SPEED || park_command == SHUTDOWN)
                    turbine_state <= REDUCED_SPEED;
                else if ((env != BLADE_ADJUSTMENT && env != REDUCED_SPEED && env != SHUTDOWN) && (park_command == NORMAL_OPERATION))
                    turbine_state <= SYNC_CONNECT_DISCONNECT;
            end
            REDUCED_SPEED: begin
                if (env == REDUCED_SPEED || park_command == REDUCED_SPEED)
                    turbine_state <= REDUCED_SPEED;
                else if (env == SHUTDOWN || env == BLADE_ADJUSTMENT || park_command == SHUTDOWN)
                    turbine_state <= BLADE_ADJUSTMENT;
                else if (park_command == NORMAL_OPERATION)
                    turbine_state <= ACCEL_DECEL;
            end
            BLADE_ADJUSTMENT: begin
                if (env == BLADE_ADJUSTMENT)
                    turbine_state <= BLADE_ADJUSTMENT;
                else if ((env != BLADE_ADJUSTMENT) && (park_command == NORMAL_OPERATION || park_command == REDUCED_SPEED))
                    turbine_state <= REDUCED_SPEED;
                else if ((env != BLADE_ADJUSTMENT) && (park_command == SHUTDOWN || env == SHUTDOWN))
                    turbine_state <= YAW_ALIGNMENT;
            end
            YAW_ALIGNMENT: begin
                if (env == REDUCED_SPEED || env == BLADE_ADJUSTMENT || park_command == NORMAL_OPERATION || park_command == REDUCED_SPEED)
                    turbine_state <= BLADE_ADJUSTMENT;
                else if ((env == SHUTDOWN || park_command == SHUTDOWN))
                    turbine_state <= SHUTDOWN;
            end
            SHUTDOWN: begin
                if (park_command == SHUTDOWN || env == SHUTDOWN)
                    turbine_state <= SHUTDOWN;
                else if (env == BLADE_ADJUSTMENT || env == REDUCED_SPEED || park_command == NORMAL_OPERATION || park_command == REDUCED_SPEED)
                    turbine_state <= YAW_ALIGNMENT;
            end
        endcase
    end
    end
endmodule

module WindPark (
    input clk,
    input rst,
    input [7:0] power_need,
    output reg [7:0] park_command
);

    parameter NORMAL_OPERATION = 8'b00000000,
              REDUCED_SPEED    = 8'b00000100,
              SHUTDOWN         = 8'b00000111;

    always @(posedge clk or posedge rst) begin
        if (rst)
            park_command <= NORMAL_OPERATION;
        else begin
            case (power_need)
                8'b00000000: park_command <= SHUTDOWN;
                8'b00000001: park_command <= REDUCED_SPEED;
                8'b00000010: park_command <= NORMAL_OPERATION;
                default: park_command <= NORMAL_OPERATION;
            endcase
        end
    end
endmodule
