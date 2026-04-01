

`timescale 1ns/1ps

module ComposedSystem (
    input clk,
    input rst,
    input ped1, ped2, ped3, ped4, ped5, ped6, ped7, ped8, ped9, ped10,               
    input occ1, occ2, occ3, occ4, occ5, occ6, occ7, occ8, occ9, occ10,               
    input friction_ok1, friction_ok2, friction_ok3, friction_ok4, friction_ok5, friction_ok6, friction_ok7, friction_ok8, friction_ok9, friction_ok10,
    input battery_ok1, battery_ok2, battery_ok3, battery_ok4, battery_ok5, battery_ok6, battery_ok7, battery_ok8, battery_ok9, battery_ok10, 
    input [7:0] traffic_need,        // external demand (maps to CTM commands)
    output [7:0] vehicle1_state,
    output [7:0] vehicle2_state,
    output [7:0] vehicle3_state,
    output [7:0] vehicle4_state,
    output [7:0] vehicle5_state,
    output [7:0] vehicle6_state,
    output [7:0] vehicle7_state,
    output [7:0] vehicle8_state,
    output [7:0] vehicle9_state,
    output [7:0] vehicle10_state,
    output [7:0] traffic_command
);

    AutonomousVehicle av1 (
        .clk(clk),
        .rst(rst),
        .ped(ped1),
        .occ(occ1),
        .friction_ok(friction_ok1),
        .battery_ok(battery_ok1),
        .traffic_command(traffic_command),
        .vehicle_state(vehicle1_state)
    );

    AutonomousVehicle av2 (
        .clk(clk),
        .rst(rst),
        .ped(ped2),
        .occ(occ2),
        .friction_ok(friction_ok2),
        .battery_ok(battery_ok2),
        .traffic_command(traffic_command),
        .vehicle_state(vehicle2_state)
    );

    AutonomousVehicle av3 (
        .clk(clk),
        .rst(rst),
        .ped(ped3),
        .occ(occ3),
        .friction_ok(friction_ok3),
        .battery_ok(battery_ok3),
        .traffic_command(traffic_command),
        .vehicle_state(vehicle3_state)
    );

    AutonomousVehicle av4 (
        .clk(clk),
        .rst(rst),
        .ped(ped4),
        .occ(occ4),
        .friction_ok(friction_ok4),
        .battery_ok(battery_ok4),
        .traffic_command(traffic_command),
        .vehicle_state(vehicle4_state)
    );

    AutonomousVehicle av5 (
        .clk(clk),
        .rst(rst),
        .ped(ped5),
        .occ(occ5),
        .friction_ok(friction_ok5),
        .battery_ok(battery_ok5),
        .traffic_command(traffic_command),
        .vehicle_state(vehicle5_state)
    );

    AutonomousVehicle av6 (
        .clk(clk),
        .rst(rst),
        .ped(ped6),
        .occ(occ6),
        .friction_ok(friction_ok6),
        .battery_ok(battery_ok6),
        .traffic_command(traffic_command),
        .vehicle_state(vehicle6_state)
    );

    AutonomousVehicle av7 (
        .clk(clk),
        .rst(rst),
        .ped(ped7),
        .occ(occ7),
        .friction_ok(friction_ok7),
        .battery_ok(battery_ok7),
        .traffic_command(traffic_command),
        .vehicle_state(vehicle7_state)
    );

    AutonomousVehicle av8 (
        .clk(clk),
        .rst(rst),
        .ped(ped8),
        .occ(occ8),
        .friction_ok(friction_ok8),
        .battery_ok(battery_ok8),
        .traffic_command(traffic_command),
        .vehicle_state(vehicle8_state)
    );

    AutonomousVehicle av9 (
        .clk(clk),
        .rst(rst),
        .ped(ped9),
        .occ(occ9),
        .friction_ok(friction_ok9),
        .battery_ok(battery_ok9),
        .traffic_command(traffic_command),
        .vehicle_state(vehicle9_state)
    );

    AutonomousVehicle av10 (
        .clk(clk),
        .rst(rst),
        .ped(ped10),
        .occ(occ10),
        .friction_ok(friction_ok10),
        .battery_ok(battery_ok10),
        .traffic_command(traffic_command),
        .vehicle_state(vehicle10_state)
    );

    TrafficController ctm (
        .clk(clk),
        .rst(rst),
        .traffic_need(traffic_need),
        .traffic_command(traffic_command)
    );

endmodule

module AutonomousVehicle (
    input clk,
    input rst,
    input ped,
    input occ,
    input friction_ok,
    input battery_ok,
    input [7:0] traffic_command,
    output reg [7:0] vehicle_state
);

    
    parameter PROCEED  = 8'b00000000,
              SLOW     = 8'b00000001,
              STOP     = 8'b00000010;

    
    parameter IDLE        = 8'b00010000,
              ALIGN       = 8'b00010001,
              GAP_ASSESS  = 8'b00010010,
              ACCEL       = 8'b00010011,
              SYNC_MERGE  = 8'b00010100,
              CRUISE      = 8'b00010101,
              CRAWL       = 8'b00010110,
              HOLD        = 8'b00010111,
              BRAKE       = 8'b00011000;

    always @(posedge clk or posedge rst) begin
        if (rst)
            vehicle_state <= IDLE;
        else begin
            case (vehicle_state)
                IDLE: begin
                    if (traffic_command == PROCEED && !ped && !occ && friction_ok && battery_ok)
                        vehicle_state <= ALIGN;
                    else if (traffic_command == STOP || ped || occ || !friction_ok || !battery_ok)
                        vehicle_state <= BRAKE;
                    else if (traffic_command == SLOW)
                        vehicle_state <= CRAWL;
                end
                ALIGN: begin
                    if (traffic_command == STOP || ped || occ || !friction_ok || !battery_ok)
                        vehicle_state <= BRAKE;
                    else
                        vehicle_state <= GAP_ASSESS;
                end
                GAP_ASSESS: begin
                    if (traffic_command == STOP || ped || occ || !friction_ok || !battery_ok)
                        vehicle_state <= HOLD;
                    else
                        vehicle_state <= ACCEL;
                end
                ACCEL: begin
                    if (traffic_command == STOP || ped || occ || !friction_ok || !battery_ok)
                        vehicle_state <= BRAKE;
                    else
                        vehicle_state <= SYNC_MERGE;
                end
                SYNC_MERGE: begin
                    if (traffic_command == STOP || ped || occ || !friction_ok || !battery_ok)
                        vehicle_state <= BRAKE;
                    else
                        vehicle_state <= CRUISE;
                end
                CRUISE: begin
                    if (traffic_command == STOP || ped || occ || !friction_ok || !battery_ok)
                        vehicle_state <= BRAKE;
                    else if (traffic_command == SLOW)
                        vehicle_state <= CRAWL;
                    else
                        vehicle_state <= CRUISE;
                end
                CRAWL: begin
                    if (traffic_command == STOP || ped || occ || !friction_ok || !battery_ok)
                        vehicle_state <= HOLD;
                    else if (traffic_command == PROCEED && !ped && !occ && friction_ok && battery_ok)
                        vehicle_state <= ACCEL;
                end
                HOLD: begin
                    if (traffic_command == STOP || ped || occ || !friction_ok || !battery_ok)
                        vehicle_state <= HOLD;
                    else if (traffic_command == PROCEED && !ped && !occ && friction_ok && battery_ok)
                        vehicle_state <= ALIGN;
                end
                BRAKE: begin
                    if (traffic_command == STOP || ped || occ || !friction_ok || !battery_ok)
                        vehicle_state <= BRAKE;
                    else
                        vehicle_state <= HOLD;
                end
                default: vehicle_state <= IDLE;
            endcase
        end
    end
endmodule

module TrafficController (
    input clk,
    input rst,
    input [7:0] traffic_need,
    output reg [7:0] traffic_command
);

    parameter PROCEED = 8'b00000000,
              SLOW    = 8'b00000001,
              STOP    = 8'b00000010;

    always @(posedge clk or posedge rst) begin
        if (rst)
            traffic_command <= STOP;
        else begin
            case (traffic_need)
                8'b00000000: traffic_command <= STOP;
                8'b00000001: traffic_command <= SLOW;
                8'b00000010: traffic_command <= PROCEED;
                default: traffic_command <= STOP;
            endcase
        end
    end
endmodule


