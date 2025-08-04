// Configure pins for devices
#define LED_PIN     3
#define PIR_PIN     6
#define BUTTON_PIN  2    
#define BUTTON2_PIN 9    
#define SOIL1_PIN   A0
#define SOIL2_PIN   A1

// Global variables
volatile bool buttonPressed = false;
volatile bool soil2ReadFlag = false;

bool ledState = false;
unsigned long lastDebounceTime = 0;
unsigned long lastSoil1Read = 0;
unsigned long lastSoil2InterruptTime = 0;

const unsigned long interval = 5000;
const unsigned long debounceInterval = 200;

bool lastMotionState = LOW;

// Interrupt Service (ISR) : button 1
void handleButtonInterrupt() 
{
  buttonPressed = true;
}

// Interrupt Service (ISR) : button 2
void handleButton2Interrupt() 
{
  unsigned long now = millis();
  if (now - lastSoil2InterruptTime > debounceInterval) 
  {
    soil2ReadFlag = true;
    lastSoil2InterruptTime = now;
  }
}

void setup() 
{
  Serial.begin(115200);
  while (!Serial); 

  //Pin modes for sensors, actuators
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(SOIL1_PIN, INPUT);
  pinMode(SOIL2_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

// Two interrupts due to SAMD21 doesnt support PCI
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), handleButton2Interrupt, FALLING);

}

void loop() 
{
  unsigned long now = millis();

  // Button 1 toggles LED
  if (buttonPressed) 
  {
    buttonPressed = false;
    bool currentState = digitalRead(BUTTON_PIN);

    if (now - lastDebounceTime > debounceInterval && currentState == LOW) 
    {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      Serial.print("LED toggled: ");
      Serial.println(ledState ? "ON" : "OFF");
      lastDebounceTime = now;
    }
  }

  // Timer for one soil moisture readings
  if (now - lastSoil1Read >= interval) 
  {
    lastSoil1Read = now;

    int soil1 = analogRead(SOIL1_PIN);
    Serial.print("[Timer] Time (s): ");
    Serial.print(now / 1000);
    Serial.print(" — Soil1: ");
    Serial.println(soil1);
  }

  // Interrupt based soil moisture readings
  if (soil2ReadFlag) 
  {
    soil2ReadFlag = false;

    int soil2 = analogRead(SOIL2_PIN);
    Serial.print("[Interrupt] Soil2 Read: ");
    Serial.println(soil2);
  }

  // PIR Motion Check
  static bool lastPirState = LOW;
  int motion = digitalRead(PIR_PIN);

  if (motion == HIGH && lastPirState == LOW) 
  {
    Serial.println("Motion: Detected");
    lastPirState = HIGH;
  } else if (motion == LOW && lastPirState == HIGH) {
    lastPirState = LOW;
  }
}
