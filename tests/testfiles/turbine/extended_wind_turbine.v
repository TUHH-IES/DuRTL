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

    parameter NORMAL_OPERATION         = 8'b00000000,
              POWER_RAMP_UP_DOWN       = 8'b00000001,
              SYNC_CONNECT_DISCONNECT  = 8'b00000010,
              ACCEL_DECEL              = 8'b00000011,
              REDUCED_SPEED            = 8'b00000100,
              BLADE_ADJUSTMENT         = 8'b00000101,
              YAW_ALIGNMENT            = 8'b00000110,
              SHUTDOWN                 = 8'b00000111,
              SYSTEM_BOOT              = 8'b00001000,
              HARDWARE_CHECK           = 8'b00001001,
              FIRMWARE_LOAD            = 8'b00001010,
              SELF_DIAGNOSTICS         = 8'b00001011,
              ENVIRONMENTAL_CHECK      = 8'b00001100,
              WIND_SPEED_EVALUATION    = 8'b00001101,
              GRID_PRE_SYNC_CHECK      = 8'b00001110,
              BRAKE_RELEASE            = 8'b00001111,
              BEARING_WARM_UP          = 8'b00010000,
              PITCH_CALIBRATION        = 8'b00010001,
              INERTIA_SYNC             = 8'b00010010,
              VOLTAGE_REGULATION       = 8'b00010011,
              FREQUENCY_MATCHING       = 8'b00010100;

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
                    turbine_state <= FREQUENCY_MATCHING;
                else turbine_state <= NORMAL_OPERATION;
            end
            FREQUENCY_MATCHING: begin
                if (env == BLADE_ADJUSTMENT || env == REDUCED_SPEED || env == SHUTDOWN || park_command == REDUCED_SPEED || park_command == SHUTDOWN)
                    turbine_state <= VOLTAGE_REGULATION;
                else if ((env != BLADE_ADJUSTMENT && env != REDUCED_SPEED && env != SHUTDOWN) && (park_command == NORMAL_OPERATION))
                    turbine_state <= NORMAL_OPERATION;
            end
            VOLTAGE_REGULATION: begin
                if (env == BLADE_ADJUSTMENT || env == REDUCED_SPEED || env == SHUTDOWN || park_command == REDUCED_SPEED || park_command == SHUTDOWN)
                    turbine_state <= POWER_RAMP_UP_DOWN;
                else if ((env != BLADE_ADJUSTMENT && env != REDUCED_SPEED && env != SHUTDOWN) && (park_command == NORMAL_OPERATION))
                    turbine_state <= FREQUENCY_MATCHING;
            end            
            POWER_RAMP_UP_DOWN: begin
                if (env == BLADE_ADJUSTMENT || env == REDUCED_SPEED || env == SHUTDOWN || park_command == REDUCED_SPEED || park_command == SHUTDOWN)
                    turbine_state <= SYNC_CONNECT_DISCONNECT;
                else if ((env != BLADE_ADJUSTMENT && env != REDUCED_SPEED && env != SHUTDOWN) && (park_command == NORMAL_OPERATION))
                    turbine_state <= VOLTAGE_REGULATION;
            end
            SYNC_CONNECT_DISCONNECT: begin
                if (env == BLADE_ADJUSTMENT || env == REDUCED_SPEED || env == SHUTDOWN || park_command == REDUCED_SPEED || park_command == SHUTDOWN)
                    turbine_state <= INERTIA_SYNC;
                else if ((env != BLADE_ADJUSTMENT && env != REDUCED_SPEED && env != SHUTDOWN) && (park_command == NORMAL_OPERATION))
                    turbine_state <= POWER_RAMP_UP_DOWN;
            end
            INERTIA_SYNC: begin
                if (env == BLADE_ADJUSTMENT || env == REDUCED_SPEED || env == SHUTDOWN || park_command == REDUCED_SPEED || park_command == SHUTDOWN)
                    turbine_state <= REDUCED_SPEED;
                else if ((env != BLADE_ADJUSTMENT && env != REDUCED_SPEED && env != SHUTDOWN) && (park_command == NORMAL_OPERATION))
                    turbine_state <= SYNC_CONNECT_DISCONNECT;
            end
            REDUCED_SPEED: begin
                if (env == REDUCED_SPEED || park_command == REDUCED_SPEED)
                    turbine_state <= REDUCED_SPEED;
                else if (env == SHUTDOWN || env == BLADE_ADJUSTMENT || park_command == SHUTDOWN)
                    turbine_state <= ACCEL_DECEL;
                else if (park_command == NORMAL_OPERATION)
                    turbine_state <= INERTIA_SYNC;
            end
            ACCEL_DECEL: begin
                if (env == BLADE_ADJUSTMENT ||  env == SHUTDOWN || park_command == SHUTDOWN)
                    turbine_state <= PITCH_CALIBRATION;
                else if(env == REDUCED_SPEED || park_command == REDUCED_SPEED  || park_command == NORMAL_OPERATION)
                    turbine_state <= REDUCED_SPEED;
            end
            PITCH_CALIBRATION: begin
                if (env == BLADE_ADJUSTMENT ||  env == SHUTDOWN || park_command == SHUTDOWN)
                    turbine_state <= BEARING_WARM_UP;
                else if(env == REDUCED_SPEED || park_command == REDUCED_SPEED  || park_command == NORMAL_OPERATION)
                    turbine_state <= ACCEL_DECEL;
            end
            BEARING_WARM_UP: begin
                if (env == BLADE_ADJUSTMENT ||  env == SHUTDOWN || park_command == SHUTDOWN)
                    turbine_state <= BRAKE_RELEASE;
                else if(env == REDUCED_SPEED || park_command == REDUCED_SPEED  || park_command == NORMAL_OPERATION)
                    turbine_state <= PITCH_CALIBRATION;
            end
            BRAKE_RELEASE: begin
                if (env == BLADE_ADJUSTMENT ||  env == SHUTDOWN || park_command == SHUTDOWN)
                    turbine_state <= GRID_PRE_SYNC_CHECK;
                else if(env == REDUCED_SPEED || park_command == REDUCED_SPEED  || park_command == NORMAL_OPERATION)
                    turbine_state <= BEARING_WARM_UP;
            end
            GRID_PRE_SYNC_CHECK: begin
                if (env == BLADE_ADJUSTMENT ||  env == SHUTDOWN || park_command == SHUTDOWN)
                    turbine_state <= BLADE_ADJUSTMENT;
                else if(env == REDUCED_SPEED || park_command == REDUCED_SPEED  || park_command == NORMAL_OPERATION)
                    turbine_state <= BRAKE_RELEASE;
            end
            BLADE_ADJUSTMENT: begin
                if (env == BLADE_ADJUSTMENT)
                    turbine_state <= BLADE_ADJUSTMENT;
                else if ((env != BLADE_ADJUSTMENT) && (park_command == NORMAL_OPERATION || park_command == REDUCED_SPEED))
                    turbine_state <= GRID_PRE_SYNC_CHECK;
                else if ((env != BLADE_ADJUSTMENT) && (park_command == SHUTDOWN || env == SHUTDOWN))
                    turbine_state <= YAW_ALIGNMENT;
            end
            YAW_ALIGNMENT: begin
                if (env == REDUCED_SPEED || env == BLADE_ADJUSTMENT || park_command == NORMAL_OPERATION || park_command == REDUCED_SPEED)
                    turbine_state <= BLADE_ADJUSTMENT;
                else if ((env == SHUTDOWN || park_command == SHUTDOWN))
                    turbine_state <= WIND_SPEED_EVALUATION;
            end
            WIND_SPEED_EVALUATION: begin
                if (env == REDUCED_SPEED || env == BLADE_ADJUSTMENT || park_command == NORMAL_OPERATION || park_command == REDUCED_SPEED)
                    turbine_state <= YAW_ALIGNMENT;
                else if ((env == SHUTDOWN || park_command == SHUTDOWN))
                    turbine_state <= ENVIRONMENTAL_CHECK;
            end
            ENVIRONMENTAL_CHECK: begin
                if (env == REDUCED_SPEED || env == BLADE_ADJUSTMENT || park_command == NORMAL_OPERATION || park_command == REDUCED_SPEED)
                    turbine_state <= WIND_SPEED_EVALUATION;
                else if ((env == SHUTDOWN || park_command == SHUTDOWN))
                    turbine_state <= SELF_DIAGNOSTICS;
            end
            SELF_DIAGNOSTICS: begin
                if (env == REDUCED_SPEED || env == BLADE_ADJUSTMENT || park_command == NORMAL_OPERATION || park_command == REDUCED_SPEED)
                    turbine_state <= ENVIRONMENTAL_CHECK;
                else if ((env == SHUTDOWN || park_command == SHUTDOWN))
                    turbine_state <= FIRMWARE_LOAD;
            end
            FIRMWARE_LOAD: begin
                if (env == REDUCED_SPEED || env == BLADE_ADJUSTMENT || park_command == NORMAL_OPERATION || park_command == REDUCED_SPEED)
                    turbine_state <= SELF_DIAGNOSTICS;
                else if ((env == SHUTDOWN || park_command == SHUTDOWN))
                    turbine_state <= HARDWARE_CHECK;
            end
            HARDWARE_CHECK: begin
                if (env == REDUCED_SPEED || env == BLADE_ADJUSTMENT || park_command == NORMAL_OPERATION || park_command == REDUCED_SPEED)
                    turbine_state <= FIRMWARE_LOAD;
                else if ((env == SHUTDOWN || park_command == SHUTDOWN))
                    turbine_state <= SYSTEM_BOOT;
            end
             SYSTEM_BOOT: begin
                if (env == REDUCED_SPEED || env == BLADE_ADJUSTMENT || park_command == NORMAL_OPERATION || park_command == REDUCED_SPEED)
                    turbine_state <= HARDWARE_CHECK;
                else if ((env == SHUTDOWN || park_command == SHUTDOWN))
                    turbine_state <= SHUTDOWN;
            end
            SHUTDOWN: begin
                if (park_command == SHUTDOWN || env == SHUTDOWN)
                    turbine_state <= SHUTDOWN;
                else if (env == BLADE_ADJUSTMENT || env == REDUCED_SPEED || park_command == NORMAL_OPERATION || park_command == REDUCED_SPEED)
                    turbine_state <= SYSTEM_BOOT;
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
