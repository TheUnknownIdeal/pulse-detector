int yellowLED = 5; // The LED pin

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(yellowLED, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH); 
  digitalWrite(yellowLED, LOW);   
  delay(1000);                      
  digitalWrite(LED_BUILTIN, LOW);   
  digitalWrite(yellowLED, HIGH); 
  delay(1000);                      
}