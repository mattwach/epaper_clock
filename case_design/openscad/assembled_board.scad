use <../lib/mattwach/util.scad>
use <../lib/mattwach/shapes.scad>
include <../lib/mattwach/vitamins/electronics/arduino.scad>
include <../lib/mattwach/vitamins/electronics/capacitor.scad>
include <../lib/mattwach/vitamins/electronics/adafruit_gps.scad>
include <../lib/mattwach/vitamins/electronics/adafruit_ms8607.scad>
include <../lib/NopSCADlib/core.scad>
include <../lib/NopSCADlib/vitamins/axials.scad>
include <../lib/NopSCADlib/vitamins/buttons.scad>
include <../lib/NopSCADlib/vitamins/dip.scad>
include <../lib/NopSCADlib/vitamins/leds.scad>
include <../lib/NopSCADlib/vitamins/pin_headers.scad>

assembled_board_length = 76.2;
assembled_board_width = 48.9;

module assembled_board() {
  board_x_offset = 8.26;
  board_y_offset = 7.63;
  board_thickness = 1.6;
  pin_spacing = 2.54;

  function cx(x) = board_x_offset + x * pin_spacing;
  function cy(y) = board_y_offset + y * pin_spacing;

  module base_pcb() {
    color("#bbb") ty(assembled_board_length) rz(90) ry(180) import("freecad_models/main_pcb.stl");
  }

  module atmega328p_socket() {
    translate([cx(8), cy(14.5), board_thickness]) rz(-90) dil_socket(14, 7.62);
  }

  module atmega328() {
    translate([cx(8), cy(14.5), 4.7]) rz(-90) pdip(28, "ATMega328P", false);
  }

  // This is for height checking the alt circuit design.  It's not intended
  // to represent the V2 board.
  module pro_mini() {
    translate([cx(4), cy(14.5), 12.6]) rz(180) arduino_pro_mini(straight_interface_pins=true);
  }

  module cpu_capacitor() {
     translate([cx(8), cy(17.7), board_thickness]) rz(90) ceramic_capacitor("104");
  }

  module isp_header() {
    translate([cx(-0.5), cy(-1), board_thickness]) pin_header(2p54header, 3, 2);
  }

  module debug_header() {
    translate([cx(12), cy(11.5), board_thickness]) pin_header(2p54header, 2, 1);
  }

  module heartbeat_led() {
    translate([cx(-0.5), cy(16.5), board_thickness]) rz(90) led(LED3mm, "green", 2);
  }

  module led_resistor() {
    translate([cx(-0.5), cy(12.5), board_thickness]) rz(90) ax_res(res1_4, 10000, pitch=8);
  }

  module ms8607_interface() {
    translate([cx(10.5), cy(4), board_thickness]) pin_socket(2p54header, 5, 1);
  }

  module gps_interface() {
    translate([cx(1.5), cy(22), board_thickness]) rz(90) pin_socket(2p54header, 9, 1);
  }

  module epaper_interface() {
    translate([cx(10), cy(0), board_thickness]) jst_xh_header(jst_xh_header, 8);
  }

  module power_connector() {
    translate([cx(4), cy(0), board_thickness]) rz(180) jst_xh_header(jst_xh_header, 2);
  }

  module power_capacitors() {
    module cap() {
      tz(4) ry(90) rx(-90) electrolytic_capacitor(
        diameter=5.2,
        height=11.4,
        pin_spacing=2.54,
        label_text="1u");
    }

    translate([cx(12), cy(5.5), board_thickness]) cap();
    translate([cx(8.5), cy(10.5), board_thickness]) rz(90) cap();
  }

  module buttons() {
    translate([cx(-0.4), cy(7.75), board_thickness]) square_button(button_6mm);
    translate([cx(-0.5), cy(3), board_thickness]) square_button(button_6mm);
  }

  module ms8607() {
    translate([cx(5.5), cy(3), 12.6]) adafruit_ms8607(true);
  }

  module gps() {
    translate([cx(0.7), cy(27), 12.6]) rz(-90) adafruit_gps(true);
  }

  base_pcb();
  atmega328p_socket();
  atmega328();
  *pro_mini();
  cpu_capacitor();
  isp_header();
  debug_header();
  heartbeat_led();
  led_resistor();
  ms8607_interface();
  gps_interface();
  epaper_interface();
  power_connector();
  power_capacitors();
  buttons();
  ms8607();
  gps();
}

/*
$fa=2.0;
$fs=0.5;
assembled_board();
*/
