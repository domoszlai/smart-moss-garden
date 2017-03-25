/*
 * This skectch demostrates the usage of the proximity sensor described at
 * http://dlacko.blogspot.com/2017/01/arduino-capacitive-proximity-sensor.html
 * 
 */

// https://www.arduino.cc/en/Tutorial/SecretsOfArduinoPWM
// https://github.com/domoszlai/arduino-frequency-counter
#include "frequency_counter_PCI.h"

#define DEBUG

#define MAX_FREQUENCY_DROP        600
#define MAX_EXPECTED_FREQ_NOISE   30
#define CL_WEIGHT                 0.2
#define BL_WEIGHT                 0.995

// Both a current line anad a baseline frequency maintaned
float current_line = 0;
float baseline = 0;

void setup()
{
pinMode(3, OUTPUT);
pinMode(5, OUTPUT);  
pinMode(6, OUTPUT);
pinMode(9, OUTPUT);
  
#ifdef DEBUG
  Serial.begin(9600);
#endif

  delay(100);
  current_line = baseline = count_frequency(2);
}

void loop()
{
  unsigned long f = count_frequency(2);

  // Current line is calculated by an exponental filter with a small weight -> changes fast
  current_line = current_line * CL_WEIGHT + (1 - CL_WEIGHT) * f;
  // Baseline is calculated by an exponental filter with a big weight -> changes slow
  baseline = baseline * BL_WEIGHT + (1 - BL_WEIGHT) * f;

  // distance is reciprocal, bigger value indicates smaller distance
  // and its range is 0-255 
  int diff = max(0, baseline - current_line);
  if(diff < MAX_EXPECTED_FREQ_NOISE)
  { 
    diff = 0;
  }
  else
  {
    diff -= MAX_EXPECTED_FREQ_NOISE;
  }
  int distance = (diff) * 255L / MAX_FREQUENCY_DROP;
  distance = min(distance, 255);

analogWrite(3, distance);
analogWrite(5, distance);
analogWrite(6, distance);
analogWrite(9, distance);

#ifdef DEBUG
  Serial.print(distance);
  Serial.print(" ");
  Serial.print(f);
  Serial.print(" ");
  Serial.print(baseline);
  Serial.print(" ");
  Serial.println(current_line);   
#endif  
}


