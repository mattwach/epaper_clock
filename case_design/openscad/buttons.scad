use <../lib/mattwach/util.scad>

module buttons(
    hole_diameter,
    hole_thickness,
    hole_span,
    button_size,
    button_depth, // cepth starting at bottom of top plate
    button_protrusion,
    use_hull,
    show_carriage,
    show_button) {

  overlap = 0.01;
  interface_thickness = 1.5;
  button_pad = 0.2;
  base_pad = 6;
  base_diameter = button_size * 2 + 0.5;

  module base() {
    module leg() {
      tz(base_pad) cylinder(
          d=base_diameter,
          h=button_depth - base_pad - interface_thickness);
    }
    hull() {
      leg();
      ty(hole_span) leg();
    }
  }

  module interface() {
    interface_pad = 4;
    module interface_leg() {
      tz(button_depth - interface_thickness - overlap) cylinder(
          d=base_diameter + interface_pad * 2, h=interface_thickness + overlap);
    }
    hull() {
      interface_leg();
      ty(hole_span) interface_leg();
    }
  }

  module hole_plugs() {
    module hole_plug() {
      tz(button_depth - overlap) cylinder(
          d=hole_diameter,
          h=hole_thickness + overlap);
    }

    if (use_hull) {
      hull() {
        hole_plug();
        ty(hole_span) hole_plug();
      }
    } else {
      hole_plug();
      ty(hole_span) hole_plug();
    }
  }

  module button_slots() {
    slot_width = button_size + button_pad * 2;
    module button_slot() {
      translate([
          -slot_width / 2,
          -slot_width / 2,
          base_pad - overlap]) cube([
            slot_width,
            slot_width,
            button_depth - base_pad + hole_thickness + overlap * 2]);
    }

    button_slot();
    ty(hole_span) button_slot();
  }

  module carriage() {
    difference() {
      union() {
        base();
        interface();
        hole_plugs();
      }
      button_slots();
    }
  }

  module button() {
    button_base_height = 3;
    module button_base() {
      button_base_width = 6;
      translate([
          -button_size / 2,
          -button_base_width / 2,
          0]) cube([
            button_size,
            button_base_width,
            button_base_height]);
    }

    module button_rod() {
      translate([
          -button_size / 2,
          -button_size / 2,
          button_base_height - overlap]) cube([
            button_size,
            button_size,
            button_depth - button_base_height + hole_thickness + button_protrusion + overlap]);
    }


    color("red") union() {
      button_base();
      button_rod();
    }
  }

  if (show_carriage) {
    carriage();
  }

  if (show_button) {
    button();
  }
}

/*
$fa=2.0;
$fs=0.5;
buttons(
    hole_diameter=7,
    hole_thickness=3.175,
    hole_span=10,
    button_size=3,
    button_depth=20,
    button_protrusion=1.5,
    show_carriage=true,
    show_button=true);
*/
