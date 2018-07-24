/////////////////////////////
const int MOISTURE_PRAG =10;
const int SOL_USCAT   = 300 + MOISTURE_PRAG;
const int SOL_UMED = 700 + MOISTURE_PRAG;
long previousMillis_moisture = 0;
long interval_moisture = 5000;

/////////////////////////////
const int releuOutput = 9;

/////////////////////////////
byte sensorInterrupt_water = 0;  // 0 = digital pin 2
byte sensorPin_water       = 2;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;


int light_sensor_2 = A3;
int light_reading_2;
long previousMillis_light_2 =0;
long interval_light_2=5000;

int temp_sensor= A2;
int temp_reading;
long previousMillis_temp=0;
long interval_temp =50000;

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CNS, CE
const byte address[6] = "00001";

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  
  lcd.init();
  lcd.backlight();

  pinMode(releuOutput, OUTPUT);
  pinMode(sensorPin_water, INPUT);
  digitalWrite(sensorPin_water, HIGH);

   pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt_water, pulseCounter, FALLING);
  
}

void loop() {
     unsigned long currentMillis = millis();
     if(currentMillis - previousMillis_moisture>interval_moisture){
      previousMillis_moisture=currentMillis;

      int rain_sensor = analogRead(0);
   Serial.println(rain_sensor);
   //radio.write(&rain_sensor, sizeof(rain_sensor));
   lcd.setCursor(0,0);
   
  int moisture = analogRead(1);
  moisture     = map(moisture, 0, 1023, 1023, 0);
  Serial.print("Moisture Sensor: ");
  
  Serial.print(moisture);
  Serial.println();
   //char text[] = "Umiditate:";
   // radio.write(&text, sizeof(text));
  radio.write(&moisture, sizeof(moisture));
 // radio.write(&rain_sensor, sizeof(rain_sensor));

  if(rain_sensor<500){
    Serial.println("Rain");
  //  radio.write(&rain_sensor, sizeof(rain_sensor));
    lcd.setCursor(0,0);
    lcd.print("Ploua!!!!");
    digitalWrite(releuOutput, LOW);
    lcd.setCursor(11,0);
    lcd.print("R:OFF");
    
  }
  else if(moisture <= 100){
    Serial.println("In aer");
    lcd.setCursor(0,0);
    lcd.print("In aer!!!");
    //const char text_aer[] = "In aer!!!";
    //radio.write(&text_aer, sizeof(text_aer));
    digitalWrite(releuOutput, HIGH);
    lcd.setCursor(11,0);
    lcd.print("R: ON");
  }
  else if ((moisture <= SOL_USCAT)&&(moisture>100)) {
    Serial.println("Uscat");
    lcd.setCursor(0,0);
    lcd.print("Uscat:");
    lcd.print(moisture);
  //  radio.write(&moisture, int(moisture));
    digitalWrite(releuOutput, HIGH);
    lcd.setCursor(11,0);
    lcd.print("R: ON");
  } else if (moisture <= SOL_UMED) {
    Serial.println("Umed: ");
    lcd.setCursor(0,0);
    lcd.print("Umed: ");
    lcd.print(moisture);
    digitalWrite(releuOutput, LOW);
    lcd.setCursor(11,0);
    lcd.print("R:OFF");
  } else {
    Serial.println("In Apa");
    lcd.setCursor(0,0);
    lcd.print("F.ud: ");
    lcd.print(moisture);
     digitalWrite(releuOutput, LOW);
     lcd.setCursor(11,0);
    lcd.print("R:OFF");
  }
     }
     if(currentMillis - previousMillis_temp > interval_temp) {
        previousMillis_temp = currentMillis;
        temp_reading=analogRead (temp_sensor);   
        float mv = (temp_reading/1024.0)*5000; 
        float cel = mv/10;
        float farh = (cel*9)/5 + 32;
        Serial.print("TEMPRATURE = ");
        Serial.print((int)(cel));
        Serial.print("*C");
        Serial.println();
        //lcd.setCursor(9,1);
        //lcd.print("Temp:");
        //lcd.print((int)(cel));
        int celsius=((int)(cel));
       radio.write(&celsius, sizeof(celsius));

        
  }
  
 
  if((millis() - oldTime) > 1000){
    detachInterrupt(sensorInterrupt_water);
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;
    
    unsigned int frac;

    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    lcd.setCursor(10,1);
    lcd.print("Deb:");
    lcd.print(int(flowRate));
    Serial.print("\t");       // Print tab space
    //lcd.setCursor(0,1);
    //lcd.print("L/min:");
    //lcd.print(int(flowRate));
   // radio.write(&flowRate,(int)(flowRate));
    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 
    Serial.print("\t");       // Print tab space
    Serial.print(totalMilliLitres/1000);
    Serial.print("Litri:");
    lcd.setCursor(0,1);
    lcd.print("Litri:");
    lcd.print(totalMilliLitres/1000);
    //radio.write(&totalMilliLitres, sizeof(totalMilliLitres));

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt_water, pulseCounter, FALLING);
  }
  // Senzor de Lumina Photoresistor Port Analog 1
  if(currentMillis - previousMillis_light_2 > interval_light_2) {
        previousMillis_light_2 = currentMillis;
        light_reading_2=analogRead (light_sensor_2);   
       // client.print(light_reading);
       Serial.print("* pe portul analogic ");
      Serial.print(3);
      Serial.print(" s-a citit valoarea ");
      Serial.print(light_reading_2);
      Serial.println("<br />");
      radio.write(&light_reading_2, sizeof(light_reading_2));
        if (light_reading_2 < 10) {
    Serial.println(" - Intuneric");
  } else if (light_reading_2 < 200) {
    Serial.println(" - Lumina Slaba");
  } else if (light_reading_2 < 500) {
    Serial.println(" - Lumina normala");
  } else if (light_reading_2 < 800) {
    Serial.println(" - Luminos");
  } else {
    Serial.println(" - Foarte Luminos");
  }
  //delay(2000);
  }
        
}

/*
Interrupt Service Routine
 */
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
