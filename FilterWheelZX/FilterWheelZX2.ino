//insert titles here

#include <AccelStepper.h>
#include <EEPROM.h>

//Global declarations
word filterPos[] = { 0, 430, 850, 1250, 1665, 2080}; //play with these to align each filter - only need to do it once.
word posOffset[] = { 0, 90, 72, 72, 72, 72 }; //todo - make this add/subtract from offset and impliment for online tuning in indi
bool Error = false;                      // Error flag
String inLine;                           // Current command.
char command;                            // Command
byte currPos = 1;                        // Start up with 1
int newPos = 1;                          // Just start somewhere
bool cmdOK = false;                      // Command ok ?
int PWMvalue = 128;                      // Set PWM to half
int CalibrationOffset = 0;               //for indi info

#define CPU_REBOOT (_reboot_Teensyduino_());

// Hall pin definition
/*  current wiring
    A0 = d0 digital(comparator)output
    A1 = vcc
    A2 = gnd
    A3 = a0 analog out
*/
const int SENSOR = A0;                  // PIN A3 = Hall effect switch - onboard comparator with predefined value
const int hallPower = A1;                    // PIN A1 = Hall effect supply - pull high
const int hallCom  = A2;                     // PIN A2 = Hall effect common - pull low
const int d0 = A3;// hall effect analog input - not used 2/18/18

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

  pinMode(hallCom, OUTPUT);       //hall gnd
  pinMode(hallPower, OUTPUT);       //hall power
  pinMode(SENSOR, INPUT_PULLUP); //(A3) hall signal
  pinMode(6, OUTPUT);        //motor +a
  pinMode(7, OUTPUT);        //motor -a
  pinMode(8, OUTPUT);        //motor +b
  pinMode(9, OUTPUT);        //motor -b

  digitalWrite(hallCom, LOW);     // set hall gnd to 0v
  digitalWrite(hallPower, HIGH);    //set hall power to 5v

  Serial.begin(9600);
  //while ( !Serial ) {
  // Wait until Serial is available.
  //}
  Serial.flush();

  // Set stepper stuff
  stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(400);
  stepper.setSpeed(400);
  stepper.setAcceleration(1000);     // For runspeed() or run() ??

  stupidInit();                      // Init and find slot 1.


} //end of setup

void loop() {

  // Get incoming command.
  Serial.setTimeout(250);
  inLine = Serial.readStringUntil('\n');


  // Run debugProcedure..
  if ( inLine == "G0" ) {
    cmdOK = true;
    debugProcedure ();  // Go do some debugging..
    return;
  }

  ////////////////////////////////////////////////////////////////////////////
  if ( inLine == ")" ) {
    cmdOK = true;
    inLine = ")0";
  }


  if ( inLine == "(" ) {
    cmdOK = true;
    inLine = "(0";
  }
  ////////////////////////////////////////////////////////////////////////////

  // Take care of commands G1 - G5
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


  // Hard reboot
  if ( inLine == "R0" ) {
    cmdOK = true;
   
CPU_REBOOT; //this resets the arduino and usb line. --or not.
   
      Locate_Home();  // currPos will be 1.
      Serial.print("P");
      Serial.println(currPos);
      delay(100);
  }


  // Initialize, restarts and moves to filter position 1. -- Mine always does this anyway so it is self correcting.
  if ( inLine == "R1" ) {
    cmdOK = true;
    currPos = 1;
    Locate_Home();  // currPos will be 1.
    delay(500);
    Serial.print("P");
    Serial.println(currPos);
  }


  // Do not! Reset all calibration values to 0.
  if ( inLine == "R2" ) {
    cmdOK = true;
    currPos = 1;
    Locate_Home();  // currPos will be 1.
    delay(500);

    //Calibration removal not possible because we are using a fixed preset table for now.
    /*filterPos[0] = 0;
      filterPos[1] = 0;
      filterPos[2] = 0;
      filterPos[3] = 0;
      filterPos[4] = 0;
      filterPos[5] = 0;
      delay(1000);
    */
  }


  // Reset Jitter value to 1, displays "Jitter 1"
  if ( inLine == "R3" ) {
    cmdOK = true;
    Serial.println("Jitter 1");
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


  // Calibrate. This just re-homes my one magnet(home) design.
  if ( inLine == "R6" ) {
    currPos = 1;
    Locate_Home();
    cmdOK = true;
    delay(1000);
  }


  // Product name
  if ( inLine == "I0" ) {
    cmdOK = true;
    Serial.println("Xagyl FW5125V1"); //I think Indi uses this as a handshake - do not change.
  }


  // Firmware version
  if ( inLine == "I1" ) {
    cmdOK = true;
    Serial.println("3.1.5"); // Indi may use this. do not change
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
    Serial.println("ZX2ray"); //don't make this too long. Serial buffer is limited length in Indi.
  }

  // Display the maximum rotation speed - "MaxSpeed XXX%"
  if ( inLine == "I4" ) {
    cmdOK = true;
    Serial.println("MaxSpeed 100%");
  }


  // Display the jitter value - "Jitter XX", XX = values 1-10
  if ( inLine == "I5" ) {
    cmdOK = true;
    Serial.println("Jitter 5");
  }

  /////////////////////////////////////////////////////////////////////////////
  // Display sensor position offset for current filter position - "PX Offset XX"
  if ( inLine == "I6" ) {
    cmdOK = true;
    Serial.print("P");
    Serial.print(currPos);
    Serial.print(" Offset ");
    Serial.println( posOffset[currPos] );
  }
  //////////////////////////////////////////////////////////////////////////////

  // Display filter position sensor threshold value - "Threshold XX"
  if ( inLine == "I7" ) {
    cmdOK = true;
    Serial.println("Threshold 30");
  }


  // Display the number of available filter slots - "FilterSlots X"
  if ( inLine == "I8" ) {
    cmdOK = true;
    Serial.println("FilterSlots 5");
  }


  // Display the Pulse Width value - "Pulse Width XXXXXuS" : PWMvalue 0..255
  if ( inLine == "I9" ) {
    cmdOK = true;
    Serial.print("Pulse Width ");  // Default 1500uS, Range 100 - 10000
    Serial.println("4950uS");      // 10000-100/2=4950 Just a value
  }


 
  if ( inLine == "()") {
    cmdOK = true;
 //insert offsets here. may require reformat of entire serial section for a longer buffer to include numbers. 
  
  }
   // Display sensor position offset for filter position Value - "PX Offset XX"
  if ( inLine == "O1" ) {
    cmdOK = true;
    Serial.print("P1");
    //Serial.print(currPos);
    Serial.print(" Offset ");
    Serial.println( posOffset[1] );
  }

  // Display sensor position offset for filter position Value - "PX Offset XX"
  if ( inLine == "O2" ) {
    cmdOK = true;
    Serial.print("P2");
    //Serial.print(currPos);
    Serial.print(" Offset ");
    Serial.println( posOffset[2] );
  }

  // Display sensor position offset for filter position Value - "PX Offset XX"
  if ( inLine == "O3" ) {
    cmdOK = true;
    Serial.print("P3");
    //Serial.print(currPos);
    Serial.print(" Offset ");
    Serial.println( posOffset[3] );
  }


  // Display sensor position offset for filter position Value - "PX Offset XX"
  if ( inLine == "O4" ) {
    cmdOK = true;
    Serial.print("P4");
    //Serial.print(currPos);
    Serial.print(" Offset ");
    Serial.println( posOffset[4] );
  }


  // Display sensor position offset for filter position Value - "PX Offset XX"
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


  // Hall-sensor data..
  if ( inLine == "T1" ) {
    cmdOK = true;
    Serial.print("Sensor ");
    Serial.print(digitalRead(SENSOR));
    Serial.print(" ");
    Serial.println(digitalRead(SENSOR));
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
    delay(100);
    return;
  }

  ////////////////////////////////////////////////////////
  // Go clockwise...
  if ( inLine == ")0" ) {
    cmdOK = true;
    //    delay(50);
    posOffset[currPos] = posOffset[currPos] + 1;
    Locate_Slot_x();
    Serial.print("P");
    Serial.print(currPos);
    Serial.print(" Offset ");
    Serial.println( posOffset[currPos] );
    delay(100);
    return;
  }
  // Go counterclockwise...
  if ( inLine == "(0" ) {
    cmdOK = true;
    posOffset[currPos] = -posOffset[currPos] - 1;
    Locate_Slot_x();

    Serial.print("P");
    Serial.print(currPos);
    Serial.print(" Offset ");
    Serial.println( posOffset[currPos] );
    delay(100);
    return;
  }
  ////////////////////////////////////////////////////////

  // Increase pulse width by 100uS, Displays “Pulse Width XXXXXuS”
  if ( inLine == "M0" ) {
    cmdOK = true;
    PWMvalue = PWMvalue + 100;
    Serial.print("Pulse Width ");
    Serial.print(PWMvalue);
    Serial.println("uS");
    delay(100);
    return;
  }


  // Decrease pulse width by 100uS, Displays “Pulse Width XXXXXuS”
  if ( inLine ==  "N0" ) {
    cmdOK = true;
    PWMvalue = PWMvalue - 100;
    Serial.print("Pulse Width ");
    Serial.print(PWMvalue);
    Serial.println("uS");
    delay(100);
    return;
  }


  // Decrease filter position threshold value, Displays Displays "Threshold XX"
  if ( inLine ==  "{0" ) {
    cmdOK = true;
    Serial.println("Threshold 30");
    delay(100);
    return;
  }


  // Increase filter position threshold value, Displays Displays "Threshold XX"
  if ( inLine ==  "}0" ) {
    cmdOK = true;
    Serial.println("Threshold 30");
    delay(100);
    return;
  }


  // Decrease jitter window by 1, Displays Displays “Jitter X” value 1-10.
  if ( inLine ==  "[0" ) {
    cmdOK = true;
    Serial.println("Jitter 5");
    delay(100);
    return;
  }


  // Increase jitter window by 1, Displays Displays “Jitter X” value 1-10.
  if ( inLine ==  "]0" ) {
    cmdOK = true;
    Serial.println("Jitter 5");
    delay(100);
    return;
  }


  // If command not recognized, flush buffer and wait for next command..
  if ( cmdOK ) {
    Serial.flush();
    cmdOK = false;
    delay(100);
    return;
  }
}   ////////// End of main loop.. //////////


// I think this sets up the filter slot and offset array?
void stupidInit() {
  int cnt;
  word apa;
  for ( cnt = 1; cnt != 5; cnt = cnt + 1 ) {
    apa = posOffset[cnt];
    apa = filterPos[cnt];
  }
  currPos = 1;
  Locate_Home();  // Rotate to index 0 and then move to slot 1.
}

//            *********Find Home********
void Locate_Home() {
  int HallValue = digitalRead(SENSOR);    // read the hall sensor value
  if (HallValue == HIGH) {
    stepper.runToNewPosition((filterPos[currPos]) - 300);  //move off magnet before homing
    delay(200); //avoid plugging motor - allow wheel to stop.
  }
  HallValue = digitalRead(SENSOR);    // recheck the hall sensor value
  stepper.moveTo(10000);
  if (stepper.distanceToGo() <= 0 ) {
    Serial.println("Error:ET Can't find home");
  }
  while (HallValue == LOW)  {                     //while hall is LOW just run motor and read sensor
    stepper.run();
    HallValue = digitalRead(SENSOR);
  }                                               //when hall goes low set position and move to first offset
  stepper.stop();
  stepper.setCurrentPosition(0);
  currPos = 0;
  Locate_Slot_x();
}

// Move to slots 2..5 ***************************************************
// Always run wheel the same direction to avoid backlash issues.

void Locate_Slot_x() {
  if ( filterPos[newPos] < filterPos[currPos] ) {
    Locate_Home();
  }
  stepper.runToNewPosition(filterPos[newPos]);          // Select filterPos[newPos] from array
  currPos = newPos;             // Tell caller this is the new requested position
  return;
}



// Show some values and reset error flag.
void debugProcedure() {
  int i = 0;
  word Value;

  //EEPROM.write(address, value);
  for ( i = 1; i < 5 ; i++ ) {
    EEPROM.write((i * 2) + 50, highByte(filterPos[i]));
    EEPROM.write((i * 2) + 51, lowByte(filterPos[i]));
  }
  Serial.println("Default values written to EEPROM!!");

  Serial.println("\n");
  Serial.println("-------------------------------");

  Serial.print("Current Offset : ");
  Serial.println( posOffset[currPos] );
  Serial.println();

  Serial.println("Values per memorycell");
  for ( i = 1; i < 5 ; i++ ) {
    Value = word( EEPROM.read(i * 2 + 50), EEPROM.read(i * 2 + 51));
    Serial.print("Pos : ");
    Serial.print( i );
    Serial.print(" : ");
    Serial.println( Value );
  }

  Serial.println("");
  Serial.print("Bytes in EEPROM : ");
  Serial.println(EEPROM.length());
  Serial.println("");
  delay(100);
  return;

  if ( !Error ) {
    Serial.println("Normal operation resumed");
  }

  Error = false;
  return;
}
