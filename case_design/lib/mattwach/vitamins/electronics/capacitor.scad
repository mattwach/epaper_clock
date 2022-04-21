
module electrolytic_capacitor(
    diameter,
    height,
    pin_spacing,
    label_text,
    pin_length=4,
    pin_diameter=0.5,
    hover = 1.5) {
  overlap = 0.1;
  module pin() {
    translate([0, 0, hover - pin_length])
      cylinder(d=pin_diameter, h=pin_length + overlap);
  }
  module can() {
    color("#444") 
      translate([0, pin_spacing / 2, hover])
      cylinder(d=diameter, h=height);
  }
  module can_labels() {
    translate([0, pin_spacing / 2, hover + overlap])
      cylinder(d=diameter + overlap * 2, h=height - overlap * 2);
  }
  module label() {
    color("#ddd") intersection() {
      translate([0, diameter / 2 - pin_spacing / 2, (height + hover) / 2])
        rotate([0, 90, 0])
        linear_extrude(diameter / 2 + overlap)
        text(label_text, size=diameter / 2.5, halign="center", valign="center");
      can_labels();
    }
  }
  module polatiry_label() {
    module cutout() {
      translate([0, pin_spacing / 2, height / 2 + hover]) cube([diameter + overlap,  diameter + overlap, height + overlap * 4], center=true);
    }
    module minus() {
      translate([-diameter / 12, pin_spacing / 2 - diameter / 2 - overlap, 0]) cube([diameter / 6, diameter, height / 4]);
    }
    color("#ddd") difference() {
      can_labels();
      translate([-diameter * 3 / 4, 0, 0]) cutout();
      translate([diameter * 3 / 4, 0, 0]) cutout();
      translate([0, diameter / 2, 0]) cutout();
      translate([0, 0, hover + height * 0.5]) minus();
    }
  }

  pin();
  translate([0, pin_spacing, 0]) pin();
  can();
  label();
  polatiry_label();
}

module ceramic_capacitor(
    label_text,
    height=3.8,
    thickness=2.7,
    pin_spacing=5.08,
    pin_length=3,
    pin_diameter=0.5,
    hover = 0.5) {
  overlap = 0.1;
  module pin() {
    translate([0, 0, hover - pin_length])
      cylinder(d=pin_diameter, h=pin_length + overlap);
  }
  module body() {
    color("#850") hull() { 
      translate([0, pin_spacing / 2, hover + height - thickness / 2]) sphere(d=thickness);
      translate([0, 0, hover + pin_diameter]) sphere(d=pin_diameter * 2);
      translate([0, pin_spacing, hover + pin_diameter]) sphere(d=pin_diameter * 2);
    }
  }
  module body_shell() {
    color("#850") hull() { 
      translate([0, pin_spacing / 2, hover + height - thickness / 2]) sphere(d=thickness + overlap * 2);
      translate([0, 0, hover + pin_diameter]) sphere(d=pin_diameter * 2 + overlap * 2);
      translate([0, pin_spacing, hover + pin_diameter]) sphere(d=pin_diameter * 2 + overlap * 2);
    }
  }
  module label() {
    color("#222") intersection() {
      translate([0, pin_spacing / 2, hover + height / 2])
        rotate([90, 0, 90])
        linear_extrude(thickness / 2 + overlap)
        text(label_text, size=pin_spacing / 4, halign="center", valign="center");
        body_shell();
    }
  }

  body();
  pin();
  translate([0, pin_spacing, 0]) pin();
  label();
}

module electrolytic_capacitor_100f_16v() {
  electrolytic_capacitor(
      diameter=5.2,
      height=11.4,
      pin_spacing=2.54,
      label_text="100u");
}

module electrolytic_capacitor_220f_16v() {
  electrolytic_capacitor(
      diameter=6.4,
      height=12.1,
      pin_spacing=3,
      label_text="220u");
}

module electrolytic_capacitor_470f_16v() {
  electrolytic_capacitor(
      diameter=8.1,
      height=12.6,
      pin_spacing=4.1,
      label_text="470u");
}
