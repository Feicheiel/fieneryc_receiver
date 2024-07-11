

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
#define l_blink   lon; delay(700); loff; delay(300);
#define led_set   l_blink; l_blink; l_blink;

SoftwareSerial Fieneryc(8,9);

char f_read = ' ';
char s_read = ' ';

String stringSRead      = "";
String stringFRead      = "";
String sendCommand      = "";
String THIS_RCVR_ADDR   = "AT+ADDRESS=11317\r\n";
String OTHER_SNDR_ADDR  = "AT+ADDRESS=11731\r\n";
String PARAMETER        = "AT+PARAMETER=7,9,3,13\r\n";
const String sendAddr   = "AT+SEND=11731,20,";

int isSRead = 0, isFRead = 0, isLedOn = 0;
uint8_t MODE = 0;

unsigned long sendTime = millis(), 
              serialUpdateTime = millis(), 
              fienerycUpdateTime = millis(), 
              lastReadFieneryc = millis(),
              lastWrite = millis(), 
              prototypeUpdateTime = millis();
              
const unsigned int updateTime = 500, ledBlinkTime = 300, prototypeTime = 1500; 
unsigned int pctr=100;


void setup() {
  Serial.begin(9600);
  delay(1);
  Fieneryc.begin(9600);
  delay(1);

  //Setup comm
  while(!Fieneryc);
  Fieneryc.print("AT\r\n");
  delay(10);
  Fieneryc.print("ATZ\r\n");
  delay(10);
  Fieneryc.print("AT+OPMODE=1\r\n"); //For RYLR993 you need to set it to 1 to use the RYLR998 syntax, with a default NETWORKID=18
  delay(10);
  Fieneryc.print("AT+BAND=915000000\r\n");
  delay(10);
  Fieneryc.print("AT+NETWORKID=18");
  delay(10);
  Fieneryc.print(PARAMETER);
  delay(10);
  Fieneryc.print(THIS_RCVR_ADDR); // Set the device address for this receiver: 11317.
  delay(10);
  Fieneryc.print("AT+CPIN=F7C3901E\r\n");
  delay(10);

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
    //check the mode that is set.
    if (isSRead < 4) {
      switch (stringSRead.charAt(0)){
        case '0': MODE = 0; break;
        case '5': MODE = 5; break;
        case '9': MODE = 9; break;
      }
    }
    else {
      //set the mode
      if (!MODE) {//Normal Mode
        Serial.print("Serial:|(");
        Serial.print(isSRead); Serial.print(" bytes read)|");
        Serial.println(stringSRead);
        sendCommand = sendAddr + stringSRead + "\r\n";
        Fieneryc.print(sendCommand);
      }
      else if (MODE == 5) { //Set Parameter Mode
        stringSRead += "\r\n";
        Fieneryc.print(stringSRead);      
      }
    }

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
  if (!MODE) { //If it is operating in normal mode then do this else don't.
    if (millis() - lastReadFieneryc >= 20000){
      if (millis() - lastWrite >= 1000){
        Serial.println("<<LoRa SENDER UNAVAILABLE!>>");
        lastWrite = millis();
      }
    }
  }

  //prototype signals sending.
  if (MODE == 9) {
    if (millis() - prototypeUpdateTime >= prototypeTime){
      Serial.print("+RCV=11731,8,");Serial.print(pctr);Serial.println(",u,1,0,1,-99,-40\r\n");
      pctr++;
      pctr %= prototypeTime;
      pctr += 100;
      prototypeUpdateTime = millis();
    }
  } 

}
