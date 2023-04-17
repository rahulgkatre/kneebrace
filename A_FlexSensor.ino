#include <Adafruit_PCF8591.h>
// Make sure that this is set to the value in volts of VCC
#define ADC_REFERENCE_VOLTAGE 5.0
Adafruit_PCF8591 pcf = Adafruit_PCF8591();

#define WINDOW_SIZE_FLEX 100

class window_filter {
private:
  int num_readings = 0;
  float sum = 0;
  float avg = 0;
  float prev_avg = 0;
  float prev_vals[WINDOW_SIZE_FLEX];
  float prev_diffs[WINDOW_SIZE_FLEX];
//  float mean = 0;
  float var = 0; 
public:
  window_filter() {
    for (int i = 0; i < WINDOW_SIZE_FLEX; i++) {
      prev_vals[i] = 0;
      prev_diffs[WINDOW_SIZE_FLEX] = 0;
    }
  }
  // Applies moving average and modifies existing reading's values.
  void update(float reading) {

    // For std
    float prev_reading = 0;
    float prev_diff = 0;
    float new_diff = (reading - prev_avg) * (reading - avg);

    // Calculate rolling sum
    if (num_readings < WINDOW_SIZE_FLEX) {
      prev_vals[num_readings] = reading;
      prev_diffs[num_readings] = new_diff;
    } else {
      prev_reading = prev_vals[num_readings % WINDOW_SIZE_FLEX];
      prev_diff = prev_diffs[num_readings % WINDOW_SIZE_FLEX];
      sum -= prev_reading;
      var -= prev_diff / (WINDOW_SIZE_FLEX - 1);
      prev_vals[num_readings % WINDOW_SIZE_FLEX] = reading;
      prev_diffs[num_readings % WINDOW_SIZE_FLEX] = new_diff;
    }
    sum += reading;
    num_readings += 1;

    // Find averages and variances
    prev_avg = avg;
    avg = sum / std::min(WINDOW_SIZE_FLEX, num_readings);
    var += new_diff / (WINDOW_SIZE_FLEX - 1);
    
    return;
  }
  float get_avg() {
    return avg;
  }
  float get_var() {
    return var;
  }
};

window_filter flex_filter;
unsigned long last_reading_flex = 0;

String getFlexJsonString() {
  return "{\"label\":\"Knee Flexion\",\"data\":{\"Flex Voltage\":" + String(get_flex_reading()) + ",\"Flex Variance\":" + String(flex_filter.get_var()) + "}}";
}

void flexSensorSetup() {
  if (!pcf.begin()) {
    Serial.println("# Adafruit PCF8591 not found!");
    while (1)
      delay(10);
  }
  pcf.enableDAC(true);
}

void flexSensorLoop() {
  int curr = millis();
  if (curr - last_reading_flex >= (REPORT_RATE_US / 1000) - UPDATE_RATE_CORRECTION) {
    flex_filter.update(get_flex_reading());
    last_reading_flex = curr;
  }
}

float get_flex_reading() {
  return int_to_volts(pcf.analogRead(3), 8, ADC_REFERENCE_VOLTAGE);
}

float int_to_volts(uint16_t dac_value, uint8_t bits, float logic_level) {
  return (((float)dac_value / ((1 << bits) - 1)) * logic_level);
}
