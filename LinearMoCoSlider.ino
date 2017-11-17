/*
 * This is a sketch to control my linear motion control slider for filming time lapses. It uses the Adafruit DC Motor + Stepper FeatherWing,
 * The OLED FeatherWing and the Feather 32u4 Basic Proto board. The sequence can run from 5 minutes to 8 hours and supports a span of any length.  
 * Get all of the other resources at https://github.com/photoresistor/LinearMoCoSlider.
 * 
 * Written by Tyler Winegarner, 2017
 */


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);



Adafruit_SSD1306 display = Adafruit_SSD1306();

  #define BUTTON_A 9
  #define BUTTON_B 6
  #define BUTTON_C 5
  #define LED      13
  #define ENDSTOP 11

int spanLength = 8625;  // Change this to your span length (in millimeters) * 25
unsigned long oldTime = 0;
unsigned long currentTime;


#if (SSD1306_LCDHEIGHT != 32)
 #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


void setup() {  
  Serial.begin(115200);

  Serial.println("OLED FeatherWing test");
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  Serial.println("OLED begun");

  AFMS.begin();  // create with the default frequency 1.6KHz
  myMotor->setSpeed(20);  // 10 rpm   


  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();
  
  Serial.println("IO test");

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(ENDSTOP, INPUT_PULLUP);

  



}


void loop() {

  //Display that we're starting the homing sequence
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Homing, please wait.");
  display.display(); // actually display all of the above

  // Homing sequence: we're running the motor until the endstop switch is hit.
  int homing = 1;
  while (homing) {
    myMotor->onestep(BACKWARD, DOUBLE);
    if (digitalRead(ENDSTOP) == 0) {
      delay(50);
      Serial.println("Endstop Hit");
      homing = 0;
    }
  }
  
  // once homed, back the motor off by 2mm
  for (int i = 0; i <= 50; i++) {
    myMotor->onestep(FORWARD, DOUBLE);
  }
  delay(250);
  display.clearDisplay();
  display.display();

  
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0,0);
  display.print("Homing Complete!");
  display.display();
  delay(1500);
  
  // Get input from user for the sequence runtime
  int minutes = 5;
  int timeinput = 1;
  unsigned long timer;
  int progress;

  //Display for user input
  display.clearDisplay();
  display.display();
  delay(50);
  display.setCursor(0,0);
  display.println("Span Travel Time");
  display.setCursor(24,8);
  display.print("Minutes");
  display.display();

  
  while (timeinput) {
    //Drawing rectangles is faster than clearing the whole display
    display.setCursor(0,8);
    display.fillRect(0,8,24,8,BLACK);
    display.setTextColor(WHITE, BLACK);
    display.print(minutes);
    display.display();

    //Use the buttons as cursor keys to select the runtime
    if (! digitalRead(BUTTON_A)) {
      if (minutes >= 5 && minutes < 30) minutes += 5;
      else if (minutes >= 30 && minutes < 120) minutes += 15;
      else if (minutes >=120 && minutes < 480) minutes += 30;
    }
    if (! digitalRead(BUTTON_C)) {
      if (minutes > 5 && minutes <= 30) minutes -= 5;
      else if (minutes > 30 && minutes <= 120) minutes -=15;
      else if (minutes > 120 && minutes <= 480) minutes -= 30; 
    }
    
    // Hit the middle button to confirm your runtime
    if (! digitalRead(BUTTON_B)) timeinput = 0;
    delay(100);
  }
  display.clearDisplay();
  display.display();
  delay(100);

  //Clear display, then set it up to start the program

  display.fillRect(0,0,128,32,BLACK);
  display.display();
  delay(100);
  display.setCursor(0,8);
  display.println("Program Starting");
  display.display();
  delay(1500);
  display.fillRect(0,8,128,16,BLACK);
  display.display();

  //Determine the interval time between motor steps
  timer = minutes * 60000;
  float interval = (float)timer / (float)spanLength;
  oldTime = millis();
  int currentPos = 0;

  // Set up display for status while program is running
  display.setCursor(0,0);
  display.println("Program Running");
  display.drawRect(0,10,128,12, WHITE);
  display.setCursor(0,24);
  display.print("Minutes Left:");
  display.display();


  //Loop that runs the motor during the sequence
  while (currentPos < spanLength) {
    currentTime = millis();
    if ((currentTime - oldTime) >= interval) {
      //move the motor
      myMotor->onestep(FORWARD, DOUBLE);
      
      //update the position reference
      currentPos++;

      //update the clock
      oldTime = currentTime;
      timer -= interval;

      //update the progress bar
      progress = map(currentPos, 0, spanLength, 2, 126);
      display.fillRect(2, 12, progress, 8, WHITE);

      //update the onscreen timer
      display.fillRect(80,24,128,32, BLACK);
      display.setCursor(80,24);
      display.print((timer / 60000) +1);
      display.display();

    }


  }
  //we're done! Update display and reset everything.
  delay(2000);
  display.fillRect(0,0,128,32,BLACK);
  display.display();
  display.setCursor(0,0);
  display.println("Program Complete.");
  display.display();
  delay(5000);  
  display.fillRect(0,0,128,32,BLACK);
  display.display();

}


