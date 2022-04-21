use <../../util.scad>
use <../../shapes.scad>
include <../../../NopSCADlib/utils/core/core.scad>
include <../../../NopSCADlib/vitamins/pin_headers.scad>

overlap = 0.01;

module adafruit_gps(include_header) {
  pcb_xsize = 25.4;
  pcb_ysize = 34;
  pcb_zsize = 1.7;
  pin_hole_y_offset = 2;

  module pcb() {
    module board() {
      color("navy") rounded_cube([pcb_xsize, pcb_ysize, pcb_zsize], fillet=3);
    }

    module mounting_holes() {
      mounting_hole_x_spacing = 20.3;
      mounting_hole_y_offset = 31.5;
      module mounting_hole() {
        mounting_hole_diameter = 2.3;
        translate([
            (pcb_xsize - mounting_hole_x_spacing) / 2,
            mounting_hole_y_offset,
            -overlap])
          cylinder(d=mounting_hole_diameter, h=pcb_zsize + overlap * 2);
      }

      mounting_hole();
      tx(mounting_hole_x_spacing) mounting_hole();
    }

    module pin_holes() {
      pin_hole_spacing = 2.54;
      pin_hole_count = 9;
      pin_hole_x_spacing = pin_hole_spacing * (pin_hole_count - 1);
      module pin_hole() {
        pin_hole_diameter = 0.7;
        translate([
            (pcb_xsize - pin_hole_x_spacing) / 2,
            pin_hole_y_offset,
            -overlap])
          cylinder(d=pin_hole_diameter, h=pcb_zsize + overlap * 2);
      }

      for (i=[0:8]) {
        tx(i * pin_hole_spacing) pin_hole();
      }
    }

    difference() {
      board();
      mounting_holes();
      pin_holes();
    }
  }

  module gps_module() {
    gps_module_xsize = 14.9;
    gps_module_ysize = 15;
    gps_module_zsize = 6.1;
    gps_module_y_offset = 10.4;

    color("#640") translate([
        (pcb_xsize - gps_module_xsize) / 2,
        gps_module_y_offset,
        pcb_zsize]) cube([
          gps_module_xsize,
          gps_module_ysize,
          gps_module_zsize]);
  }

  module led() {
    led_xsize = 1.4;
    led_ysize = 2.8;
    led_zsize = 1;
    led_x_offset = 1.1;
    led_y_offset = 20.6;

    color("#faa", 0.2) translate([
        led_x_offset,
        led_y_offset,
        pcb_zsize]) cube([
          led_xsize,
          led_ysize,
          led_zsize]);
  }

  module sma_connector() {
    sma_connector_diameter = 2;
    sma_connector_inner_diameter = 1.7;
    sma_connector_height = 1;
    sma_connector_x_offset = 19.2;
    sma_connector_y_offset = 29;

    module connector() {
      color("gold") union() {
        difference() {
          cylinder(d=sma_connector_diameter, h=sma_connector_height);
          tz(-overlap) cylinder(
              d=sma_connector_inner_diameter, h=sma_connector_height+overlap*2);
        }
        cylinder(d=0.5, h=0.75);
      }
    }

    translate([
      sma_connector_x_offset,
      sma_connector_y_offset,
      pcb_zsize
    ]) connector();

  }

  module pin_headers() {
    translate([
        pcb_xsize / 2,
        pin_hole_y_offset,
        0
    ]) ry(180) pin_header(2p54header, 9, 1);
  }

  pcb();
  gps_module();
  led();
  sma_connector();

  if (include_header) {
    pin_headers();
  }
}

/*
$fa=2;
$fs=0.5;
adafruit_gps(true);
*/

