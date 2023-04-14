#define ADC_REFERENCE_VOLTAGE 5.0
Adafruit_PCF8591 pcf = Adafruit_PCF8591();
bool mockPCF8591 = true;
int adc_val = 0;

String getFlexJsonString() {
    return "{\"label\":\"Flex Sensor\",\"data\":{\"ADC Value\":" + String(adc_val) + "},\"bounds\":{\"minimum\": 0, \"maximum\": 255}}";
}

float int_to_volts(uint16_t dac_value, uint8_t bits, float logic_level) {
  return (((float)dac_value / ((1 << bits) - 1)) * logic_level);
}

void flexSensorSetup() {
    if (!pcf.begin()) {
        Serial.println("Adafruit PCF8591 not found!");    
    } else {
        mockPCF8591 = false;
        pcf.enableDAC(false);
    }
}

void flexSensorLoop() {
  if (mockPCF8591) {
    adc_val = 5 * sq(sq(sin(millis())));
  } else {
    adc_val = pcf.analogRead(3);
  }
}
