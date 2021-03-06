//insert titles here

#include <AccelStepper.h>
#include <EEPROM.h>
#include <avr/wdt.h>

//Global declarations
word filterPos[] = { 0, 700, 1400, 2100, 2800, 3500}; //play with these to course align each filter - only need to do it once.
word posOffset[] = { 0, 0, 0, 0, 0, 0 };
bool Error = false;                      // Error flag
String inLine;                           // Current command.
char command;                            // Command
byte currPos = 1;                        // Start up with 1
int newPos = 1;                          // Just start somewhere
bool cmdOK = false;                      // Command ok ?
int PWMvalue = 128;                      // Set PWM to half
int CalibrationOffset = 0;               //for indi info
int maxSpeed = 400;                      // max motor speed
int setSpeed = maxSpeed;                      //current step speed (apparently max is used by the library?)
int setAccel = 1000;                     //
// Hall pin definition
/*  current wiring
    A0 = d0 digital(comparator)output
    A1 = vcc
    A2 = gnd
    A3 = a0 analog out
*/

const int SENSOR = A3;       // PIN A3 = Hall effect switch - onboard comparator with predefined value (red)
const int hallPower = A1; //(a1 orig)    // PIN A1 = Hall effect supply - pull high   (blue)
const int hallCom  = A2;  //(a2 orig)   // PIN A2 = Hall effect common - pull low     (black)
const int d0 = A0;           // hall effect analog input - not used 2/18/18 (green)

// Motor definitions
#define STEPS 4                        // 28BYJ-48 steps 4 or 8

// Motor pin definitions
#define motorPin1  9         //6 IN1 on ULn2003
#define motorPin2  6         //9 IN2 on ULn2003
#define motorPin3  8         //7 IN3 on ULn2003
#define motorPin4  7         //8 IN4 on ULn2003

// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper with 28BYJ-48
AccelStepper stepper(STEPS, motorPin1, motorPin3, motorPin2, motorPin4);

void setup() {
  pinMode(hallCom, OUTPUT);         //hall gnd
  pinMode(hallPower, OUTPUT);       //hall power
  pinMode(SENSOR, INPUT_PULLUP);           //(A3) hall signal
  pinMode(6, OUTPUT);               //motor +a
  pinMode(7, OUTPUT);               //motor -a
  pinMode(8, OUTPUT);               //motor +b
  pinMode(9, OUTPUT);               //motor -b

  digitalWrite(hallCom, LOW);     // set hall gnd to 0v
  digitalWrite(hallPower, HIGH);    //set hall power to 5v

  Serial.begin(9600);
  //while ( !Serial ) {
  // Wait until Serial is available.
  //}
  Serial.flush();

  wdt_disable();

  // Set stepper stuff
  stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(maxSpeed);    //maximum step rate
  stepper.setSpeed(setSpeed);       //current step rate
  stepper.setAcceleration(setAccel);     //"1000 = 100%" accellerationt to set step rate --check this in accelStepper

  // read offsets from eaprom
  for ( int i = 1; i < 6 ; i++ ) {
    posOffset[i] = word( EEPROM.read(i * 2 + 50), EEPROM.read(i * 2 + 51));
  }

  //intial calibration.
  Locate_Home();                          // Rotate to index 0
  Locate_Slot_x();                             //go to first filter offset



} // **** END OF SETUP ****

void loop() {

  // Get incoming command.
  Serial.setTimeout(250);
  inLine = Serial.readStringUntil('\n');


  // Run debugProcedure                       -todo --- use to store changes to eprom..
  if ( inLine == "G0" ) {
    cmdOK = true;
    debugProcedure();  // Go do some debugging..
    return;
  }
  // Product name
  if ( inLine == "I0" ) {
    cmdOK = true;
    Serial.println("Rayzwheel b.097");
  }

  // Firmware version
  if ( inLine == "I1" ) {
    cmdOK = true;
    Serial.println("0.98");
  }
  // Current filter pos
  if ( inLine == "I2" ) {
    cmdOK = true;
    Serial.print("P");
    Serial.println(currPos);
  }

  // Serial number
  if ( inLine == "I3" ) {
    cmdOK = true;
    Serial.println("sn.000001");
  }

  // Display the maximum rotation speed - "MaxSpeed XXX%"
  if ( inLine == "I4" ) {
    cmdOK = true;
    Serial.println(maxSpeed);
  }

  // Display sensor position offset for current filter position - "PX Offset XX"
  if ( inLine == "I6" ) {
    cmdOK = true;
    Serial.print("P");
    Serial.print(currPos);
    Serial.print(" Offset ");
    Serial.println( posOffset[currPos] );
  }

  // Display the number of available filter slots - "FilterSlots X"
  if ( inLine == "I8" ) {
    cmdOK = true;
    Serial.println("FilterSlots 5");
  }

  // Send Current filter offset value  - "PX Offset XXXX"
  if ( inLine == "O1" ) {
    cmdOK = true;
    Serial.print("P1");
    Serial.print(" Offset ");
    Serial.println( posOffset[1] );
  }

  // Send Current filter offset value  - "PX Offset XXXX"
  if ( inLine == "O2" ) {
    cmdOK = true;
    Serial.print("P2");
    Serial.print(" Offset ");
    Serial.println( posOffset[2] );
  }

  // Send Current filter offset value  - "PX Offset XXXX"
  if ( inLine == "O3" ) {
    cmdOK = true;
    Serial.print("P3");
    Serial.print(" Offset ");
    Serial.println( posOffset[3] );
  }


  // Send Current filter offset value  - "PX Offset XXXX"
  if ( inLine == "O4" ) {
    cmdOK = true;
    Serial.print("P4");
    Serial.print(" Offset ");
    Serial.println( posOffset[4] );
  }


  // Send Current filter offset value  - "PX Offset XXXX"
  if ( inLine == "O5" ) {
    cmdOK = true;
    Serial.print("P5");
    //Serial.print(currPos);
    Serial.print(" Offset ");
    Serial.println( posOffset[5] );
  }


  // Hall-sensor data..
  if ( inLine == "T0" ) {
    cmdOK = true;
    Serial.print("Sensors ");
    Serial.print(digitalRead(SENSOR));
    Serial.print(" ");
    Serial.println(digitalRead(SENSOR));
  }

  // Filter select G1 - G5
  command = inLine.charAt(0);         // command+newPos=Goto pos newPos
  newPos = int(inLine.charAt(1) - 48); // newPos=int -48 if no input..

  if ( newPos == -48 ) {
    return;
  }

  if ( command == 'G' && ( newPos >= 1 || newPos <= 5 ) ) {

    if ( newPos == 1 ) {
      Locate_Home();
    }

    if ( newPos > 5 ) {
      newPos = currPos;
    }

    if ( newPos != currPos ) {
      Locate_Slot_x();
    }
    Serial.print("P");
    Serial.println(currPos);
    delay(100);
  }

  // Set Offset of current filter to array. (save to eprom with G0)
  if ( command == 'F' ) {
    cmdOK = true;
    newPos = currPos;
    posOffset[currPos] = (inLine.substring(1).toInt());   //get the offset int value from the serial string
    Locate_Home();
    Locate_Slot_x();
    Serial.print("P");
    Serial.print(currPos);
    Serial.print(" Offset ");
    Serial.println( posOffset[currPos] );
  }


  // Hard reboot --this should be a cpu reset command
  if ( inLine == "R0" ) { 
    //Good luck.
  }


  // Initialize, restarts and moves to filter position 1.
  if ( inLine == "R1" ) {
    cmdOK = true;
    Locate_Home();  // currPos will be 1.
    delay(500);
    Serial.print("P");
    Serial.println(currPos);
  }


  // Reset all calibration values to 0. Handy for first run/calibrating, if you then save zeroes to eprom with G0
  if ( inLine == "R2" ) {
    cmdOK = true;
    currPos = 1;
    for ( int i = 1; i < 6 ; i++ ) {
      posOffset[i] = 0;
      if (i == 5) {
        Serial.println("Offsets set to zero but not stored. Store new offsets with G0");
      }
    }
  }

  // If command not recognized, flush buffer and wait for next command..
  if ( cmdOK ) {
    Serial.flush();
    cmdOK = false;
    delay(100);
    return;
  }
}   ////////// End of main loop.. //////////


//            *********Find Home********
void Locate_Home() {
  int HallValue = digitalRead(SENSOR);                       // read the hall sensor value
  if (HallValue == LOW) {
    stepper.runToNewPosition((filterPos[currPos]) - 300);    //move off magnet before homing
    delay(200); //avoid plugging motor - allow wheel to stop.
  }
  HallValue = digitalRead(SENSOR);                           // recheck the hall sensor value
  stepper.moveTo(10000);
  if (stepper.distanceToGo() <= 0 ) {
    Serial.println("Error:ET Can't find home");              //if it runs too long stop and error
    motor_Off();
    Error = true;
  }
  while (HallValue == HIGH)  {                               //while hall is  just run motor and read sensor
    stepper.run();
    HallValue = digitalRead(SENSOR);
  }                                                          //when hall goes LOW set position as home and move to selected filter
  stepper.stop();
  stepper.setCurrentPosition(0);
  delay(300); //let motor settle from decel
  currPos = 0;
  // Locate_Slot_x(); //this might be better handled separately
}

// Move to slots 1..5 ***************************************************
// Always run wheel the same direction to avoid backlash issues.

void Locate_Slot_x() {
  if ( filterPos[newPos] < filterPos[currPos] ) {            //prevent backwards movement to avoid backlash errors
    Locate_Home();
  }
  int pos_ofs = filterPos[newPos] + posOffset[newPos];
  stepper.runToNewPosition(pos_ofs);                          // move to position indicated by arrays
  currPos = newPos;                                           // Tell caller this is the new requested position
  motor_Off();
  return;
}

void motor_Off() {                                            //power down the stepper to save battery
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  return;
}

// Show some values and reset error flag.
void debugProcedure() {
  int i = 0;
  word Value;

  //EEPROM.write(address, value);
  for ( i = 1; i < 6 ; i++ ) {
    EEPROM.write((i * 2) + 50, highByte(posOffset[i]));
    EEPROM.write((i * 2) + 51, lowByte(posOffset[i]));
  }
  //  Serial.println("Default values written to EEPROM!!");

  Serial.println("\n");
  Serial.println("-------------------------------");
  Serial.print("Current Offset : ");
  Serial.println( posOffset[currPos] );
  Serial.println();

  Serial.println("offset values in eaprom");
  for ( i = 1; i < 6 ; i++ ) {
    Value = word( EEPROM.read(i * 2 + 50), EEPROM.read(i * 2 + 51));
    Serial.print("Offset: ");
    Serial.print( i );
    Serial.print(" = ");
    Serial.println( Value );
  }

  if ( !Error ) {
    Serial.println("Normal operation resumed");
  }

  Error = false;
  return;
}
