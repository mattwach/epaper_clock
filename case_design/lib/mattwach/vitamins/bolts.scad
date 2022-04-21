use <../util.scad>

module bolt(length, diameter, head_length, head_diameter) {
  overlap = 0.01;
  color("#99d") union() {
    cylinder(d=head_diameter, h=head_length);
    tz(-length) cylinder(d=diameter, h=length + overlap);
  }
}

module bolt_M2(length) {
  bolt(length, 1.9, 1.8, 3.6);
}

module bolt_M2_5(length) {
  bolt(length, 2.5, 2.5, 4.5);
}

module bolt_M3(length) {
  bolt(length, 2.9, 3.1, 5.4);
}

module solid_nut(flat_side_width, height) {
  cylinder(
    d=flat_side_width / cos(30),
    h=height,
    $fn=6
  );
}

module nut(flat_side_width, height, hole_diameter) {
  overlap = 0.01;
  difference() {
    solid_nut(flat_side_width, height);
    tz(-overlap) cylinder(
      d = hole_diameter,
      h = height + overlap * 2
    );
  }
}

module nut_M2() {
  nut(3.9, 1.5, 2);
}

module nut_M3() {
  nut(5.4, 2.3, 3);
}
