use <../../util.scad>

PCB_RESET_BUTTON_SIZE = 5.1;
module pcb_reset_button() {
  base_thickness = 0.6;
  module base() {
    color("silver") cube([
        PCB_RESET_BUTTON_SIZE,
        PCB_RESET_BUTTON_SIZE,
        base_thickness
    ]);
  }
  module button() {
    diameter = 2;
    thickness = 0.4;
    color("gold") cylinder(d=diameter, h=thickness);
  }
  base();
  translate([
      PCB_RESET_BUTTON_SIZE / 2, 
      PCB_RESET_BUTTON_SIZE / 2, 
      base_thickness
  ]) button();
}

module push_switch_12x12(include_button=true, button_color="red") {
  hover = 1.2;
  length = 12;
  overlap = 0.01;
  base_height = 3.2;
  interface_length = 3.8;
  metal_top_fillet = 1;
  metal_top_height = 0.1;
  metal_top_length = 10;
  pin_height = 4.1;
  pin_width = 1;
  pin_thickness = 0.6;
  pin_x_spacing = 2.54 * 2;
  pin_y_spacing = 2.54 * 5;
  pin_top_offset = 2.6;
  switch_base_height = 1.2;

  module metal_top(height) {
    module metal_top_corner() {
      translate([
          metal_top_fillet + (length - metal_top_length) / 2,
          metal_top_fillet + (length - metal_top_length) / 2,
          base_height - metal_top_height
      ]) cylinder(r=metal_top_fillet, h=height);
    }

    hull() {
      metal_top_corner();
      tx(metal_top_length - metal_top_fillet * 2) metal_top_corner();
      txy(
        metal_top_length - metal_top_fillet * 2,
        metal_top_length - metal_top_fillet * 2) metal_top_corner();
      ty(metal_top_length - metal_top_fillet * 2) metal_top_corner();
    }
  }

  module base() {
    fillet = 1;
    metal_top_peg_pad = 1.6;
    metal_top_peg_diameter = 1.7;

    module corner() {
      cylinder(r=fillet, h=base_height);
    }

    module peg() {
      txy(0.75, length / 2) cylinder(d=1.5, h=hover + overlap);
    }

    module metal_top_peg() {
      extrude = 0.2;
      tz(hover + base_height - metal_top_height - overlap) cylinder(
          d=metal_top_peg_diameter, h=metal_top_height + overlap + extrude);
    }

    color("#222") union() {
      tz(hover) difference() {
        hull() {
          txy(fillet, fillet) corner();
          txy(length - fillet, fillet) corner();
          txy(fillet, length - fillet) corner();
          txy(length - fillet, length - fillet) corner();
        }
        metal_top(metal_top_height + overlap);
      }
      peg();
      tx(length - 1.5) peg();
      txy(
         metal_top_peg_diameter / 2 + metal_top_peg_pad,
         metal_top_peg_diameter / 2 + metal_top_peg_pad
      ) metal_top_peg();
      txy(
         length - (metal_top_peg_diameter / 2 + metal_top_peg_pad),
         metal_top_peg_diameter / 2 + metal_top_peg_pad
      ) metal_top_peg();
      txy(
         length - (metal_top_peg_diameter / 2 + metal_top_peg_pad),
         length - (metal_top_peg_diameter / 2 + metal_top_peg_pad)
      ) metal_top_peg();
      txy(
         metal_top_peg_diameter / 2 + metal_top_peg_pad,
         length - (metal_top_peg_diameter / 2 + metal_top_peg_pad)
      ) metal_top_peg();
    }
  }

  module pin() {
    translate([
        length / 2 - pin_width / 2,
        -0.65,
        hover + base_height - pin_height - pin_top_offset])
      color("#bbb") cube([pin_width, pin_thickness, pin_height]);
  }

  module pin_side() {
    tx(-pin_x_spacing / 2) pin();
    tx(pin_x_spacing / 2) pin();
  }

  module switch() {
    module base() {
      diameter = 6.9;
      cylinder(d=diameter, h=switch_base_height);
    }

    module interface() {
      height = 3;
      translate([
          -interface_length / 2,
          -interface_length / 2,
          switch_base_height - overlap
      ]) cube([interface_length, interface_length, height + overlap]);
    }

    color("yellow") translate([
        length / 2,
        length / 2,
        hover + base_height - metal_top_height]) {
      union() {
        base();
        interface();
      }
    }
  }

  module button() {
    lower_diameter = 12.9;
    lower_height = 1.4;
    upper_diameter = 11.4;
    height = 5.7;
    interface_width = 5.5;
    interface_height = 2.8;

    translate([
        length / 2,
        length / 2,
        hover + base_height + switch_base_height
    ]) color(button_color) difference() {
      union() {
        txy(-interface_width / 2, -interface_width / 2)
          cube([interface_width, interface_width, interface_height + overlap]);
        tz(interface_height) cylinder(d=lower_diameter, h=lower_height);
        tz(interface_height + lower_height - overlap) cylinder(d=upper_diameter, h=height - lower_height + overlap);
      }
      translate([
          -interface_length / 2,
          -interface_length / 2,
          -overlap
      ])cube([interface_length, interface_length, height + interface_height - 1]);
    }
  }

  txy(-((length - pin_x_spacing) / 2), 0.35) {
    base();
    color("#bbb") tz(hover) metal_top(metal_top_height);
    pin_side();
    ty(pin_y_spacing) pin_side();
    switch();
    if (include_button) {
      button();
    } 
  }
}

/*
$fa=2;
$fs=0.5;
push_switch_12x12();
//pcb_reset_button();
*/
