Different Hardware Configurations:

   * `arduino_nano_with_cheap_gps`:  The "easy" design which uses a minimum part count
     and is intended to be powered with a USB power adapter.
   * `raw_atmega328p_with_adafruit_gps`: Uses raw Atmega328P and an Adafruit
       GPS unit.  A good match for the 32k oscillator firmware for long battery life.
   * `arduino_pro_mini_with_cheap_gps`: Uses an arduino pro mini with an inexpensive GPS
      unit and MOSFET shutoff.  The pro mini can be hacked with with a 32k oscillator
      (see the main README.md for more info)  

You can mix and match above, for example raw atmega328p + cheap gps, but will need to make
layout corrections (or make a simple adapter board) if you are using the PCB layout.
