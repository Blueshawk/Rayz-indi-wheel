# indi-wheel
   A custom Arduino controlled, Indi compatible DIY filter wheel for remote astronomy/AP 

By: Ray Wells

CC: Markku Siitonen who provided the skeleton, Jasem Mutlaq, for his continued tireless support at Indilib.

  This motorized remote filter wheel uses an Arduino Nano with a 24byj-48 gearhead stepper motor and the uln2003 driver board that comes with it.  It is powered by 5v from the usb port in bipolar configuration.

Hardware
A generic 5 position manual filter wheel with the stepper's double flat shaft epoxied into the wheel center screw hole (after removing the screw of course).
I used header pins to connect the driver board directly to the nano to reduce wiring mess and a single hall effect(HET) that came with an on board comparator(for a digital output) powered by nano i/o for simplicity. To get around the calibration errors that sometimes creep into these systems, I set it so it would only move one way to prevent backlash. It also resets to the home position(HET) whenever it would normally move backward and then moves forward from 0 to the new filter position.

Whats new?: 
    - Offsets can now be entered as "Fnnnn" and will change the position offset for the currently selected filter.
    - "G0" now saves the offsets to eprom, and they are loaded from eprom on restart. Many unused commands have been removed. 

Cancelled driver development and decided to make it compatable with the Zagyl wheel ptorocol used in indi_zagyl_wheel.
