//Start init Variable
struct insfarmData {
  char header[4] = {'$', '\0', '\0', '\0'};
  float watertemp;
  float roomtemp;
  float lux;
  float tds;
  float ph;
  float setPointUpper;
  float setPointBottom;
  int   sprayerState;
  char footer[4] = {'\0', '\0', '\0', '\n'};
};
