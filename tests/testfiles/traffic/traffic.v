`timescale 1ns/1ps
module ComposedSystem (
    input clk,
    input rst,
    input ped,               
    input occ,             
    input friction_ok,     
    input battery_ok,        
    input [7:0] traffic_need,
    output [7:0] vehicle_state,
    output [7:0] traffic_command
);

    AutonomousVehicle av (
        .clk(clk),
        .rst(rst),
        .ped(ped),
        .occ(occ),
        .friction_ok(friction_ok),
        .battery_ok(battery_ok),
        .traffic_command(traffic_command),
        .vehicle_state(vehicle_state)
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
