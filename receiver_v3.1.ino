

/*
 * 
 * 
 * v3:
 * - Detects if within range of reception or not...
 * - Configured LEDs
 *
 * v3.1:
 * - Added MODES to it to be able to set the operational characteristics of the device
 *   1) SET PARAMETERS (5)   2) PROTOTYPE MODE (9)    3) OPERATIONAL MODE (0).
 * 
 * 
*/

#include <SoftwareSerial.h>
#define GREEN     4
#define lon       digitalWrite(GREEN, HIGH)
#define loff      digitalWrite(GREEN, LOW)
#define l_blink   lon; delay(300); loff; delay(300);
#define led_set   l_blink; l_blink; l_blink;

SoftwareSerial Fieneryc(8,9);

char f_read = ' ';
char s_read = ' ';

String stringSRead = "";
String stringFRead = "";
String sendCommand = "AT+SEND=3,8,B0\r\n";

int isSRead = 0, isFRead = 0, isLedOn = 0;
uint_8t MODE = 0;

unsigned long sendTime = millis(), 
              serialUpdateTime = millis(), 
              fienerycUpdateTime = millis(), 
              lastReadFieneryc = millis(),
              lastWrite = millis();
              
unsigned int updateTime = 500, ledBlinkTime = 300;


void setup() {
  Serial.begin(9600);
  Fieneryc.begin(9600);

  //Setup comm
  Fieneryc.print("AT+ADDRESS=6\r\n");
  Fieneryc.print("AT+PARAMETER=7,3,4,5\r\n");

  pinMode(GREEN, OUTPUT);
  led_set;
  
}

void readSerial(){
  while(Serial.available()>0){    
    s_read = Serial.read();         
    if (!((s_read <= 0) || (s_read == '\r') || (s_read == '\n'))){
      stringSRead += s_read; 
      isSRead++;
    }    
   }
}

void readFieneryc(){
  while(Fieneryc.available()){
    f_read = Fieneryc.read();
    stringFRead += f_read;
    isFRead++;
  }
}

void loop() {

   if(millis() - serialUpdateTime >= updateTime){
    readSerial();
    serialUpdateTime = millis();
   }

   if(isSRead) {    
    Serial.print("Serial:|(");
    Serial.print(isSRead); Serial.print(" bytes read)|");
    Serial.println(stringSRead);
    stringSRead += "\r\n";
    Fieneryc.print(stringSRead);

    //if (stringSRead.equals("+OK")){
      //
    //}
    isSRead = 0; stringSRead = "";
    
   }

   if(millis() - fienerycUpdateTime >= updateTime){
    
    readFieneryc();
    fienerycUpdateTime = millis();
    
   }

   if (isFRead){
    lon; isLedOn = 1;
    Serial.print("Fieneryc:|(");
    Serial.print(isFRead); Serial.print(" bytes read)|");
    Serial.println(stringFRead);
    isFRead = 0; stringFRead = "";
    lastReadFieneryc = millis();
    
   }

   if(isLedOn){
      if (millis()- lastReadFieneryc >= ledBlinkTime){
         loff;
         isLedOn = 0;
      }
   }

   //range testing...
  
   if (millis() - lastReadFieneryc >= 20000){
    if (millis() - lastWrite >= 1000){
      Serial.println("OOR OOR");
      lastWrite = millis();
    }
   }

}
