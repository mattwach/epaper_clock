use <../../util.scad>

// Eachs pins_ is an array of the form [count, spacing]
module smd_chip_detailed(
    length,
    width,
    thickness,
    pin_length,
    pins_n=[0,0],
    pins_s=[0,0],
    pins_e=[0,0],
    pins_w=[0,0]) {
  module pin_set(spacing, count, len) {
    module pin() {
      pin_width = spacing / 2;
      color("silver") ty(pin_width/2) cube([pin_length, pin_width, thickness * 0.5]);
    }

    if (count > 0) {
      span = count * spacing;  
      y0 = (len - span) / 2;
      for (i = [0:count-1]) {
        ty(y0 + spacing * i) pin();
      }
    }
  }
  color("#222") cube([length, width, thickness]);
  rz(-90) pin_set(pins_s[1], pins_s[0], length);
  ty(width + pin_length) rz(-90) pin_set(pins_n[1], pins_n[0], length);
  tx(-pin_length) pin_set(pins_w[1], pins_w[0], width);
  tx(length) pin_set(pins_e[1], pins_e[0], width);
}


module smd_chip(length, width, thickness, pin_spacing, pin_length, pin_count_length, pin_count_width) {
  smd_chip_detailed(
      length = length,
      width = width,
      thickness = thickness,
      pin_length = pin_length,
      pins_n = [pin_count_length, pin_spacing],
      pins_s = [pin_count_length, pin_spacing],
      pins_e = [pin_count_width, pin_spacing],
      pins_w = [pin_count_width, pin_spacing]);
}

/*
$fa=2;
$fs=0.5;
smd_chip_detailed(
    length=7,
    width=5,
    thickness=0.8,
    pin_length=2.2,
    pins_n=[2, 2],
    pins_s=[3, 1],
    pins_e=[4, 0.9],
    pins_w=[5, 0.8]);
*/
