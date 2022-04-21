module tx(x) translate([x,0,0]) children();
module ty(y) translate([0,y,0]) children();
module tz(z) translate([0,0,z]) children();

// Planes
module txy(x, y) translate([x,y,0]) children();
module txz(x, z) translate([x,0,z]) children();
module tyz(y, z) translate([0,y,z]) children();

module boardxy(x, y, pin_spacing=2.54) {
  translate([x * pin_spacing, y * pin_spacing,0]) children();
}


module rx(x) rotate([x,0,0]) children();
module ry(y) rotate([0,y,0]) children();
module rz(z) rotate([0,0,z]) children();
