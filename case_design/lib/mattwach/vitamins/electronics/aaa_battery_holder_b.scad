// A different brand I bought on amazon that ias a bit bigger

use <../../util.scad>

aaa_holder_height = 15.9;
aaa_holder_width = 48.8;  // a bit wider
aaa_holder_length = 62.1;  // A bit longer too
aaa_holder_fillet = 3;
overlap = 0.01;

module aaa_box_simple(length=aaa_holder_length) {
  module edge() {
    rx(-90) cylinder(r=aaa_holder_fillet, h=length);
  }

  hull() {
    translate([aaa_holder_fillet, 0, aaa_holder_fillet]) edge();
    translate([aaa_holder_width - aaa_holder_fillet, 0, aaa_holder_fillet]) edge();
    translate([aaa_holder_fillet, 0, aaa_holder_height - aaa_holder_fillet]) edge();
    translate([aaa_holder_width - aaa_holder_fillet, 0, aaa_holder_height - aaa_holder_fillet]) edge();
  }
}

module aaa_box_opening(length=aaa_holder_length, gap=0.1) {
  fillet = aaa_holder_fillet + gap;
  module edge() {
    rx(-90) cylinder(r=fillet, h=length);
  }

  hull() {
    translate([aaa_holder_fillet, 0, aaa_holder_fillet]) edge();
    translate([aaa_holder_width - aaa_holder_fillet, 0, aaa_holder_fillet]) edge();
    translate([aaa_holder_fillet, 0, aaa_holder_height - aaa_holder_fillet]) edge();
    translate([aaa_holder_width - aaa_holder_fillet, 0, aaa_holder_height - aaa_holder_fillet]) edge();
  }
}


module aaa_battery_holder() {
  shell_thickness = 1.25;
  slit_width = 0.4;
  cover_depth = 5.4;
  wire_port_side_offset = 16.3;
  wire_port_width = 3.9;
  wire_port_height = 5.2;
  wire_port_depth = 10;
  wire_port_top_offset = 6.5;
  battery_port_length = 8;
  battery_port_width = 4.5;
  battery_port_top_margin = 3.3;
  battery_port_side_margin = 8.5;

  module inside_box() {
    inside_fillet = aaa_holder_fillet - shell_thickness;
    module edge() {
      rx(-90) cylinder(
          r=inside_fillet,
          h=aaa_holder_length - shell_thickness * 2);
    }

      translate([
        shell_thickness,
        shell_thickness,
        shell_thickness]) hull() {
      translate([inside_fillet, 0, inside_fillet]) edge();
      translate([
          aaa_holder_width - inside_fillet - shell_thickness * 2,
          0,
          inside_fillet
      ]) edge();
      translate([
          inside_fillet,
          0,
          aaa_holder_height - inside_fillet - shell_thickness * 2
      ]) edge();
      translate([
          aaa_holder_width - inside_fillet - shell_thickness * 2,
          0,
          aaa_holder_height - inside_fillet - shell_thickness * 2
      ]) edge();
    }
  }
  
  module top_slit() {
    width = 0.5;
    translate([
        0,
        aaa_holder_length - shell_thickness - width,
        aaa_holder_height - cover_depth - overlap,
    ]) cube([aaa_holder_width, width, cover_depth + overlap * 2]);
  }

  module middle_slit() {
    translate([
      0,
      shell_thickness,
      aaa_holder_height - cover_depth,
    ]) cube([aaa_holder_width, aaa_holder_length - shell_thickness * 2, slit_width]);
  }

  module end_slit() {
    edge_border = 4.75;
    tab_width = 9.6;
    tab_depth = 4.2;

    module tab() {
      hole_width = 6;
      hole_height = 1.6;
      module vertical_tab_slit() {
        tz(aaa_holder_height - cover_depth - tab_depth)
          cube([slit_width, shell_thickness + overlap, tab_depth + slit_width]);
      }
      vertical_tab_slit();
      tz(aaa_holder_height - cover_depth - tab_depth)
        cube([tab_width, shell_thickness + overlap, slit_width]);
      tx(tab_width - slit_width) vertical_tab_slit();
      translate(
          [(tab_width - hole_width) / 2,
          0,
          aaa_holder_height - cover_depth - hole_height])
        cube([hole_width, shell_thickness + overlap, hole_height]);
    }

    ty(-overlap) union() {
      tz(aaa_holder_height - cover_depth)
        cube([edge_border, shell_thickness + overlap, slit_width]);
      tx(edge_border - slit_width) tab();
      translate([
          edge_border + tab_width - slit_width * 2,
          0,
          aaa_holder_height - cover_depth]) cube([
            aaa_holder_width - edge_border * 2 - tab_width * 2 + slit_width * 4,
            shell_thickness + overlap,
            slit_width]);
      tx(aaa_holder_width - edge_border - tab_width + slit_width) tab();
      translate([aaa_holder_width - edge_border, 0, aaa_holder_height - cover_depth])
        cube([edge_border, shell_thickness + overlap, slit_width]);
    }
  }

  module screw_hole() {
    outer_diameter = 3.8;
    inner_diameter = 1.9;
    outer_depth = 2;
    inner_depth = 2;
    edge_offset = 5.1;
    side_offset = 21.8;
    translate([side_offset, edge_offset, aaa_holder_height - outer_depth]) union() {
      cylinder(d=outer_diameter, h=outer_depth + overlap);
      tz(-outer_depth) cylinder(d=inner_diameter, h=inner_depth + overlap); 
    }
  }

  module wire_port() {
    translate([
        wire_port_side_offset,
        -overlap,
        aaa_holder_height - wire_port_height - wire_port_top_offset]) union() {
        tz(wire_port_width / 2) cube([
            wire_port_width,
            wire_port_depth,
            wire_port_height - wire_port_width / 2]);
        translate([wire_port_width / 2, 0, wire_port_width / 2]) rx(-90)
          cylinder(d=wire_port_width, h=wire_port_depth);
    }
  }

  module wire() {
    diameter = 1.3;
    length = 20;
    translate([
        wire_port_side_offset + wire_port_width / 2,
        wire_port_depth - length,
        aaa_holder_height - wire_port_top_offset - wire_port_height + wire_port_width / 2])
      rx(-90) cylinder(d=diameter, h=length);
  }

  module battery_switch_port() {
    depth = 10;
    translate([
        aaa_holder_width - battery_port_length - battery_port_side_margin,
        battery_port_top_margin,
        -overlap,
    ]) cube([battery_port_length, battery_port_width, depth]);
  }

  module battery_switch() {
    tab_size = 3.4;
    tab_depth = 2;
    bottom_plate_size = tab_size + 4;
    bottom_plate_depth = 2;
    notch_size = 0.5;
    module notch() {
      translate([
          (bottom_plate_size - tab_size) / 2 + 0.5,
          -overlap, bottom_plate_depth + tab_depth]) rx(-90) 
        cylinder(d=notch_size, h=tab_size + overlap * 2);
    }

    translate([
        aaa_holder_width - battery_port_side_margin + (bottom_plate_size - tab_size) / 2 - 1,
        battery_port_top_margin + 0.5,
        tab_depth + bottom_plate_depth]) ry(180) union() {
      cube([bottom_plate_size, tab_size, bottom_plate_depth]);
      translate([(bottom_plate_size - tab_size) / 2, 0, bottom_plate_depth]) cube([tab_size, tab_size, tab_depth]);
      for (i = [0:3]) {
        tx(i * notch_size * 1.55) notch();
      }
    }
  }

  module finger_tabs() {
    tab_max = 9;
    tab_min = 6;
    tab_width = 0.7;
    tab_height = 0.5;
    tab_count = 5;
    span = 7.05;
    top_offset = 9.05;
    step = (span + tab_width) / tab_count;

    module tab(width, i) {
      translate([
          (aaa_holder_width - width) / 2,
          aaa_holder_length - top_offset - tab_width - i * step,
          aaa_holder_height]) cube([width, tab_width, tab_height]);
    }

    for (i = [0:tab_count-1]) {
      tab(tab_max - ((tab_max - tab_min) * i / (tab_count - 1)), i);
    }
  }

  color("#222") union() {
    difference() {
      aaa_box_simple();
      inside_box();
      top_slit();
      middle_slit();
      end_slit();
      screw_hole();
      wire_port();
      battery_switch_port();
    }
    finger_tabs();
  }
  color("#222") battery_switch();
  color("red") wire();
  tz(2) color("#333") wire();
}

/*
$fa=2;
$fs=0.5;
aaa_battery_holder();
*/


