// MINIMAL SERIAL TEST - Upload this to test basic communication
// This isolates serial issues from main firmware

void setup() {
  Serial.begin(9600);
  delay(2000);
  
  Serial.println("====================================");
  Serial.println("ESP8266 SERIAL TEST - MINIMAL");
  Serial.println("====================================");
  Serial.println("If you see this clearly, serial works!");
  Serial.println("Free Heap: " + String(ESP.getFreeHeap()));
  Serial.println("Chip ID: " + String(ESP.getChipId()));
  Serial.println("====================================");
}

void loop() {
  Serial.println("Serial test heartbeat - " + String(millis()) + "ms");
  delay(2000);
}