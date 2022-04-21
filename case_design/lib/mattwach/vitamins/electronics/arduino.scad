use <../../util.scad>
use <smd_chip.scad>
include <buttons.scad>
include <../../../NopSCADlib/utils/core/core.scad>
include <../../../NopSCADlib/vitamins/pin_headers.scad>

module arduino_pro_mini(
    side_pins=true,
    angled_interface_pins=true,
    sda_scl_pins=false,
    rotated_sda_scl_pins=false,
    a6_a7_pins=false,
    straight_interface_pins=false) {
  pcb_length = 33.6;
  pcb_width = 18.5;
  pcb_thickness = 0.95;
  hole_pitch = 2.54;
  board_offset = 1.3;
  side_hole_y_offset = 4;
  inner_hole_offset = 4.3;
  sda_scl_y_offset = 12.9;
  a6_a7_y_offset = 23.6;
  overlap = 0.01;

  module pcb() {
    color("#008") cube([pcb_width, pcb_length, pcb_thickness]);
  }

  module hole(plating) {
    hole_diameter = 1;
    thickness = 0.03;
    module ring() {
      diameter = 1.8;
      color("silver") difference() {
        cylinder(d=diameter, h=thickness);
        tz(-overlap) cylinder(d=hole_diameter, h=thickness + overlap * 2);
      }
    }

    if (plating) {
      tz(pcb_thickness) ring();
      tz(-thickness) ring();
    } else {
      tz(-overlap) cylinder(d = hole_diameter, h = pcb_thickness + overlap * 2);
    }
  }


  module header_holes(plating) {
    n = 6;
    spacing = (n - 1) * hole_pitch;
    x0 = (pcb_width - spacing) / 2;
    for (i = [0:n-1]) {
      txy(x0 + i * hole_pitch, board_offset) hole(plating);
    }
  }

  module side_holes(plating) {
    module side() {
      n = 12;
      for (i = [0:n-1]) {
        ty(side_hole_y_offset + i * hole_pitch) hole(plating);
      }
    }

    tx(board_offset) side();
    tx(pcb_width - board_offset) side();
  }

  module inner_holes(plating) {
    tx(inner_hole_offset) {
      hole(plating);
      ty(hole_pitch) hole(plating);
    }
  }

  module all_holes(plating) {
    header_holes(plating);
    side_holes(plating);
    ty(sda_scl_y_offset) inner_holes(plating);
    ty(a6_a7_y_offset) inner_holes(plating);
  }

  module atmega_328p() {
    txy(-3.5, -3.5) smd_chip(
         length=7,
         width=7,
         thickness=0.8,
         pin_spacing=0.78,
         pin_length=1.6,
         pin_count_length=8,
         pin_count_width=8);
  }

  module voltage_regulator() {
    tx(-1.55 / 2) smd_chip_detailed(
        length=1.55,
        width=2.9,
        thickness=0.8,
        pin_length=1,
        pins_e=[2, 1.5],
        pins_w=[3, 0.8]);
  }

  module power_led() {
    length = 2;
    tx(-length / 2) color("red", 0.3) cube([length, 0.8, 0.5]);
  }

  module crystal_oscillator() {
    color("silver") cube([1.5, 3.5, 0.6]);
  }

  module capacitor() {
    color("orange") cube([2, 3.3, 1]);
  }

  module side_pin_header() {
    ty(2.54 * 5.5 + side_hole_y_offset) ry(180) rz(90)
      pin_header(2p54header, 12, 1);
  }

  module angled_interface_header(right_angle) {
      translate([
          pcb_width / 2,
          board_offset,
          pcb_thickness]) pin_header(2p54header, 6, 1, right_angle=right_angle);
  }

  module inner_pins() {
    txy(inner_hole_offset, 2.54 / 2) rx(180) rz(90) pin_header(2p54header, 2, 1);
  }

  difference() {
    pcb();
    all_holes(plating=false);
  }
  all_holes(plating=true);
  translate([9.6, 19.1, pcb_thickness]) rz(45) atmega_328p();
  translate([pcb_width / 2, 7.2, pcb_thickness]) voltage_regulator();
  translate([pcb_width / 2, 10.4, pcb_thickness]) power_led();
  translate([13.5, 11.7, pcb_thickness]) crystal_oscillator();
  translate([3.4, 7.8, pcb_thickness]) capacitor();
  translate([12.6, 5.2, pcb_thickness]) capacitor();
  translate([
      (pcb_width - PCB_RESET_BUTTON_SIZE) / 2,
      pcb_length - PCB_RESET_BUTTON_SIZE,
      pcb_thickness
  ]) pcb_reset_button();
  if (side_pins) {
    tx(board_offset) side_pin_header();
    tx(pcb_width - board_offset) side_pin_header();
  }
  if (straight_interface_pins) {
    angled_interface_header(false);
  } else if (angled_interface_pins) {
    angled_interface_header(true);
  }
  if (sda_scl_pins) {
    ty(sda_scl_y_offset) inner_pins();
  }
  if (rotated_sda_scl_pins) {
    tx(8.6) ty(sda_scl_y_offset) tz(pcb_thickness) ry(180) inner_pins();
  }
  if (a6_a7_pins) {
    ty(a6_a7_y_offset) inner_pins();
  }
}

/*
$fa=2;
$fs=0.5;
arduino_pro_mini();
*/

