//Start init Library
#include "DHT.h"
#include <Wire.h>
#include <BH1750.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//Start init PinOut Instrument
//DHT22
#define DHTTYPE DHT22
#define DHTPIN 19
DHT dht(DHTPIN, DHTTYPE);

//DS18B20
int DS18Pin = 18;
OneWire oneWire(DS18Pin);
DallasTemperature watertemp (&oneWire);

//DF-ROBTOT TDS
#define TDSPin              A0
#define VREF                5.0
#define SCOUNT              30
int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0;

//DF-ROBOT PH
#define PHPin               A1            
#define Offset              -0.08 
#define samplingInterval    20
#define printInterval       800
#define ArrayLenth          40 
int pHArray[ArrayLenth];
int pHArrayIndex = 0;

//BH1750
BH1750 luxMeter;

//Relay
#define Sprayer   15
#define Pump      14
#define Nutrition A2 
#define Water     A3

class INSFARMSensor {
  private:
    //Instrument State and Value
    bool  _pumpState;
    bool  _sprayerState;
    float _waterTemp;
    float _roomTemp;
    float _roomHumd;
    float _luxVal;
    float _phVal;
    float _tdsVal;
    
  public:
    //Instrument Function
    int   initialSetup();
    float getRoomTemp();
    float getRoomHumd();
    float getWaterTemp();
    float getLuxVal();
    float getTDSVal();
    float getPHVal();
    void  initTDS();
    void  getWaterNutrition();
    void  sprayerActive();
    void  sprayerInactive();
};

//Constructor
int INSFARMSensor::initialSetup() {
  pinMode (Sprayer, OUTPUT);
  pinMode (Pump, OUTPUT);
  pinMode (Nutrition, OUTPUT);
  pinMode (Water, OUTPUT);
  pinMode (TDSPin, INPUT);
  Serial.begin(115200);
  Wire.begin();
  dht.begin();
  luxMeter.begin();
  watertemp.begin();
  return 1;
}

float INSFARMSensor::getRoomTemp() {
  _roomTemp = dht.readTemperature();
  Serial.print("Temp: ");
  Serial.print(_roomTemp);
  Serial.println(" °C");
  return _roomTemp;
}

float INSFARMSensor::getRoomHumd() {
  _roomHumd = dht.readHumidity(); 
  Serial.print("Humd: ");
  Serial.print(_roomHumd);
  Serial.println(" %");
  return _roomHumd;
}

float INSFARMSensor::getWaterTemp() {
  watertemp.requestTemperatures();
  _waterTemp = watertemp.getTempCByIndex(0);
  Serial.print("Water Temp: ");
  Serial.print(_waterTemp);
  Serial.println(" °C");
  return _waterTemp;
}

float INSFARMSensor::getLuxVal() {
  float _luxVal = luxMeter.readLightLevel();
  Serial.print("Lux: ");
  Serial.print(_luxVal);
  Serial.println(" lux");
  return _luxVal;
}

int getMedianNum(int bArray[], int iFilterLen) {
   int bTab[iFilterLen];
   for (byte i = 0; i<iFilterLen; i++)
     bTab[i] = bArray[i];
   int i, j, bTemp;
   for (j = 0; j < iFilterLen - 1; j++) {
     for (i = 0; i < iFilterLen - j - 1; i++) {
       if (bTab[i] > bTab[i + 1]) {
         bTemp = bTab[i];
         bTab[i] = bTab[i + 1];
         bTab[i + 1] = bTemp;
       }
     }
   }
   if ((iFilterLen & 1) > 0)
     bTemp = bTab[(iFilterLen - 1) / 2];
   else
     bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
   return bTemp;
}

float INSFARMSensor::getTDSVal() {
  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U) {    //every 40 milliseconds,read the analog value from the ADC
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TDSPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
        analogBufferIndex = 0;
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U) {
      printTimepoint = millis();
      for(copyIndex=0; copyIndex<SCOUNT; copyIndex++)
        analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient = 1.0 + 0.02 * (_waterTemp-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge = averageVoltage / compensationCoefficient;  //temperature compensation
      float _tdsVal = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge)* 0.5; //convert voltage value to tds value
      Serial.print("TDS Value: ");
      Serial.print(_tdsVal);
      Serial.println(" ppm");
      return _tdsVal;
   }
   return _tdsVal;
}

double avergearray(int* arr, int number) {
  int i;
  int max, min;
  double avg;
  long amount=0;
  if (number<=0) {
    Serial.println("Error!/n");
    return 0;
  }
  if (number<5) {   //less than 5, calculated directly statistics
    for(i=0; i<number; i++) {
      amount += arr[i];
    }
    avg = amount/ number;
    return avg;
  } else {
    if(arr[0] < arr[1]){
      min = arr[0]; max = arr[1];
    }
    else {
      min = arr[1]; max = arr[0];
    }
    for(i=2; i<number; i++) {
      if(arr[i] < min){
        amount += min;        //arr<min
        min = arr[i];
      } else {
        if(arr[i] > max) {
          amount += max;    //arr>max
          max = arr[i];
     
          amount += arr[i]; //min<=arr<=max
        }
      }
    }
    avg = (double)amount/(number-2);
  }
  return avg;
}

float INSFARMSensor::getPHVal() {
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float Voltage;
  if (millis()-samplingTime > samplingInterval) {
      pHArray[pHArrayIndex++] = analogRead(PHPin);
      if (pHArrayIndex==ArrayLenth)pHArrayIndex = 0;
      float Voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
      float _phVal = 3.5 * Voltage + Offset;
      samplingTime = millis(); 
      return _phVal;
  }
  if(millis() - printTime > printInterval) {  //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
    Serial.print("Voltage: ");
    Serial.print(Voltage);
    Serial.print("    pH value: ");
    Serial.println(_phVal);
    printTime = millis();
  }
  return _phVal;
}

void INSFARMSensor::getWaterNutrition() {
  if ( _tdsVal >= 600 ) {
    digitalWrite(Nutrition, HIGH);
    digitalWrite(Water, LOW);
    delay(2000);
    digitalWrite(Nutrition, LOW);
  }
  else {
    digitalWrite(Nutrition, LOW);
    digitalWrite(Water, HIGH);
    delay(2000);
    digitalWrite(Water, LOW);
  }
  delay(3000);
}

void INSFARMSensor::sprayerActive() {
  digitalWrite(Sprayer, HIGH);
}

void INSFARMSensor::sprayerInactive() {
  digitalWrite(Sprayer, LOW);
}
