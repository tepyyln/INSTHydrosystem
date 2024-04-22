#include "TFTDisplay.h"

//buffer config:
int buffSize = sizeof(INSFARMData);

//class display TFT:
DisplayTFT dTFT; 

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  int sensorReady = inst.initialSetup();
  if (sensorReady == 1) {
    dTFT.init();
  }
}

void loop() {
  dTFT.renderPage();
}
