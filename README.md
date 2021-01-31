# Rayz-indi-wheel
   A custom Arduino controlled, Indi compatible DIY filter wheel for remote astronomy/AP 

By: Ray Wells

CC: Markku Siitonen who provided the skeleton, Jasem Mutlaq, for his continued tireless support at Indilib.

  As with all my DIY projects, my goal is to make an electronic filter wheel as affordable as possible. This motorized remote filter wheel uses an Arduino Nano with a 24byj-48 gearhead stepper motor and the uln2003 driver board that comes with it. The wheel is turned by a small gerhead stepper motor like the type used to move flaps in car dashboards, and powered by 5v from the usb port. A powered hub is recommended). To get around the calibration errors that sometimes creep into these systems I set it up to only turn clockwise at all times. If a filter is ccw from the current one, the unit first resets to the home position(leading edge of the het) and then moves cw from 0 to the new filter position. The zero point will be in error if the wheel is already over the hall effect for any reason, so the wheel will back up far enough to clear the het, and then rehome on the leading edge again. This is the only time it runs backward(ccw). Other than the time I ripped out the transmission in the cheap little motor. I've not had a blocked filter using this setup. The code is annotated with descriptions, hopefully in a helpful way. I'm excited to find folks giving this design a try. Please let me know if you built one of these!


Hardware:
--Any small stepper motor - I used the 24byj-48 that comes with a uln2003 driver board with leds available at ebay and amazon.
--A generic 5 position manual filter wheel(ebay had them for about 35$ when I did this)
--a small powerful magnet.
--a reed switch or board mounted hall effect sensor-switch. The het I used had a comparator that turned it into a switch, giving a very sharp position for home.
--glues, doublebacktape, some foam stuff to block light from getting into the finger hole. 

How I made mine:
 I took the center axle screw out of the wheel and used epoxy to glue the double flat output shaft of the stepper into the wheel center screw hole so it would turn the inner wheel.
I soldered idc header pins to the driver board and used them as a standoff to directly bridge to the nano to shorten wires and simplify mounting.
Glue the magnet between between 4-5 on the inner wheel so it passes the finger opening and mount the Hall effect or reed switch nearby to act as the "home" or zero position. 

Setup filter positions using a serial terminal like the one in Ardunio ide.
  - each filter offset can be entered as "Fnnnn" and it will change the position offset for the currently selected filter, then move to show the new position after passing home. This makes desktop setup easy for different motor/drive assemblies.
  - "G0" saves the offsets to eprom, and they are loaded from eprom on restart. 

Pinouts as defined for arduino nano:
  Power = 5v from usb hub

Motor Wiring:
motor Pin1 = 22  = IN1 on ULn2003 
motor Pin2 = 21  = IN2 on ULn2003
motor Pin3 = 23  = IN3 on ULn2003
motor Pin4 = 20  = IN4 on ULn2003

The Motor plugs into the driver board using the included socket 
Be sure to check wiring order as they sometimes mix up the colors. It should be smooth and not vibrating or wobbling when connected okay. Also note that changing the numbers in the ino is easier than rewiring.


Reed switch wiring:
   A1 = common
   A3 = input --define as input_pullup

 --OR--
 
Hall effect transistor(HET) wiring using comparator card:
    A1 = common
    A2 = Vcc(5v)
    A3 = d0 digital output from HET 0-5v
    
Whats new?: 
 - Cancelled driver development and decided to make it compatable with the Zagyl wheel protocol used in indi_zagyl_wheel.
 - Renamed repository so it doesn't show on indilib searches and derail new users.


todo:
- make a protocol command list, example pinout and photos. 
- have head screwed on tighter so I don't leave half baked code laying around for some noob to find.
++I can't really recommend using more than one cheapduino in a system as they have no serial number I.e (0) to make a proper udev rule. I can recommend a teensy from France.
More info as I get time... 

Some pics.
