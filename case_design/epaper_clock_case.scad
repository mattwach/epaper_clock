use <lib/mattwach/util.scad>
use <lib/mattwach/vitamins/bolts.scad>
use <lib/mattwach/shapes.scad>
include <lib/mattwach/vitamins/electronics/aaa_battery_holder_b.scad>
include <lib/mattwach/vitamins/electronics/waveshare_epaper_2.9in.scad>
include <openscad/assembled_board.scad>
include <openscad/buttons.scad>

overlap = 0.01;

epaper_x_offset = 0;
epaper_y_offset = 8;
epaper_z_offset = 16.2;
epaper_display_angle = 75;
face_plate_zoffset = waveshare_epaper_pixel_area_zoffset + 0.3;
pcb_xpadding = 5;
pcb_ypadding = 3;
base_xsize = assembled_board_length + pcb_xpadding * 2;
base_ysize = assembled_board_width + pcb_ypadding * 2;
base_thickness = 18;
top_cover_inset = 1;
top_cover_thickness = 3.175;
top_shell_zsize = 23;
pcb_zpad = 3;
pcb_thickness = 1.6;

button_a_x = 20.5;
button_b_x = 32.5;
button_y = 45;
button_access_hole_diameter = 8;

module base_template(height, inset=0) {
  base_fillet = 4;
  txy(inset, inset) rounded_cube(
      [base_xsize - inset * 2, base_ysize - inset * 2, height], fillet=base_fillet);
}

module epaper_clock_case(add_parts) {
  pcb_bolt_board_offset = 1.1;
  battery_holder_xpad = 9;
  battery_holder_zpad = 0.5;

  module base() {
    aaa_gap_width = aaa_holder_width - 2;
    module aaa_slot() {
      translate([
          -battery_holder_xpad,
          (base_ysize - aaa_holder_width) / 2,
          aaa_holder_height + battery_holder_zpad]) rz(-90) ry(180) aaa_box_opening(gap=0.2);
      translate([
          -overlap,
          (base_ysize - aaa_gap_width) / 2,
          -overlap]) cube([aaa_holder_length - battery_holder_xpad, aaa_gap_width, 10]);
    }

    module pcb_cutout() {
      pcb_cutout_xsize = assembled_board_length + 0.3;
      pcb_cutout_ysize = assembled_board_width + 0.2;
      translate([
          (base_xsize - pcb_cutout_xsize) / 2,
          (base_ysize - pcb_cutout_ysize) / 2,
          base_thickness]) cube([
          pcb_cutout_xsize,
          pcb_cutout_ysize,
          pcb_zpad + pcb_thickness + overlap]);
    }

    module pcb_support_pegs() {
      pcb_support_peg_diameter = 4;
      module support_peg() {
        translate([
            pcb_xpadding,
            (base_ysize - aaa_holder_width) / 2,
            base_thickness - overlap]) cylinder(
              d=pcb_support_peg_diameter, h=pcb_zpad+overlap);
      }

      support_peg();
      tx(assembled_board_length) support_peg();
      txy(assembled_board_length, assembled_board_width) support_peg();
      ty(assembled_board_width) support_peg();
    }

    module pcb_bolt_holes() {
      pcb_bolt_hole_diameter = 1.85;
      pcb_bolt_hole_depth = 8;
      module pcb_bolt_hole() {
        translate([
            pcb_xpadding,
            base_ysize / 2,
            base_thickness + pcb_zpad + pcb_thickness - pcb_bolt_hole_depth + overlap,
        ]) cylinder(d=pcb_bolt_hole_diameter, h=pcb_bolt_hole_depth+overlap);
      }

      tx(-pcb_bolt_board_offset) pcb_bolt_hole();
      tx(assembled_board_length+pcb_bolt_board_offset) pcb_bolt_hole();
    }

    module epaper_interface() {
      module interface_plate() {
        electronics_cutout_width = 22;
        interface_plate_thickness = 5;
        interface_plate_xpad = 7;
        interface_plate_width = waveshare_epaper_length + interface_plate_xpad * 2;
        interface_plate_height = waveshare_epaper_width;

        module main_plate() {
          cube([
            interface_plate_width,
            interface_plate_height,
            interface_plate_thickness]);
        }

        module electronics_cutout() {
          electronics_cutout_height = 3;
          translate([
              -overlap,
              (interface_plate_height - electronics_cutout_width) / 2,
              interface_plate_thickness - electronics_cutout_height
          ]) cube([
            interface_plate_width + overlap * 2,
            electronics_cutout_width,
            electronics_cutout_height + overlap]);
        }

        module connector_cutout() {
          connector_cutout_width = 15;
          translate([
              -overlap,
              (interface_plate_height - electronics_cutout_width) / 2,
              -overlap,
          ]) cube([
            connector_cutout_width + overlap,
            electronics_cutout_width,
            interface_plate_thickness + overlap * 2]);
        }

        module face_plate_holes() {
          face_plate_hole_diameter = 3.2;
          face_plate_hole_offset = 4;
          face_plate_countersink_diameter = 6;
          face_plate_countersink_depth = interface_plate_thickness - 0.5;
          module face_plate_hole() {
            translate([
                0,
                0,
                -overlap]) union() {
                  cylinder(
                    d=face_plate_countersink_diameter,
                    h=face_plate_countersink_depth);
                  tz(face_plate_countersink_depth - overlap) cylinder(
                    d=face_plate_hole_diameter,
                    h=10);
            }
          }

          txy(face_plate_hole_offset, face_plate_hole_offset) face_plate_hole();
          txy(interface_plate_width - face_plate_hole_offset, face_plate_hole_offset) face_plate_hole();
          txy(face_plate_hole_offset, interface_plate_height - face_plate_hole_offset) face_plate_hole();
          txy(interface_plate_width - face_plate_hole_offset, interface_plate_height - face_plate_hole_offset) face_plate_hole();
        }

        module epaper_mount_holes() {
          epaper_mount_hole_diameter = 1.85;
          module hole() {
            translate([
                interface_plate_xpad,
                0,
                -overlap])  cylinder(d=epaper_mount_hole_diameter, h=interface_plate_thickness + overlap * 2);
          }
          txy(waveshare_epaper_bolt_inset, waveshare_epaper_bolt_inset) hole();
          txy(waveshare_epaper_length - waveshare_epaper_bolt_inset, waveshare_epaper_bolt_inset) hole();
          txy(waveshare_epaper_bolt_inset, waveshare_epaper_width - waveshare_epaper_bolt_inset) hole();
          txy(waveshare_epaper_length - waveshare_epaper_bolt_inset,
              waveshare_epaper_width - waveshare_epaper_bolt_inset) hole();
        }

        module face_plate_interface() {
          face_plate_interface_width = 6.5;
          face_plate_interface_height = (interface_plate_height - electronics_cutout_width) / 2;
          module face_plate_interface_tab() {
            tz(interface_plate_thickness - overlap) cube([
                  face_plate_interface_width,
                  face_plate_interface_height,
                  face_plate_zoffset]);
          }
          face_plate_interface_tab();
          tx(interface_plate_width - face_plate_interface_width) face_plate_interface_tab();
          txy(interface_plate_width - face_plate_interface_width,
              interface_plate_height - face_plate_interface_height) face_plate_interface_tab();
          ty(interface_plate_height - face_plate_interface_height) face_plate_interface_tab();
        }

        translate([
            -epaper_x_offset - waveshare_epaper_bolt_inset - interface_plate_xpad,
            -epaper_y_offset,
            epaper_z_offset - waveshare_epaper_bolt_inset
        ]) rx(epaper_display_angle) tz(-4.3) difference() {
          union() {
            main_plate();
            face_plate_interface();
          }
          electronics_cutout();
          connector_cutout();
          face_plate_holes();
          epaper_mount_holes();
        }
      }

      // attached interface plate to main body
      module interface_glue() {
        interface_glue_xsize = base_xsize - 11;
        interface_glue_ysize = epaper_y_offset;
        interface_glue_zsize = epaper_z_offset + 22;
        interface_glue_fillet = 2;
        module block() {
          translate([
              (base_xsize - interface_glue_xsize) / 2,
              -interface_glue_ysize,
              0
          ]) cube([
            interface_glue_xsize,
            interface_glue_ysize + pcb_ypadding,
            interface_glue_zsize]);
        }

        module cutout() {
          translate([
              (base_xsize - interface_glue_xsize) / 2 - overlap,
              -epaper_y_offset + 2.5,
              epaper_z_offset - waveshare_epaper_bolt_inset]) rx(epaper_display_angle) cube([
                interface_glue_xsize + overlap * 2,
                40,
                10]);
        }

        difference() {
          block();
          cutout();
        }
      }

      interface_plate();
      interface_glue();
    }

    module top_shell() {
      top_shell_thickness = 2;
    
      module cutout() {
        cutout_width = 8;
        translate([
            -overlap,
            -overlap,
            0]) cube([
              pcb_xpadding + 2,
              cutout_width + overlap,
              top_shell_zsize + overlap]);

        // A little messy, trying to get ride of the overlap ridge
        tz(10) rx(epaper_display_angle) cube([base_xsize,10,10]);
      }

      module vents() {
        vent_width = 2;
        vent_pad = 6;

        module vent(height) {
          module peg() {
            tx(-overlap) ry(90) cylinder(d=vent_width, h = height);
          }

          hull() {
            tz(vent_pad) peg();
            tz(top_shell_zsize - vent_pad) peg();
          }
        }

        module side_vents() {
          side_vent_count = 3;
          side_vent_ypad = 15;
          side_vent_span = (base_ysize - side_vent_ypad * 2);
          side_vent_gap = side_vent_span / (side_vent_count - 1);

          for (i=[0:side_vent_count-1]) {
            ty((base_ysize - side_vent_span) / 2 + i * side_vent_gap) vent(base_xsize + overlap * 2);
          }
        }

        module back_vents() {
          module back_vent(height) {
            ty(base_ysize + overlap) rz(-90) vent(height);
          }

          back_vent_count = 5;
          back_vent_xpad = 15;
          back_vent_span = (base_xsize - back_vent_xpad * 2);
          back_vent_gap = back_vent_span / (back_vent_count - 1);

          for (i=[0:back_vent_count-1]) {
            tx((base_xsize - back_vent_span) / 2 + i * back_vent_gap) back_vent(top_shell_thickness + overlap * 4);
          }
        }

        side_vents();
        back_vents();
      }

      module inset() {
        tz(top_shell_zsize - top_cover_thickness) base_template(top_cover_thickness + overlap * 2, top_cover_inset);
      }

      tz(base_thickness + pcb_zpad + pcb_thickness - overlap) difference() {
        base_template(top_shell_zsize);
        tz(-overlap) base_template(top_shell_zsize + overlap * 2, top_shell_thickness);
        cutout();
        inset();
        vents();
      }
    } 

    union() {
      difference() {
        base_template(base_thickness + pcb_zpad + pcb_thickness);
        aaa_slot();
        pcb_cutout();
        pcb_bolt_holes();
      }
      top_shell();
      pcb_support_pegs();
      epaper_interface();
    }
  }

  module battery_holder() {
    translate([
        -battery_holder_xpad,
        (base_ysize - aaa_holder_width) / 2,
        aaa_holder_height + battery_holder_zpad]) rz(-90) ry(180) aaa_battery_holder();
  }

  module pcb_bolts() {
    module pcb_bolt() {
      translate([
         pcb_xpadding,
         base_ysize / 2,
         base_thickness + pcb_zpad + pcb_thickness,
      ]) bolt_M2(6);
    }

    tx(-pcb_bolt_board_offset) pcb_bolt();
    tx(assembled_board_length+pcb_bolt_board_offset) pcb_bolt();
  }

  module board() {
    translate([
        pcb_xpadding,
        pcb_ypadding + assembled_board_width,
        base_thickness + pcb_zpad
    ]) rz(-90) assembled_board();
  }

  base();
  if (add_parts) {
    battery_holder();
    board();
    pcb_bolts();
  }
}

module face_plate() {
  face_plate_thickness = 6.375;
  face_plate_xsize = 110;
  face_plate_ysize = 68;  // golden ratio
  face_plate_fillet = 11;
  viewport_border = 0.7;
  viewport_xoffset = waveshare_epaper_pixel_area_yoffset - waveshare_epaper_bolt_inset - viewport_border;
  viewport_yoffset = waveshare_epaper_pixel_area_xoffset - waveshare_epaper_bolt_inset - viewport_border;
  viewport_width = waveshare_epaper_pixel_area_width + viewport_border * 2;
  viewport_height = waveshare_epaper_pixel_area_height + viewport_border * 2;

  module epaper_viewport() {
    module viewport(pad=0, height=face_plate_thickness + 1) {
      translate([
          viewport_xoffset-pad,
          viewport_yoffset-pad,
          waveshare_epaper_pixel_area_zoffset]) cube([
        viewport_width+pad*2,
        viewport_height+pad*2,
        height
      ]);
    }
    hull() {
      viewport();
      tz(face_plate_thickness + 1) viewport(face_plate_thickness + 1, 0.01);
    }
  }

  module plate() {
    color("#a98") translate([
        viewport_xoffset - (face_plate_xsize - viewport_width) / 2,
        viewport_yoffset - (face_plate_ysize - viewport_height) / 2,
        face_plate_zoffset]) rounded_cube([
          face_plate_xsize, face_plate_ysize, face_plate_thickness], fillet=face_plate_fillet);
  }

  module mounting_holes() {
    // another hack because face_plate_hole is many transformations nester
    mounting_hole_xspan = 95.5;
    mounting_hole_yspan = 30;
    mounting_hole_xoffset = -5.5;
    mounting_hole_yoffset = 1.6;

    module hole() {
      mounting_hole_diameter = 2.85;
      mounting_hole_depth = 4;
      tz(face_plate_zoffset - overlap) cylinder(
          d=mounting_hole_diameter, h=mounting_hole_depth + overlap);
    }

    txy(mounting_hole_xoffset, mounting_hole_yoffset) hole();
    txy(mounting_hole_xoffset, mounting_hole_yoffset + mounting_hole_yspan) hole();
    txy(mounting_hole_xoffset + mounting_hole_xspan, mounting_hole_yoffset + mounting_hole_yspan) hole();
    txy(mounting_hole_xoffset + mounting_hole_xspan, mounting_hole_yoffset) hole();
  }

  difference() {
    plate();
    epaper_viewport();
    mounting_holes();
  }
}


module display_with_face_plate() {

  module epaper_display() {
    ty(waveshare_epaper_width - waveshare_epaper_bolt_inset * 2) rz(-90) waveshare_epaper_2_9in();
  }

  translate([
     -epaper_x_offset,
     -epaper_y_offset,
     epaper_z_offset,
  ]) rx(epaper_display_angle) {
    epaper_display();
    face_plate();
  }
}

// Can be made with CNC (acrylic or similar) or 3D printed
module top_cover() {
  cutaway_width = 5.4;
  cover_pad = 0.1;

  module button_access_hole() {
    tz(-overlap) cylinder(d=button_access_hole_diameter, h=top_cover_thickness + overlap * 2);
  }

  color("#ddd", 0.4) tz(base_thickness + pcb_zpad + top_shell_zsize + pcb_thickness - top_cover_thickness)
    difference() {
      base_template(top_cover_thickness, top_cover_inset + cover_pad);
      translate([-overlap, 0, -overlap]) cube([base_xsize, cutaway_width, top_cover_thickness + overlap * 2]);
      txy(button_a_x, button_y) button_access_hole();
      txy(button_b_x, button_y) button_access_hole();
    }
}

module top_cover_projection() {
  projection() top_cover();
}

module face_plate_projection() {
  projection(cut=true) tz(-3) face_plate();
}

module cover_buttons() {
  button_depth = top_shell_zsize - pcb_zpad - pcb_thickness - 3.75;
  translate([
      button_a_x,
      button_y,
      base_thickness + pcb_zpad + top_shell_zsize + pcb_thickness - top_cover_thickness - button_depth]) rz(-90) buttons(
        hole_diameter=button_access_hole_diameter - 0.15,
        hole_thickness=top_cover_thickness,
        hole_span=button_b_x - button_a_x,
        button_size=3,
        button_depth=button_depth,
        button_protrusion=1.5,
        use_hull = true,
        show_carriage=true,
        show_button=true);
}

$fa=2.0;
$fs=0.5;
// Prefix with ! and Set to false to hide parts for model export
epaper_clock_case(true);
display_with_face_plate();
top_cover();
cover_buttons();
// change * to ! to produce projections for CNC cutting
*top_cover_projection();
*face_plate_projection();
