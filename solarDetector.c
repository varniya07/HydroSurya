#include <WiFi.h>
#include <ThingSpeak.h> 
#define PUMP_PIN 23      
#define TRIG_PIN 5         
#define ECHO_PIN 18        
#define FLOW_PIN 4         
#define HIGH_LEVEL 40      
#define LOW_LEVEL 10       
#define MIN_SOLAR_VOLTAGE 2.5 
#define DRY_RUN_THRESHOLD 5
const char* ssid = "Wokwi-GUEST";
const char* password = ""; 
unsigned long myChannelNumber = 3073680 ; 
const char* myWriteAPIKey = "8Z563Z4VIUSDMNZL"; 
long duration;
int waterLevel;
volatile int flowPulses = 0; 
float solarVoltage;
WiFiClient client;
void IRAM_ATTR handleFlowInterrupt() {
    flowPulses++;
}
void setup() {
    Serial.begin(115200); 
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW); 
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(FLOW_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(FLOW_PIN), handleFlowInterrupt, FALLING);
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");
    ThingSpeak.begin(client);
}

void loop() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH);
    waterLevel = duration * 0.034 / 2;

    solarVoltage = analogRead(A0) * (3.3 / 4095.0); 

    if (waterLevel <= HIGH_LEVEL && waterLevel > DRY_RUN_THRESHOLD) { 
        
        if (solarVoltage >= MIN_SOLAR_VOLTAGE) {
            digitalWrite(PUMP_PIN, HIGH);
            Serial.println("PUMP ON: Running on solar power.");
        } else {
            // backup
            digitalWrite(PUMP_PIN, HIGH);
            Serial.println("PUMP ON: Switched to backup power.");
        }
    } else { 
        digitalWrite(PUMP_PIN, LOW);
        if (waterLevel <= DRY_RUN_THRESHOLD) {
            Serial.println("PUMP OFF: Water level too low, dry-run protection activated.");
        } else {
            Serial.println("PUMP OFF: Water level is low.");
        }
    }
    Serial.print("Level: ");
    Serial.print(waterLevel);
    Serial.print(" cm | Voltage: ");
    Serial.print(solarVoltage);
    Serial.print(" V | Flow Pulses: ");
    Serial.println(flowPulses);
    ThingSpeak.setField(1, waterLevel);
    ThingSpeak.setField(2, solarVoltage);
    ThingSpeak.setField(3, flowPulses);
    
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if(x == 200){
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }

    delay(20000);
}