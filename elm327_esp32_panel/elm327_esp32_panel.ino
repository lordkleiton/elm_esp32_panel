//run sudo chmod a+rw /dev/ttyUSB to upload code
//hold boot button to upload code

#include "ELMduino.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define ELM_PORT Serial1

#define OLED_RESET     -1 // -1 because no reset signal on oled
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

ELM327 myELM327;
typedef enum { ENG_RPM, SPEED, TEMP } obd_pid_states;
obd_pid_states obd_state = ENG_RPM;

const bool DEBUG        = true;
const int  TIMEOUT      = 100;
const bool HALT_ON_FAIL = false;

float rpm = 0;
float mph = 0;
float temp = 0;

void setupOLED() 
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.display();

  display.clearDisplay();
}

void setupELM()
{
  ELM_PORT.begin(38400);

  Serial.println("Attempting to connect to ELM327...");

  if (!myELM327.begin(ELM_PORT, DEBUG, TIMEOUT))
  {
    Serial.println("Couldn't connect to OBD scanner");

    if (HALT_ON_FAIL)
      while (1);
  }

  Serial.println("Connected to ELM327");
}

void setup()
{
  Serial.begin(115200);
  
  setupOLED();

  setupELM();
}

void handleState(float value, char* title, obd_pid_states nextState)
{
  if (myELM327.nb_rx_state == ELM_SUCCESS)
  {
    // Serial.print(title);
    // Serial.print(":");
    // Serial.println(value);
    obd_state = nextState;
  }
  else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
  {
    // myELM327.printError();
    obd_state = nextState;
  }
}

void drawScreen()
{
  rpm++;
  mph+=2;
  temp+=3;
  display.clearDisplay();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("RPM:"));
  display.println(rpm);

  display.print(F("MPH:"));
  display.println(mph);

  display.print(F("TEMP:"));
  display.println(temp);

  display.display();
}

void loop()
{
  drawScreen();

  switch (obd_state)
  {
    case ENG_RPM:
    {
      rpm = myELM327.rpm();

      handleState(rpm, "rpm", SPEED);
      
      break;
    }
    case SPEED:
    {
      mph = myELM327.mph();

      handleState(mph, "mph", TEMP);
      
      break;
    }
    case TEMP:
    {
      temp = myELM327.engineCoolantTemp();

      handleState(temp, "temp", ENG_RPM);

      break;
    }
  }
}