//Rayz_filter_wheel. ver .1 currently compatable with indi-Xagyl

#include <AccelStepper.h>
#include <EEPROM.h>

//Global declarations
word filterPos[] = { 0, 200, 600, 1000, 1400, 1800}; //play with these to course align each filter - only need to do it once.
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
int jitter = 5;                      //fake jitter changes to please indi
int threshold = 30;  // fake threshold change to please indi
// Hall pin definition
/*  
 *   reedsw. wiring
 *   a1 = common
 *   a3 = input --pullup
 *   hall effect wiring
    A0 = d0 digital(comparator)output
    A1 = vcc
    A2 = gnd
    A3 = a0 analog out
*/

const int SENSOR = A3;       // PIN A3 = Hall effect switch - onboard comparator with predefined value (red)
const int hallPower = A2; //(a1 orig)    // PIN A1 = Hall effect supply - pull high   (blue)
const int hallCom  = A1;  //(a2 orig)   // PIN A2 = Hall effect common - pull low     (black)
const int d0 = A0;           // hall effect analog input - not used 2/18/18 (green)

// Motor definitions
#define STEPS 4                        // 28BYJ-48 steps 4 or 8

// Motor pin definitions
#define motorPin1 22// 9         //6 IN1 on ULn2003
#define motorPin2 21 // 6         //9 IN2 on ULn2003
#define motorPin3 23// 8         //7 IN3 on ULn2003
#define motorPin4 20 // 7         //8 IN4 on ULn2003

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


  // Go clockwise...
  if ( inLine == ")0" ) {
    cmdOK = true;
    posOffset[currPos] = posOffset[currPos] + 10;
    newPos = currPos;
    Locate_Home();
    Locate_Slot_x();
    Serial.print("P");
    Serial.print(currPos);
    Serial.print(" Offset ");
    Serial.println( posOffset[currPos] );
    return;
  }

  // Go counterclockwise...  decrements offset value/ not actually reverse
  if ( inLine == "(0" ) {
    cmdOK = true;
    posOffset[currPos] = posOffset[currPos] - 10;
    newPos = currPos;
    Locate_Home();
    Locate_Slot_x();
    Serial.print("P");
    Serial.print(currPos);
    Serial.print(" Offset ");
    Serial.println( posOffset[currPos] );
  }
  

  // Hall-sensor data..
  if ( inLine == "T1" ) {
    cmdOK = true;
    Serial.print("Sensors ");
    Serial.print(digitalRead(SENSOR));
    Serial.print(" ");
    Serial.println(digitalRead(SENSOR));
  }


  // Increase pulse width by 100uS, Displays “Pulse Width XXXXXuS”
  if ( inLine == "M0" ) {
    cmdOK = true;
    PWMvalue = PWMvalue + 100;
    Serial.print("Pulse Width ");
    Serial.print(PWMvalue);
    Serial.println("uS");
  }


  // Decrease pulse width by 100uS, Displays “Pulse Width XXXXXuS”
  if ( inLine ==  "N0" ) {
    cmdOK = true;
    PWMvalue = PWMvalue - 100;
    Serial.print("Pulse Width ");
    Serial.print(PWMvalue);
    Serial.println("uS");
  }

  // Decrease filter position threshold value, Displays Displays "Threshold XX"
  if ( inLine ==  "{0" ) {
    cmdOK = true;
    if (threshold > 0) {
      threshold = threshold - 10;
    }
    Serial.println(threshold);
  }

  // Increase filter position threshold value, Displays Displays "Threshold XX"
  if ( inLine ==  "}0" ) {
    cmdOK = true;
    if (threshold < 100) {
      threshold = threshold + 10;
    }
    Serial.println(threshold);
  }

  // Decrease jitter window by 1, Displays Displays “Jitter X” value 1-10.
  if ( inLine ==  "[0" ) {
    cmdOK = true;
    if (jitter > 0) {
      jitter--;
    }
    Serial.println(jitter);
  }

  // Increase jitter window by 1, Displays Displays “Jitter X” value 1-10.
  if ( inLine ==  "]0" ) {
    cmdOK = true;
    if (jitter > 10 ) {
      jitter++;
    }
    Serial.println(jitter);
  }

  // Hall-sensor data..
  if ( inLine == "T2" ) {
    cmdOK = true;
    Serial.println("MidRange 520");
  }


  // Hall-sensor data..
  if ( inLine == "T3" ) {
    cmdOK = true;
    Serial.print("RightCal ");
    Serial.println(digitalRead(SENSOR) - digitalRead(SENSOR));
  }

  // Display the Pulse Width value - "Pulse Width XXXXXuS" : PWMvalue 0..255
  if ( inLine == "I9" ) {
    cmdOK = true;
    Serial.print("Pulse Width ");  // Default 1500uS, Range 100 - 10000
    Serial.println("4950uS");      // 10000-100/2=4950 Just a value
  }

  // Display filter position sensor threshold value - "Threshold XX"
  if ( inLine == "I7" ) {
    cmdOK = true;
    Serial.println("Threshold 30");
  }
  // Reset Jitter value to 1, displays "Jitter 1"
  if ( inLine == "R3" ) {
    cmdOK = true;
    Serial.println("Jitter 5");
  }


  // Reset maximum carousel rotation speed to 100%, displays "MaxSpeed 100%"
  if ( inLine == "R4" ) {
    cmdOK = true;
    Serial.println("MaxSpeed 100%");
  }


  // Reset Threshold value to 30, displays "Threshold 30"
  if ( inLine == "R5" ) {
    cmdOK = true;
    Serial.println("Threshold 30");
  }


  // Calibrate, No return value displayed.
  if ( inLine == "R6" ) {
    cmdOK = true;
    delay(1000);
  }

  // Display the jitter value - "Jitter XX", XX = values 1-10
  if ( inLine == "I5" ) {
    cmdOK = true;
    Serial.println("Jitter 5");
  }

  // Store offsets to eprom..
  if ( inLine == "G0" ) {
    cmdOK = true;
    epromSave(); //store offsets to eprom and display contents
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
    Serial.println("FW3.1.5"); //must begin with FW and have only digits after. (RC= "FW" %d)
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
    Serial.println("sn.001");
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

  // Send filter offset value  - "PX Offset XXXX"
  if ( inLine == "O1" ) {
    cmdOK = true;
    Serial.print("P1");
    Serial.print(" Offset ");
    Serial.println( posOffset[1] );
  }

  // Send filter offset value  - "PX Offset XXXX"
  if ( inLine == "O2" ) {
    cmdOK = true;
    Serial.print("P2");
    Serial.print(" Offset ");
    Serial.println( posOffset[2] );
  }

  // Send filter offset value  - "PX Offset XXXX"
  if ( inLine == "O3" ) {
    cmdOK = true;
    Serial.print("P3");
    Serial.print(" Offset ");
    Serial.println( posOffset[3] );
  }


  // Send filter offset value  - "PX Offset XXXX"
  if ( inLine == "O4" ) {
    cmdOK = true;
    Serial.print("P4");
    Serial.print(" Offset ");
    Serial.println( posOffset[4] );
  }


  // Send filter offset value  - "PX Offset XXXX"
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

  // Filter select G1 - G5 , also any "Xnnn" numerical input is handled here
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
    posOffset[currPos] = (inLine.substring(1).toInt());   //get the int value from the serial string
    Locate_Home();
    Locate_Slot_x();
    Serial.print("P");
    Serial.print(currPos);
    Serial.print(" Offset ");
    Serial.println( posOffset[currPos] );
  }
  if (command == 'S' ) {
    maxSpeed = (inLine.substring(1).toInt());   //get the int value from the serial string
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


  // Reset all calibration values to 0. Handy for first run/calibrating,
  // if you then save zeroes to eprom with G0
  if ( inLine == "R2" ) {
    cmdOK = true;
    currPos = 1;
    for ( int i = 1; i < 6 ; i++ ) {
      posOffset[i] = 0;
      if (i == 5) {
        Serial.println("Calibration Removed");
      }
    }
  } //end of inline commands


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
  if ( filterPos[newPos] < filterPos[currPos] ) { //prevent backwards movement to avoid backlash errors
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
void epromSave() {
  int i = 0;
  word Value;

  //EEPROM.write(address, value);
  for ( i = 1; i < 6 ; i++ ) {
    EEPROM.write((i * 2) + 50, highByte(posOffset[i]));
    EEPROM.write((i * 2) + 51, lowByte(posOffset[i]));
  }
  //  Serial.println("Current values written to EEPROM!!");
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
