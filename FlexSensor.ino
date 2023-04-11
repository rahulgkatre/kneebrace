#define ADC_REFERENCE_VOLTAGE 5.0

Adafruit_PCF8591 pcf = Adafruit_PCF8591();

void flexSensorSetup() {
    if (!pcf.begin()) {
        Serial.println("# Adafruit PCF8591 not found!");    
    } else {
        pcf.enableDAC(false);
    }
}

void flexSensorLoop() {
  Serial.println(pcf.analogRead(3));
  delay(50);
}
