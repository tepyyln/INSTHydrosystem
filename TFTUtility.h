//=====Start Set Value Temperatur=====//

float addValue(float inputValue) {
  return inputValue += 0.1;
};
float substractValue(float inputValue) {
  if(inputValue <= 0) {
    return inputValue = 0;
  } 
  else {
    return inputValue -= 0.50;
  }
};
