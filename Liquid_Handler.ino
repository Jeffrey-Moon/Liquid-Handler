// Import Libraries
#include <Servo.h>

// Declaring Servo
Servo myservo;

// Class of coordinate
class Coordinate {
public:
  int X;
  int Y;
  Coordinate()
  {
    Serial.println("Coordinate initialized");
    X = 0;
    Y = 0;
  }
  Coordinate(int x, int y)
  {
    X = x;
    Y = y;
  }
  void PrintCoor() 
  {
    Serial.println("X Coordinate: ");
    Serial.println(X);
    Serial.println("Y Coordinate: ");
    Serial.println(Y);
  }
};

// Boolean values that acts as indicators
bool runComplete = false; // boolean indicating if the loop completed once

// Boolean two dimensional array of the tip rack to indicate if the tip rack slots are empty or not
bool tipRack[8][12] = {1};

// The designated pin numbers
const int enPin = 8;
const int stepXPin = 2; //X.STEP
const int dirXPin = 5; // X.DIR
const int stepYPin = 3; //Y.STEP
const int dirYPin = 6; // Y.DIR
const int stepZPin = 4; //Z.STEP
const int dirZPin = 7; // Z.DIR
const int servoPin = 11; // Servo

// Stepper motor variables
const int stepsPerRev = 200;
int pulseWidthMicros = 100;  // microseconds
int millisBtwnStepsXY = 615;
int millisBtwnStepsZ = 1000;

// Gaps inbetween the tip rack slots and tube rack slots
const int tipGap_X = 45.56;
const int tipGap_Y = 45.6;
const int tubeGap_X = 46;
const int tubeGap_Y = 46;
const int bigTubeGap_X = 75;
const int bigTubeGap_Y = 75.33;

// Number of tips used
int tipCnt = 0;

// Declaration of coordinates of specific places and the current position
Coordinate *curPos = new Coordinate();
Coordinate *firstTipPos = new Coordinate(194, 45); 
Coordinate *firstTubePos = new Coordinate(195.5, 477);
Coordinate *firstBigTubePos = new Coordinate(845, 89);
Coordinate *tipEjection = new Coordinate(1015, 240);
Coordinate *origin = new Coordinate(0, 0);

// setup function for enabling motors and the CNC shield
void setup() {
  Serial.begin(9600);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);
  Serial.println(F("CNC Shield Enabled"));

  pinMode(stepXPin, OUTPUT);
  pinMode(dirXPin, OUTPUT);
  pinMode(stepYPin, OUTPUT);
  pinMode(dirYPin, OUTPUT);
  pinMode(stepZPin, OUTPUT);
  pinMode(dirZPin, OUTPUT);
  Serial.println(F("Stepper Motor Initialized"));

  myservo.attach(servoPin);
  Serial.println(F("Servo Motor Initialized"));

  Serial.println(F("Initial Position: "));
  curPos->PrintCoor();
  return;
}
// function for pressing the pipette with the servo (servo val: 15)
void pressPipette() {
  myservo.write(73.45);
  delay(1000);
}
// 
void emptyPipette() {
  myservo.write(25);
  delay(1000);
}
// function for releasing the pipette with the servo (servo val: 165)
void releasePipette(bool pressto=0) {
  if (pressto) {
    for (int i = 74.5; i < 166; i++){
      myservo.write(i);
      delay(4);
    }
  }
  else {
    myservo.write(165);
  }
  delay(1000);
}
// function for raising the pipette with the Z Axis stepper motor
void raisePipette(bool from=0) {
  Serial.println(F("Positive Z Movement"));
  digitalWrite(dirZPin, LOW); // Enables the motor to move in a particular direction
  // Makes 1000 pulses for making one full cycle rotation
  int steps;
  if (curPos->X < 840 && curPos->X > 50)
  {
    if (curPos->Y <= 475)
    {
      Serial.println(F("Tip Rack Z Movement"));
      steps = stepsPerRev * 7.34;
    }
    else
    {
      Serial.println(F("Tube Race Z Movement"));
      if (from)
      {
        steps = stepsPerRev * 5.205;
      }
      else
      {
        // steps = stepsPerRev * 5.075;
        steps = stepsPerRev * 4.9;
      }
    }
  }
  else if (curPos->X >= 840)
  {
    if (curPos->X >= 1000)
    {
      Serial.println(F("Tip Ejection Z Movement"));
      steps = stepsPerRev * 7.2;
    }
    else 
    {
      Serial.println(F("Big Tube Z Movement"));
      steps = stepsPerRev * 6.81;
      // 6.9 
    }
  }
  else
  {
    Serial.println(F("ERROR: Coordinate Not Found"));
    steps = stepsPerRev * 1;
  }
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepZPin, HIGH);
    delayMicroseconds(pulseWidthMicros);
    digitalWrite(stepZPin, LOW);
    delayMicroseconds(millisBtwnStepsZ);
  }
  delay(1000); // One second delay
}
// function for lowering the pipette with the Z Axis stepper motor
void lowerPipette(bool from=0) {
  Serial.println(F("Negative Z Movement"));
  digitalWrite(dirZPin, HIGH); // Enables the motor to move in a particular direction
  // Makes 1000 pulses for making one full cycle rotation       
  int steps;
   if (curPos->X < 840 && curPos->X > 50)
  {
    if (curPos->Y <= 475)
    {
      Serial.println(F("Tip Rack Z Movement"));
      steps = stepsPerRev * 7.34;
    }
    else
    {
      Serial.println(F("Tube Race Z Movement"));
      if (from)
      {
        steps = stepsPerRev * 5.205;
      }
      else
      {
        // steps = stepsPerRev * 5.075;
        steps = stepsPerRev * 4.9;
      }
    }
  }
  else if (curPos->X >= 840)
  {
    if (curPos->X >= 1000)
    {
      Serial.println(F("Tip Ejection Z Movement"));
      steps = stepsPerRev * 7.2;
    }
    else 
    {
      Serial.println(F("Big Tube Z Movement"));
      steps = stepsPerRev * 6.813;
      // 6.9
    }
  }
  else
  {
    Serial.println(F("ERROR: Coordinate Not Found"));
    steps = stepsPerRev * 1;
  }

  for (int i = 0; i < steps; i++) {
    digitalWrite(stepZPin, HIGH);
    delayMicroseconds(pulseWidthMicros);
    digitalWrite(stepZPin, LOW);
    delayMicroseconds(millisBtwnStepsZ);
  }
  delay(1000); // One second delay
}
// function for moving the body in the X coordinate by spinning the XY Axis stepper motors in oppostie directions
void moveX(int stepNum) {
  if (stepNum < 0) {
    Serial.println(F("Negative X Movement"));
    digitalWrite(dirXPin, HIGH); // Enables the motor to move in a particular direction
    digitalWrite(dirYPin, HIGH); // Enables the motor to move in a particular direction
    // Makes 200 pulses for making one full cycle rotation
    for (int i = 0; i < -stepNum; i++, curPos->X--) {
      digitalWrite(stepXPin, HIGH);
      delayMicroseconds(pulseWidthMicros);
      digitalWrite(stepXPin, LOW);
      delayMicroseconds(millisBtwnStepsXY);
      digitalWrite(stepYPin, HIGH);
      delayMicroseconds(pulseWidthMicros);
      digitalWrite(stepYPin, LOW);
      delayMicroseconds(millisBtwnStepsXY);
    }
    delay(1000); // One second delay
  }
  else {
    Serial.println(F("Positive X Movement"));
    digitalWrite(dirXPin, LOW); //Changes the rotations direction
    digitalWrite(dirYPin, LOW); //Changes the rotations direction
    // Makes 200 pulses for making two full cycle rotation
    for (int i = 0; i < stepNum; i++, curPos->X++) {
      digitalWrite(stepXPin, HIGH);
      delayMicroseconds(pulseWidthMicros);
      digitalWrite(stepXPin, LOW);
      delayMicroseconds(millisBtwnStepsXY);
      digitalWrite(stepYPin, HIGH);
      delayMicroseconds(pulseWidthMicros);
      digitalWrite(stepYPin, LOW);
      delayMicroseconds(millisBtwnStepsXY);
    }
    delay(1000);
  }
  return;
}
// function for moving the body in the Y coordinate by spinning the XY Axis stepper motors in the same direction
void moveY(int stepNum) {
  if (stepNum < 0) 
  {
    Serial.println(F("Negative Y Movement"));
    digitalWrite(dirXPin, HIGH); // Enables the motor to move in a particular direction
    digitalWrite(dirYPin, LOW); // Enables the motor to move in a particular direction
    // Makes 200 pulses for making one full cycle rotation
    for (int i = 0; i < -stepNum; i++, curPos->Y--) {
      digitalWrite(stepXPin, HIGH);
      delayMicroseconds(pulseWidthMicros);
      digitalWrite(stepXPin, LOW);
      delayMicroseconds(millisBtwnStepsXY);
      digitalWrite(stepYPin, HIGH);
      delayMicroseconds(pulseWidthMicros);
      digitalWrite(stepYPin, LOW);
      delayMicroseconds(millisBtwnStepsXY);
    }
    delay(1000); // One second delay
  }
  else
  {
    Serial.println(F("Positive Y Movement"));
    digitalWrite(dirXPin, LOW); //Changes the rotations direction
    digitalWrite(dirYPin, HIGH); //Changes the rotations direction
    // Makes 200 pulses for making two full cycle rotation
    for (int i = 0; i < stepNum; i++, curPos->Y++) {
      digitalWrite(stepXPin, HIGH);
      delayMicroseconds(pulseWidthMicros);
      digitalWrite(stepXPin, LOW);
      delayMicroseconds(millisBtwnStepsXY);
      digitalWrite(stepYPin, HIGH);
      delayMicroseconds(pulseWidthMicros);
      digitalWrite(stepYPin, LOW);
      delayMicroseconds(millisBtwnStepsXY);
    }
    delay(1000);
  }
  return;
}
// moving to a given coordinate from another coordinate by moving the X and then moving the Y (the coordinate a will usually be curPos)
void goTo(Coordinate *a, Coordinate *b) {
  int X_movement = b->X - a->X;
  int Y_movement = b->Y - a->Y;
  moveX(X_movement);
  moveY(Y_movement);
  return;
}
// Go to the origin and reset the servo motor
void reset() {
  goTo(curPos, origin);
  releasePipette();
  return;
}
void B2TPipette(int X, int Y, int x, int y) {
  goTo(curPos, new Coordinate(firstBigTubePos->X + X * bigTubeGap_X, firstBigTubePos->Y + Y * bigTubeGap_Y));
  pressPipette();
  lowerPipette();
  delay(500);
  releasePipette(1);
  delay(300);
  raisePipette();
  goTo(curPos, new Coordinate(firstTubePos->X + x * tubeGap_X, firstTubePos->Y + y * tubeGap_Y));
  lowerPipette();
  emptyPipette();
  raisePipette();
  releasePipette();
}
// Pipette the liquid from (x1, y1) to (x2, y2)
void T2TPipette(int x1, int y1, int x2, int y2) {
  goTo(curPos, new Coordinate(firstTubePos->X + x1 * tubeGap_X, firstTubePos->Y + y1 * tubeGap_Y));
  pressPipette();
  lowerPipette(1);
  delay(500);
  releasePipette(1);
  delay(300);
  raisePipette(1);
  goTo(curPos, new Coordinate(firstTubePos->X + x2 * tubeGap_X, firstTubePos->Y + y2 * tubeGap_Y));
  lowerPipette();
  emptyPipette();
  raisePipette();
  releasePipette();
  return;
}
// Eject the current tip and attach the next tip on the tip rack
void Tip() {
  if (tipCnt != 0)
  {
    goTo(curPos, new Coordinate(tipEjection->X, 0));
    lowerPipette();
    goTo(curPos, new Coordinate(tipEjection->X, tipEjection->Y));
    raisePipette();
  }
  Coordinate *nextTip = new Coordinate(firstTipPos->X + (tipCnt%12) * tipGap_X, firstTipPos->Y + (tipCnt/12) * tipGap_Y);
  nextTip->PrintCoor();
  goTo(curPos, nextTip);
  lowerPipette();
  raisePipette();
  tipCnt++;
  return;
}
// Test for the first coordiante of the tip rack
void TestTip(int a) {
  Coordinate *nextTip = new Coordinate(firstTipPos->X + (a%12) * tipGap_X, firstTipPos->Y + (a/12) * tipGap_Y);
  goTo(curPos, nextTip);
  lowerPipette();
  raisePipette();
}

void loop() {
  if (!runComplete)
  {
    // ====================== Code Begins Here ======================
    delay(1000);
    releasePipette();
    
    //int array[5] = {5, 1, 0, 2, 0};
    //int array[5] = {1, 0, 0, 0, 1};
    //example array of moves
    int array[5] = {0, 0, 2, 0, 1};
    for (int i = 0; i <= 4; i++)
    {
      if (array[i] != 0){
        Tip();
      }
      for (int j = 0; j < array[i]; j++)
      {
        Serial.println(F("Loop"));
        B2TPipette(0, i, 0, 0);
      }
    }
    
    //TestTip(95);
    // while(true){
    //   B2TPipette(0, 0, 0, 0);
    // }

    reset();
    // ======================= Code Ends Here =======================
    runComplete = true;
  }
}
  