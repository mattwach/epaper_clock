use <../../util.scad>
use <../../shapes.scad>
include <../../../NopSCADlib/utils/core/core.scad>
include <../../../NopSCADlib/vitamins/pin_headers.scad>

overlap = 0.01;

module adafruit_ms8607(include_header) {
  pcb_xsize = 25.4;
  pcb_ysize = 17.8;
  pcb_zsize = 1.7;
  pin_hole_y_offset = 2.54;

  module pcb() {
    module board() {
      color("navy") rounded_cube([pcb_xsize, pcb_ysize, pcb_zsize], fillet=3);
    }

    module mounting_holes() {
      mounting_hole_x_spacing = 20.3;
      mounting_hole_y_spacing = 12.7;
      module mounting_hole() {
        mounting_hole_diameter = 2.5;
        translate([
            (pcb_xsize - mounting_hole_x_spacing) / 2,
            (pcb_ysize - mounting_hole_y_spacing) / 2,
            -overlap])
          cylinder(d=mounting_hole_diameter, h=pcb_zsize + overlap * 2);
      }

      mounting_hole();
      tx(mounting_hole_x_spacing) mounting_hole();
      txy(mounting_hole_x_spacing, mounting_hole_y_spacing) mounting_hole();
      ty(mounting_hole_y_spacing) mounting_hole();
    }

    module pin_holes() {
      pin_hole_spacing = 2.54;
      pin_hole_count = 5;
      pin_hole_x_spacing = pin_hole_spacing * (pin_hole_count - 1);
      module pin_hole() {
        pin_hole_diameter = 0.7;
        translate([
            (pcb_xsize - pin_hole_x_spacing) / 2,
            pin_hole_y_offset,
            -overlap])
          cylinder(d=pin_hole_diameter, h=pcb_zsize + overlap * 2);
      }

      for (i=[0:4]) {
        tx(i * pin_hole_spacing) pin_hole();
      }
    }

    module chip_slots() {
      slot_thickness = 1;
      slot_xsize = 6.1;
      slot_ysize = 6.7;
      slot_top_y_offset = 4.9;

      slot_x_offset = (pcb_xsize - slot_xsize) / 2;

      translate([
          slot_x_offset,
          pcb_ysize - slot_top_y_offset - slot_thickness,
          -overlap]) cube([slot_xsize, slot_thickness, pcb_zsize + overlap * 2]);

      translate([
          slot_x_offset,
          pcb_ysize - slot_top_y_offset - slot_ysize,
          -overlap]) cube([slot_thickness, slot_ysize, pcb_zsize + overlap * 2]);

      translate([
          slot_x_offset + slot_xsize - slot_thickness,
          pcb_ysize - slot_top_y_offset - slot_ysize,
          -overlap]) cube([slot_thickness, slot_ysize, pcb_zsize + overlap * 2]);

    }

    difference() {
      board();
      mounting_holes();
      pin_holes();
      chip_slots();
    }
  }

  module chip() {
    chip_xsize = 2.5;
    chip_ysize = 5;
    chip_zsize = 1.4;

    color("silver") translate([
      (pcb_xsize - chip_xsize) / 2,
      (pcb_ysize - chip_ysize) / 2,
      pcb_zsize
    ]) cube([
      chip_xsize,
      chip_ysize,
      chip_zsize]);
  }

  module stemma_connectors() {
    stemma_connector_xsize = 4.3;
    stemma_connector_ysize = 6;
    stemma_connector_zsize = 2.9;

    module stemma_connector() {
      color("#222") translate([
          0,
          (pcb_ysize - stemma_connector_ysize) / 2,
          pcb_zsize]) cube([
            stemma_connector_xsize,
            stemma_connector_ysize,
            stemma_connector_zsize]);
    }

    stemma_connector();
    tx(pcb_xsize - stemma_connector_xsize) stemma_connector();
  }

  module pin_headers() {
    translate([
        pcb_xsize / 2,
        pin_hole_y_offset,
        0
    ]) ry(180) pin_header(2p54header, 5, 1);
  }

  pcb();
  chip();
  stemma_connectors();

  if (include_header) {
    pin_headers();
  }
}

/*
$fa=2;
$fs=0.5;
adafruit_ms8607(true);
*/

