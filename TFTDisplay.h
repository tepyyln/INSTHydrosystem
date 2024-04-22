#include <UTFT.h>
#include <URTouch.h>
#include "readSensor.h"
#include "structData.h"
#include "TFTUtility.h"

//Class Sensor:
INSFARMSensor inst;

//Struct Declaration:
insfarmData *INSFARMData = new insfarmData;
int len = sizeof(struct insfarmData);
int sprayerStateFromMaster;
int sprayerState;

// Declare which fonts we will be using
extern uint8_t BigFont[];
extern uint8_t SmallFont[];
String message;

// Remember to change the model parameter to suit your display module!
UTFT myGLCD(ILI9341_16, 38, 39, 40, 41);
URTouch  myTouch( 6, 5, 4, 3, 2);

//Prototype:
void writeText(String text, String fontSize, int yCoordinate, String color);
void Button(String text, String buttonType, int xStart, int yStart, int xStop, int yStop);
void formNumber(String inputVal, int xStart, int yStart, int xStop, int yStop);
void HoverEffect(int x1, int y1, int x2, int y2);
void saveValueToEEPROM(float value, int address);
void writeSensorValue(String title, String sensorValue, String fontSize, int yCoordinate, String color);
void send (const insfarmData* table);
void checkMessage();

class DisplayTFT {
  private:
    int x, y, currentPage;
    float _roomTemp, _luxVal, _waterTemp, _phVal, _tdsVal, setPointUpper, setPointBottom;

  public:
    void init();
    void renderPage();
    void setpointConfigPage();
    void realtimeMonitoringPage();
    void RealTimeSensorDataComponent();
};

void DisplayTFT::init() {
  // Setup the LCD TFT
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
  saveValueToEEPROM(35, 0);
  EEPROM.get(0, setPointUpper);
  EEPROM.get(4, setPointBottom);
  Serial.print("setPointUpper =>");
  Serial.println(setPointUpper);
  if (isnan(setPointUpper)) {
    setPointUpper = 0;
  }
  if (isnan(setPointBottom)) {
    setPointBottom = 0;
  }
  send(INSFARMData);
  currentPage = 1;
  realtimeMonitoringPage();
}

void DisplayTFT::renderPage() {
  if (currentPage == 1) {
    checkMessage();
    RealTimeSensorDataComponent();
    if (myTouch.dataAvailable()) {
      myTouch.read();
      x = myTouch.getX();
      y = 240 - myTouch.getY();

      if ((y >= 200) && (y <= 230)) { // Upper row
        if ((x >= 20) && (x <= 170)) { // Button: config
          HoverEffect(20, 200, 170, 230);
          currentPage = 2;
          myGLCD.clrScr();
          setpointConfigPage();
          formNumber(String(setPointUpper), 110, 110, 220, 140);
          formNumber(String(setPointBottom), 110, 150, 220, 180);
        }
      }
    }
  }
  else if (currentPage == 2) {
    checkMessage();
    if (myTouch.dataAvailable()) {
      myTouch.read();
      x = myTouch.getX();
      y = 240 - myTouch.getY();
      //      setpointConfigPage();
      formNumber(String(setPointUpper), 110, 110, 220, 140);
      formNumber(String(setPointBottom), 110, 150, 220, 180);

      if ((y >= 110) && (y <= 140)) { // set Point Upper
        if ((x >= 230) && (x <= 260)) { // Button: add
          HoverEffect(230, 110, 260, 140);
          setPointUpper += 0.50;
          Serial.println(setPointUpper);
          formNumber(String(setPointUpper), 110, 110, 220, 140);
          formNumber(String(setPointBottom), 110, 150, 220, 180);
        }
        if ((x >= 70) && (x <= 100)) { // Button: substract
          HoverEffect(70, 110, 100, 140);
          //substract set point action:
          if (setPointUpper <= 0) {
            setPointUpper = 0;
          } else {
            setPointUpper -= 0.50;
          }
          formNumber(String(setPointUpper), 110, 110, 220, 140);
          formNumber(String(setPointBottom), 110, 150, 220, 180);
          Serial.println(setPointUpper);

        }
      }
      if ((y >= 150) && (y <= 180)) { // setPoint bottom
        if ((x >= 230) && (x <= 260)) { // Button: add
          HoverEffect(230, 150, 260, 180);
          setPointBottom += 0.50;
          Serial.println(setPointBottom);
          formNumber(String(setPointUpper), 110, 110, 220, 140);
          formNumber(String(setPointBottom), 110, 150, 220, 180);
        }
        if ((x >= 70) && (x <= 100)) { // Button: substract
          HoverEffect(70, 150, 100, 180);
          //substract set point action:
          if (setPointBottom <= 0) {
            setPointBottom = 0;
          } else {
            setPointBottom -= 0.50;
          }
          Serial.println(setPointBottom);
          formNumber(String(setPointUpper), 110, 110, 220, 140);
          formNumber(String(setPointBottom), 110, 150, 220, 180);

        }
      }
      if ((y >= 200) && (y <= 230)) { // action button
        if ((x >= 15) && (x <= 95)) { // Button: back
          HoverEffect(15, 200, 95, 230);
          EEPROM.get(0, setPointUpper);
          EEPROM.get(4, setPointBottom);
          currentPage = 1;
          myGLCD.clrScr();
          realtimeMonitoringPage();
        }
        if ((x >= 110) && (x <= 190)) { // Button: save
          HoverEffect(110, 200, 190, 230);
          saveValueToEEPROM(setPointUpper, 0);
          delay(50);
          saveValueToEEPROM(setPointBottom, 4);
          delay(50);
          Serial.print("saved to eeprom! => ");
          EEPROM.get(0, setPointUpper);
          EEPROM.get(4, setPointBottom)v ;
          currentPage = 1;
          myGLCD.clrScr();
          realtimeMonitoringPage();
        }
      }
    }
  }
}

void DisplayTFT::setpointConfigPage() {
  writeText("INSFARM", "big", 20, "secondary");
  writeText("INSTRUMENTATION ENGINEERING ITS", "small", 45, "secondary");
  writeText("SETPOINT CONFIGURATION", "small", 80, "secondary");
  
  //set point atas config:
  myGLCD.setFont(SmallFont);
  myGLCD.print("Upper :", 10, 115);
  myGLCD.print("Bottom:", 10, 155);
  Button("-", "cancel", 70, 110, 100, 140);
  Button("+", "save", 230, 110, 260, 140);
  
  //set point bawah config:
  Button("-", "cancel", 70, 150, 100, 180);
  Button("+", "save", 230, 150, 260, 180);
  
  // action button
  Button("Back", "cancel", 15, 200, 95, 230);
  Button("Save", "save", 110, 200, 190, 230);
}

void DisplayTFT::realtimeMonitoringPage() {
  //build card:
  formNumber("", 10, 10, 310, 190); // value, xstart, ystart, xstop, ystop
  //title:
  writeText("SMART HIDROPONICS", "big", 20, "primary");
  writeText("INSTRUMENTATION ENGINEERING ITS", "small", 45, "primary");
  // action button
  Button("Config->", "config", 20, 200, 170, 230);
}

void sprayerON(int state){
  //sprayer ON
  sprayerState = state;
  inst.sprayerActive();
  myGLCD.setColor(VGA_BLUE);
  myGLCD.fillRect(270, 220 , 300 , 230);
}

void sprayerOFF(){
  //sprayer OFF
  sprayerState = 0;
  inst.sprayerInactive();
  myGLCD.setColor(VGA_RED);
  myGLCD.fillRect(270, 220 , 300 , 230);
}

void DisplayTFT::RealTimeSensorDataComponent(){
  _roomTemp      = inst.getRoomTemp();
  _luxVal        = inst.getLuxVal();
  _waterTemp     = inst.getWaterTemp();
  _tdsVal        = inst.getTDSVal();
  _phVal         = inst.getPHVal();
  
  
  // draw sensor value, note: place in sensorFunction for real time updating data!
  writeSensorValue("Lux", String(_luxVal), "small", 70, "primary");
  writeSensorValue("Suhu Air",  String(_waterTemp), "small", 87.5, "primary");
  writeSensorValue("Suhu Ruang", String(_roomTemp), "small", 105, "primary");
  writeSensorValue("TDS Air", String(_tdsVal), "small", 122.5, "primary");
  writeSensorValue("PH ", String(_phVal), "small", 140, "primary");
  writeSensorValue("SP upper", String(setPointUpper), "small", 157.5, "primary");
  writeSensorValue("SP bottom", String(setPointBottom), "small", 175, "primary");

  bool isSprayerOnState = (_roomTemp >= setPointUpper);
  if (isSprayerOnState == true) {
    //sprayer ON
    sprayerON(1);
    INSFARMData->sprayerState = 1;
  }
  else if ((_roomTemp >= setPointBottom && _roomTemp <= setPointUpper) || (_roomTemp <= setPointBottom))
  {
    if (sprayerStateFromMaster == 1)
    {
      //sprayer ON
      sprayerON(2);
      INSFARMData->sprayerState = 2;
    } else {
      //sprayer OFF
      sprayerOFF();
      INSFARMData->sprayerState = 0;
    }
  }

//  Serial.print("room => ");
//  Serial.println(_roomTemp);
//  Serial.print(" water => ");
//  Serial.println(_waterTemp);
//  Serial.print(" lux => ");
//  Serial.println(_luxVal);
//  Serial.print("pH => ");
//  Serial.println(_phVal);
//  Serial.print("tds => ");
//  Serial.println(_tdskVal);
//  Serial.print(" SP Bottom => ");
//  Serial.println(setPointBottom);
//  Serial.print(" SP Upper => ");
//  Serial.println(setPointUpper);

   

  if(_waterTemp != 0){
    INSFARMData->watertemp = _waterTemp;
  }
  if(_roomTemp != 0){
    INSFARMData->roomtemp = _roomTemp;
  }
  if(_luxVal != 0){
    INSFARMData->lux = _luxVal;
  }
  if(_tdsVal != 0){
    INSFARMData->tds = _tdsVal;
  }
  if(_phVal != 0){
    INSFARMData->ph = _phVal;
  }
  
  INSFARMData->setPointUpper = setPointUpper;
  INSFARMData->setPointBottom = setPointBottom;
  
  send(INSFARMData);
}

// Write a text
void writeText(String text, String fontSize, int yCoordinate, String color) {

  if (color == "primary") {
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.setColor(0, 0, 0);
  }
  else{
    myGLCD.setBackColor(0, 0, 0);
    myGLCD.setColor(255, 255, 255);
  }
  myGLCD.setFont(fontSize == "small" ? SmallFont : BigFont);
  myGLCD.print(text, CENTER, yCoordinate);
}

// draw button
void Button(String text, String buttonType, int xStart, int yStart, int xStop, int yStop) {
  // bg color roundrect
  if (buttonType == "save")
    myGLCD.setColor(0, 200, 0);
  else if (buttonType == "cancel")
    myGLCD.setColor(255, 0, 0);
  else
    myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect(xStart, yStart, xStop, yStop);
  if (buttonType == "save")
    myGLCD.setColor(0, 0, 0);
  else
    myGLCD.setColor(255, 255, 255);
  // bg color text
  if (buttonType == "save")
    myGLCD.setBackColor(0, 200, 0);
  else if (buttonType == "cancel")
    myGLCD.setBackColor(255, 0, 0);
  else
    myGLCD.setBackColor(0, 0, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print(text, xStart + 10, yStart + 5);
}

// draw form
void formNumber(String inputVal, int xStart, int yStart, int xStop, int yStop) {
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect(xStart, yStart, xStop, yStop);
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawRoundRect(xStart, yStart, xStop, yStop);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print(inputVal, xStart + 10, yStart + 5);
}

void checkMessage() {
  //default state:
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print("Wifi:", 200, 195);
  myGLCD.print("Sprayer:", 200, 220);

  if (Serial2.available()) {
    message = Serial2.readStringUntil('\n');
  }
  if (message.length() > 0) {
    Serial.println(message);
    if (message == "100") {
      // wifi on.
      myGLCD.setColor(VGA_LIME);
      myGLCD.fillRect(270, 195 , 300 , 205);
    }
    else if (message == "103") {
      // wifi WAIT FOR CONNECTION.
      myGLCD.setColor(VGA_YELLOW);
      myGLCD.fillRect(270, 195 , 300 , 205);
    }
    else if (message == "104") {
      // wifi off.
      myGLCD.setColor(VGA_RED);
      myGLCD.fillRect(270, 195 , 300 , 205);
    }
    else if (message == "700") {
      // sprayer on by trigger.
      sprayerStateFromMaster = 1;
    }
    else if (message == "704") {
      // sprayer off.
      sprayerStateFromMaster = 0;
    }
  }
}

// Draw a red frame while a button is touched
void HoverEffect(int x1, int y1, int x2, int y2) {
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawRoundRect(x1, y1, x2, y2);
  while (myTouch.dataAvailable())
    myTouch.read();
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect(x1, y1, x2, y2);
}

// sub function of realtime sensor data function to draw a text with satuan
void writeSensorValue(String title, String sensorValue, String fontSize, int yCoordinate, String color) {
  // title
  if (color == "primary") {
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.setColor(0, 0, 0);
  }
  else {
    myGLCD.setBackColor(0, 0, 0);
    myGLCD.setColor(255, 255, 255);
  }
  myGLCD.setFont(fontSize == "small" ? SmallFont : BigFont);
  myGLCD.print(title, 40, yCoordinate);
  myGLCD.print(":", 160, yCoordinate);
  // value
  if (color == "primary") {
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.setColor(0, 0, 0);
  }
  else {
    myGLCD.setBackColor(0, 0, 0);
    myGLCD.setColor(255, 255, 255);
  }
  myGLCD.setFont(fontSize == "small" ? SmallFont : BigFont);

  String unitOfMeasure = "C";
  if (title == "Lux")
    unitOfMeasure = "lx";
  else if (title == "PH ")
    unitOfMeasure = "pH ";
  else if (title == "TDS Air")
    unitOfMeasure = "ppm";

  myGLCD.print(sensorValue + " " + unitOfMeasure, 180, yCoordinate);
}

void saveValueToEEPROM(float value, int address) {
  EEPROM.put(address, value);
}

void send (const insfarmData* table) {
  Serial2.write((const char*)table, len);
  //  Serial.println(table->lux);
}
