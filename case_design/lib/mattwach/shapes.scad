//also look at NopSCADlib/utils/core/ for more shapes

// Parameters:
// d: diameter
// h: height
// fillet: lower fillet.  <0 fillets out, >= fillets in, ==0 no fillet
// chamfer: lower fillet.  <0 fillets out, >= fillets in, ==0 no fillet
module rounded_cylinder(d=1, h=1, fillet=0, chamfer=0, fillet2=0, chamfer2=0) {
  assert(fillet != 0 || chamfer != 0 || fillet2 != 0 || chamfer2 != 0, "fillet or chamfer should be non-zero");
  assert(!(fillet == 0 && chamfer == 0 && fillet2 == 0 && chamfer2 == 0), "filler of chamfer should be zero");
  clearance = abs(fillet) + abs(chamfer);
  clearance2 = abs(fillet2) + abs(chamfer2);
  overlap = 0.01;

  main_zoffset = clearance > 0 ? clearance - overlap : 0;
  main_height = clearance > 0 && clearance2 > 0 ?
    h - clearance - clearance2 + overlap * 2 :
    h - clearance - clearance2 + overlap;

  module inner_fillet(r) {
    union() {
      rotate_extrude() {
        translate([d / 2 - r, r, 0]) circle(r=r);
      }
      cylinder(d=d - r * 2, h=r * 2);
    }
  }

  union() {
    translate([0, 0, main_zoffset]) cylinder(d=d, h=main_height);

    if (fillet > 0) {
      inner_fillet(clearance);
    }

    if (fillet2 > 0) {
      translate([0, 0, h - clearance2 * 2]) inner_fillet(clearance2);
    }

    if (fillet < 0) {
      difference() {
        cylinder(d=d + clearance * 2, h=clearance);
        rotate_extrude() {
          translate([d / 2 + clearance, clearance, 0]) circle(r=clearance);
        }
      }
    }

    if (fillet2 < 0) {
      translate([0, 0, h - clearance2]) difference() {
        cylinder(d=d + clearance2 * 2, h=clearance2);
        translate([0, 0, -clearance2]) rotate_extrude() {
          translate([d / 2 + clearance2, clearance2, 0]) circle(r=clearance2);
        }
      }
    }

    if (chamfer != 0) {
      cylinder(d1=d - chamfer * 2, d2=d, h=clearance); 
    }

    if (chamfer2 != 0) {
      translate([0, 0, h - clearance2])
        cylinder(d1=d, d2=d - chamfer2 * 2, h=clearance2); 
    }
  }
}

// Create a cube with chamfers.  chamfer_top, chamfer_bottom can be positive
// (chamfer in) or negative (chamfer out)
module chamfered_cube_z(size, chamfer_top=0, chamfer_bottom=0) {
  xsize = size[0];
  ysize = size[1];
  zsize = size[2];
  overlap = 0.01;

  union() {
    hull() {
      translate([
          chamfer_top,
          chamfer_top,
          zsize - overlap]) cube([
            xsize - chamfer_top * 2,
            ysize - chamfer_top * 2,
            overlap]);
      translate([
        0,
        0,
        zsize - abs(chamfer_top) - overlap]) cube([
          xsize,
          ysize,
          overlap]);
    }

    translate([
      0,
      0,
      abs(chamfer_bottom)]) cube([
        xsize,
        ysize,
        zsize - abs(chamfer_bottom) - abs(chamfer_top)]);

    hull() {
      translate([
          chamfer_bottom,
          chamfer_bottom,
          0]) cube([
            xsize - chamfer_bottom * 2,
            ysize - chamfer_bottom * 2,
            overlap]);
      translate([
        0,
        0,
        abs(chamfer_bottom)]) cube([
          xsize,
          ysize,
          overlap]);
    }
  }
}

module chamfered_cube_x(size, chamfer_x0=0, chamfer_x1=0) {
  xsize = size[0];
  ysize = size[1];
  zsize = size[2];
  overlap = 0.01;

  union() {
    hull() {
      translate([
          0,
          chamfer_x0,
          chamfer_x0]) cube([
            overlap,
            ysize - chamfer_x0 * 2,
            zsize - chamfer_x0 * 2]);
      translate([
          abs(chamfer_x0),
          0,
          0]) cube([
            overlap,
            ysize,
            zsize]);
    }

    translate([
      abs(chamfer_x0),
      0,
      0]) cube([
        xsize - abs(chamfer_x0) - abs(chamfer_x1),
        ysize,
        zsize]);

    hull() {
      translate([
          xsize - overlap,
          chamfer_x1,
          chamfer_x1]) cube([
            overlap,
            ysize - chamfer_x1 * 2,
            zsize - chamfer_x1 * 2]);
      translate([
          xsize - abs(chamfer_x1) - overlap,
          0,
          0]) cube([
            overlap,
            ysize,
            zsize]);

    }
  }
}

module chamfered_cube_y(size, chamfer_y0=0, chamfer_y1=0) {
  xsize = size[0];
  ysize = size[1];
  zsize = size[2];
  overlap = 0.01;

  hull() {
    translate([
        chamfer_y0,
        0,
        chamfer_y0]) cube([
          xsize - chamfer_y0 * 2,
          overlap,
          zsize - chamfer_y0 * 2]);
    translate([
        0,
        abs(chamfer_y0),
        0]) cube([
          xsize,
          overlap,
          zsize]);
  }

  union() {
    translate([
      0,
      abs(chamfer_y0),
      0]) cube([
        xsize,
        ysize - abs(chamfer_y0) - abs(chamfer_y1),
        zsize]);
  }

  hull() {
    translate([
        chamfer_y1,
        ysize - overlap,
        chamfer_y1]) cube([
          xsize - chamfer_y1 * 2,
          overlap,
          zsize - chamfer_y1 * 2]); 
    translate([
        0,
        ysize - abs(chamfer_y1) - overlap,
        0]) cube([
          xsize,
          overlap,
          zsize]);
  }
}

module rounded_cube(size, fillet) {
  xsize = size[0];
  ysize = size[1];
  zsize = size[2];
  hull() {
    translate([fillet, fillet, 0]) cylinder(r=fillet, h=zsize);
    translate([xsize - fillet, fillet, 0]) cylinder(r=fillet, h=zsize);
    translate([xsize - fillet, ysize - fillet, 0]) cylinder(r=fillet, h=zsize);
    translate([fillet, ysize - fillet, 0]) cylinder(r=fillet, h=zsize);
  }
}


/*
$fn=36;
translate([0, 0, 0]) cylinder(d=5, h=10);
color("red") translate([-8, 0, 0]) rounded_cylinder(d=5, h=10, fillet=1);
color("green") translate([-16, 0, 0]) rounded_cylinder(d=5, h=10, fillet=-1);
color("purple") translate([-24, 0, 0]) rounded_cylinder(d=5, h=10, chamfer=1);
color("orange") translate([-32, 0, 0]) rounded_cylinder(d=5, h=10, chamfer=-1);

translate([0, 8, 0]) cylinder(d=5, h=10);
color("red") translate([-8, 8, 0]) rounded_cylinder(d=5, h=10, fillet2=1);
color("green") translate([-16, 8, 0]) rounded_cylinder(d=5, h=10, fillet2=-1);
color("purple") translate([-24, 8, 0]) rounded_cylinder(d=5, h=10, chamfer2=1);
color("orange") translate([-32, 8, 0]) rounded_cylinder(d=5, h=10, chamfer2=-1);

translate([0, 16, 0]) cylinder(d=5, h=10);
color("red") translate([-8, 16, 0]) rounded_cylinder(d=5, h=10, fillet=1, fillet2=1);
color("green") translate([-16, 16, 0]) rounded_cylinder(d=5, h=10, fillet=-1, fillet2=-2);
color("purple") translate([-24, 16, 0]) rounded_cylinder(d=5, h=10, chamfer=1, chamfer2=2);
color("orange") translate([-32, 16, 0]) rounded_cylinder(d=5, h=10, chamfer=-1, chamfer2=-2);

chamfered_cube_x([10, 10, 10], chamfer_x0=-1, chamfer_x1=-2);
chamfered_cube_y([10, 10, 10], chamfer_y0=-1, chamfer_y1=-2);
chamfered_cube_z([10, 10, 10], chamfer_top=-1, chamfer_bottom=-2);
*/



