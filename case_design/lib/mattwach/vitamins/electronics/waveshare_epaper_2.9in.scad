use <../../util.scad>
use <../../shapes.scad>

waveshare_epaper_width = 38;
waveshare_epaper_length = 89.5;
waveshare_epaper_thickness = 1.6;
waveshare_epaper_bolt_inset = 2.5;
waveshare_epaper_pixel_area_width = 68.5;
waveshare_epaper_pixel_area_height = 31;
waveshare_epaper_screen_height = 36.8;
waveshare_epaper_screen_width = 79.1;
waveshare_epaper_screen_xoffset = (waveshare_epaper_width - waveshare_epaper_screen_height) / 2;
waveshare_epaper_screen_yoffset = (waveshare_epaper_length - waveshare_epaper_screen_width) / 2;
waveshare_epaper_screen_thickness = 1.1;
waveshare_epaper_pixel_area_xoffset =  waveshare_epaper_screen_xoffset + (waveshare_epaper_screen_height - waveshare_epaper_pixel_area_height) / 2;
waveshare_epaper_pixel_area_yoffset = waveshare_epaper_screen_yoffset + 2.4;
waveshare_epaper_pixel_area_zoffset = waveshare_epaper_thickness + waveshare_epaper_screen_thickness;

module waveshare_epaper_2_9in(wire_length=0) {
  overlap = 0.01;
  connector_zsize = 5.7;

  module pcb() {
    module bolt_hole() {
      bolt_hole_diameter = 2;
      tz(-overlap) cylinder(
          d=bolt_hole_diameter, h=waveshare_epaper_thickness+(overlap * 2));
    }

    module board() {
      color("navy")
        rounded_cube([
            waveshare_epaper_width,
            waveshare_epaper_length,
            waveshare_epaper_thickness],
            fillet=2);
    }

    difference() {
      board();
      txy(waveshare_epaper_bolt_inset,
          waveshare_epaper_bolt_inset) bolt_hole();
      txy(waveshare_epaper_bolt_inset,
          waveshare_epaper_length - waveshare_epaper_bolt_inset) bolt_hole();
      txy(waveshare_epaper_width - waveshare_epaper_bolt_inset,
          waveshare_epaper_length - waveshare_epaper_bolt_inset) bolt_hole();
      txy(waveshare_epaper_width - waveshare_epaper_bolt_inset,
          waveshare_epaper_bolt_inset) bolt_hole();
    }
  }

  module screen() {
    module screen_base() {
      color("#bbb") cube([waveshare_epaper_screen_height, waveshare_epaper_screen_width, waveshare_epaper_screen_thickness]);
    }

    module pixel_area() {
      pixel_area_zsize = 0.2;
      color("#aaa") cube([
          waveshare_epaper_pixel_area_height, waveshare_epaper_pixel_area_width, pixel_area_zsize]);
    }

    module ribbon_connector() {
      ribbon_connector_xsize = 14.7;
      ribbon_connector_ysize = 6.3;
      ribbon_connector_zsize = 1.2;

      color("#822") translate([
        (waveshare_epaper_screen_height -ribbon_connector_xsize) / 2,
        waveshare_epaper_screen_width,
        0,
      ]) cube([
          ribbon_connector_xsize,
          ribbon_connector_ysize,
          ribbon_connector_zsize]);
    }

    translate([
        waveshare_epaper_screen_xoffset,
        waveshare_epaper_screen_yoffset,
        waveshare_epaper_thickness]) {
      screen_base();
      ribbon_connector();
    }
    translate([
        waveshare_epaper_pixel_area_xoffset,
        waveshare_epaper_pixel_area_yoffset,
        waveshare_epaper_pixel_area_zoffset]) {
      pixel_area();
    }
  }

  module connector() {
    connector_xsize = 18.8;
    connector_ysize = 6;

    color("#ddd") translate([
        (waveshare_epaper_width - connector_xsize) / 2,
        0,
        -connector_zsize]) cube([
          connector_xsize, connector_ysize, connector_zsize]);
  }

  module ribbon_iface() {
    ribbon_iface_xsize = 15.5;
    ribbon_iface_ysize = 5.7;
    ribbon_iface_zsize = 2;
    ribbon_y_offset = 5.2;

    color("#ddd") translate([
        (waveshare_epaper_width - ribbon_iface_xsize) / 2,
        waveshare_epaper_length - ribbon_iface_ysize - ribbon_y_offset,
        -ribbon_iface_zsize]) cube([
          ribbon_iface_xsize, ribbon_iface_ysize, ribbon_iface_zsize]);
  }

  module wires() {
    wire_span = 16;
    wire_spacing = wire_span / 7;

    module wire() {
      translate([
        (waveshare_epaper_width - wire_span) / 2,
        0,
        -connector_zsize / 2
      ]) rx(90) cylinder(d=1.5, h=wire_length);
    }

    if (wire_length > 0) {
      color("red") wire();
      tx(wire_spacing) color("#222") wire();
      for (i = [2:7]) {
        tx(wire_spacing * i) color("#344") wire();
      }
    }
  }

  txy(-waveshare_epaper_bolt_inset, -waveshare_epaper_bolt_inset) {
    pcb();
    screen();
    connector();
    ribbon_iface();
    wires();
  }
}

/*
$fa=2;
$fs=0.5;
waveshare_epaper_2_9in(10);
*/
