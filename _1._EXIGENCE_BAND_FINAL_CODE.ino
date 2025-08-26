//                            PIN CONNECTIONS
//=======================================================================
//                            OLED (SCL = D1) (SDA = D2)
//                            MPU  (SCL = D1) (SDA = D2)
//                            GSM  (Tx =  D7) (Rx =  D8)
//                            GPS  (Tx =  D4) (Rx =  D3)
//                            MAX  (SCL = D1) (SDA = D2) (INIT = D0)
//=======================================================================

//========================CODE BEGINS=========================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include "MAX30100_PulseOximeter.h"


//Tx Pin of GSM connected to D7
//Rx pin of GSM connected to D8
SoftwareSerial mySerial(D7, D8);


TinyGPSPlus gps;  // The TinyGPS++ object

//TX>>>D3
//RX>>>D4
//object(Tx,Rx)
SoftwareSerial ss(D3, D4); // The serial connection to the GPS device


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int MPU_addr=0x68;  // I2C address of the MPU-6050

// Select SDA and SCL pins for I2C communication 
const uint8_t scl = D1;
const uint8_t sda = D2;

int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float ax=0, ay=0, az=0, gx=0, gy=0, gz=0, tmp=0;
boolean fall = false; //stores if a fall has occurred
boolean trigger1=false; //stores if first trigger (lower threshold) has occurred
boolean trigger2=false; //stores if second trigger (upper threshold) has occurred
boolean trigger3=false; //stores if third trigger (orientation change) has occurred
byte trigger1count=0; //stores the counts past since trigger 1 was set true
byte trigger2count=0; //stores the counts past since trigger 2 was set true
byte trigger3count=0; //stores the counts past since trigger 3 was set true
int angleChange=0;

float latitude , longitude;
int year , month , date, hour , minute , second;
String date_str , time_str , lat_str , lng_str;
int pm;

String link;


//Spo2 and BPM 
#define REPORTING_PERIOD_MS 1000
PulseOximeter pox;
float BPM, SpO2;
uint32_t tsLastReport = 0;


//Heart
const unsigned char bitmap [] PROGMEM=
{
0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x18, 0x00, 0x0f, 0xe0, 0x7f, 0x00, 0x3f, 0xf9, 0xff, 0xc0,
0x7f, 0xf9, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xe0, 0x7f, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xf7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0x7f, 0xdb, 0xff, 0xe0,
0x7f, 0x9b, 0xff, 0xe0, 0x00, 0x3b, 0xc0, 0x00, 0x3f, 0xf9, 0x9f, 0xc0, 0x3f, 0xfd, 0xbf, 0xc0,
0x1f, 0xfd, 0xbf, 0x80, 0x0f, 0xfd, 0x7f, 0x00, 0x07, 0xfe, 0x7e, 0x00, 0x03, 0xfe, 0xfc, 0x00,
0x01, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x3f, 0xc0, 0x00,
0x00, 0x0f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//if stop becomes in not equal to 1, the program stops
int stop = 1;

//Counter to resend the emergency message after 10 minutes of any emergency
int counter = 0;

//=======================================================================
//                    Power on setup
//=======================================================================
void setup() 
{
  //GPS PART STARTS
  ss.begin(9600);
   
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  delay(500);
  display.clearDisplay();
  display.setTextSize(2.8);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  
  // TEXT 1
  display.println(" GROUP-16");
  display.display(); 
  display.setCursor(0, 30);
  display.println("  PROJECT ");
  display.display(); 

  delay(1500);
  display.clearDisplay();
  display.setTextSize(2.8);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // TEXT 2
  display.println("  WELCOME");
  display.display(); 
  display.setCursor(0, 30);
  display.println("   USER");
  display.display(); 

  delay(1000);
  display.clearDisplay();
  display.setTextSize(2.8);
  display.setTextColor(WHITE);
  display.setCursor(0, 30);
  // TEXT 3
  display.println(" CONTACTS");
  display.display(); 

  delay(1000);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // TEXT 4
  display.println("1)Contact 1");
  display.display(); 
  display.setCursor(0, 35);
  display.println("2)Contact 2");
  display.display(); 


  delay(3000);
  display.clearDisplay();
  display.setTextSize(2.8);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // TEXT 2
  display.println("  STATS");
  display.display(); 
  display.setCursor(0, 35);
  display.println("  NORMAL");
  display.display();


 Wire.begin(sda, scl);
 Wire.beginTransmission(MPU_addr);
 Wire.write(0x6B);  // PWR_MGMT_1 register
 Wire.write(0);     // set to zero (wakes up the MPU-6050)
 Wire.endTransmission(true);


 //Oximeter
if (!pox.begin())
    {
         Serial.println("MAX30100 FAILED");
         for(;;);
    }
    else
    {
         Serial.println("MAX30100 Connected SUCCESSFULLY");
         pox.setOnBeatDetectedCallback(onBeatDetected);
    }
 
    // The default current for the IR LED is 50mA and it could be changed by uncommenting the following line.
     //pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

 pinMode(D6,INPUT);

}

 

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() 
{
 pox.update();
 
 //Emergency Button
 int button = digitalRead(D6);


 //Check for SPo2 and BPM
 if (millis() - tsLastReport > REPORTING_PERIOD_MS)
    {
        Serial.print("Heart rate: ");
        Serial.print(BPM);
        Serial.print("\nSpO2: ");
        Serial.print(SpO2);
        Serial.print("%");
 
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(1);
        display.setCursor(0,16);
        display.println(pox.getHeartRate());
 
        display.setTextSize(1);
        display.setTextColor(1);
        display.setCursor(0, 0);
        display.println("Heart BPM");
 
        display.setTextSize(1);
        display.setTextColor(1);
        display.setCursor(0, 30);
        display.println("Spo2");
 
        display.setTextSize(1);
        display.setTextColor(1);
        display.setCursor(0,45);
        display.println(pox.getSpO2());
        display.display();

        display.drawBitmap( 60, 20, bitmap, 28, 28, 1);
        display.display();
 
        tsLastReport = millis();
    }
    

    while (ss.available() > 0)
    if (gps.encode(ss.read()))
    {
      if (gps.location.isValid())
      {
        latitude = gps.location.lat();
        lat_str = String(latitude , 6);
        longitude = gps.location.lng();
        lng_str = String(longitude , 6);
        link = link + lat_str + lng_str;
      }

      if (gps.date.isValid())
      {
        date_str = "";
        date = gps.date.day();
        date = date + 1;
        month = gps.date.month();
        year = gps.date.year();

        if (date < 10)
          date_str = '0';
        date_str += String(date);

        date_str += " / ";

        if (month < 10)
          date_str += '0';
        date_str += String(month);

        date_str += " / ";

        if (year < 10)
          date_str += '0';
        date_str += String(year);
      }

      if (gps.time.isValid())
      {
        time_str = "";
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();

        minute = (minute + 30);
        if (minute > 59)
        {
          minute = minute - 60;
          hour = hour + 1;
        }
        hour = (hour + 5) ;
        if (hour > 23)
          hour = hour - 24;

        if (hour >= 12)
          pm = 1;
        else
          pm = 0;

        hour = hour % 12;

        if (hour < 10)
          time_str = '0';
        time_str += String(hour);

        time_str += " : ";

        if (minute < 10)
          time_str += '0';
        time_str += String(minute);

        time_str += " : ";

        if (second < 10)
          time_str += '0';
        time_str += String(second);

        if (pm == 1)
          time_str += " PM ";
        else
          time_str += " AM ";

      }
    }

    if (gps.location.isValid())
  {
    link = "http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=";
    link += lat_str;
    link += "+";
    link += lng_str;

  Serial.println("\nThe Latitude is: "+lat_str);
  Serial.println("The Longitude is: "+lng_str);
  Serial.println("\nThe Link  is: "+link);
  Serial.println("\nThe Time is: "+time_str);
  Serial.println("The Date is: "+date_str);
  }

 mpu_read();
 ax = (AcX-2050)/16384.00;
 ay = (AcY-77)/16384.00;
 az = (AcZ-1947)/16384.00;
 gx = (GyX+270)/131.07;
 gy = (GyY-351)/131.07;
 gz = (GyZ+136)/131.07; 
 tmp = ((Tmp)/340+36.53); 
 
 
 // calculating Amplitute vactor for 3 axis
 float Raw_Amp = pow(pow(ax,2)+pow(ay,2)+pow(az,2),0.5);
 int Amp = Raw_Amp * 10;  // Mulitiplied by 10 bcz values are between 0 to 1
 
 Serial.print("The Accelerometer Amplitude Vector reading is: ");
 Serial.println(Amp);
 
 if (Amp<=2 && trigger2==false)
 { //if AM breaks lower threshold (0.4g)
   trigger1=true;
   Serial.println("TRIGGER 1 ACTIVATED");
   }
   
 if (trigger1==true)
 {
   trigger1count++;
   
   if (Amp>=12)
   { //if AM breaks upper threshold (3g)
     trigger2=true;
     Serial.println("TRIGGER 2 ACTIVATED");
     trigger1=false; trigger1count=0;
     }
     
 }
 
 if (trigger2==true)
 {
   trigger2count++;
   angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5); Serial.println(angleChange);
   
   if (angleChange>=30 && angleChange<=400)
   { //if orientation changes by between 80-100 degrees
     trigger3=true; trigger2=false; trigger2count=0;
     Serial.println(angleChange);
     Serial.println("TRIGGER 3 ACTIVATED");
    }
   }
   
 if (trigger3==true)
 {
    trigger3count++;
    if (trigger3count>=10)
    { 
       angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
       //delay(10);
       Serial.println(angleChange); 
       if ((angleChange>=0) && (angleChange<=10))
       { 
        //if orientation changes remains between 0-10 degrees
           fall=true; trigger3=false; trigger3count=0;
           Serial.println(angleChange);
             }
       else
       { 
        //user regained normal orientation
          trigger3=false; trigger3count=0;
          Serial.println("TRIGGER 3 DEACTIVATED");
       }
     }
  }
  
 if (fall==true)
 { 
   //in event of a fall detection
   Serial.println("FALL DETECTED");

  //Emergency Message
  display.clearDisplay();
  display.setTextSize(2.8);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  
  display.println("   FALL");
  display.display(); 
  display.setCursor(0, 35);
  display.println(" DETECTED! ");
  display.display();

  delay(2000);
  //Emergency Message2
  display.clearDisplay();
  display.setTextSize(2.8);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  
  display.println("EMERGENCY");
  display.display(); 
  display.setCursor(0, 35);
  display.println("MSG SENT");
  display.display();

  //Sending the emergency message
  emergency();
  
  stop = 0;
   }

  //Button Press on Emergency
  if(button==1)
  {
   Serial.println("The HELP Button is pressed. Sending Emergency Message!");

  //Emergency Message
  display.clearDisplay();
  display.setTextSize(2.8);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  
  display.println("HelpButton");
  display.display(); 
  display.setCursor(0, 35);
  display.println(" Pressed! ");
  display.display();

  delay(2000);
  //Emergency Message2
  display.clearDisplay();
  display.setTextSize(2.8);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  
  display.println("EMERGENCY");
  display.display(); 
  display.setCursor(0, 35);
  display.println("MSG SENT");
  display.display();

  //Sending the emergency message
  emergency();

  stop = 0;
  }

  //Emergency because of Low SpO2 and Abnormal Heart Beats
  if(SpO2 < 85 || BPM < 55 || BPM > 100)
  {
   Serial.println("Abnormal SPO2 or BPM Detected. Sending Emergency Message!");

  //Emergency Message
  display.clearDisplay();
  display.setTextSize(2.8);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  
  display.println("Abnormal");
  display.display(); 
  display.setCursor(0, 35);
  display.println(" STATS! ");
  display.display();

  delay(2000);
  //Emergency Message2
  display.clearDisplay();
  display.setTextSize(2.8);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  
  display.println("EMERGENCY");
  display.display(); 
  display.setCursor(0, 35);
  display.println("MSG SENT");
  display.display();

  //Sending the emergency message
  emergency();

  stop = 0;
  }

  if (trigger2count>=6)
 { //allow 0.5s for orientation change
   trigger2=false; trigger2count=0;
   Serial.println("TRIGGER 2 DECACTIVATED");
   }
 if (trigger1count>=6)
 { //allow 0.5s for AM to break upper threshold
   trigger1=false; trigger1count=0;
   Serial.println("TRIGGER 1 DECACTIVATED");
   }
   
  delay(100);

  //Resend the emergency message after 10 minutes.
  if(counter>0)
  {
    emergency();

    counter = 0;
  }
  
  //After any Emergency Situation, the restarts after 10 Minutes
  if(stop==0)
  {
    //10 Minutes
    delay(600000);
    counter++;
  }

   }

   
void mpu_read()
{
 Wire.beginTransmission(MPU_addr);
 Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
 Wire.endTransmission(false);
 Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
 AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
 AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
 AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
 Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
 GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
 GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
 GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
 }


 void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}


void emergency()
{
  
  //GSM PART STARTS
  mySerial.begin(9600);
  delay(8000);

  //PART 1 : Calling the Help Number
  mySerial.println("AT");
  updateSerial();
  //CONTACT 1
  mySerial.println("ATD+ +91**********;");
  updateSerial();
  delay(15000);
  mySerial.println("ATH"); //hang up
  updateSerial();
  delay(10000);
  mySerial.println("AT");
  updateSerial();
  //CONTACT 2
  mySerial.println("ATD+ +91**********;");
  updateSerial();
  delay(15000);
  mySerial.println("ATH"); //hang up
  updateSerial();
  delay(10000);

  
  //PART 2 : Sending Message through GSM\
  //CONTACT 1
  mySerial.println("AT");
  delay(500);
  mySerial.println("AT+CMGF=1");
  delay(500);
  mySerial.println("AT+CMGS=\"+91**********\"\r");
  delay(500);
  mySerial.print("Help, Emergency situation..!!");
  delay(500);
  mySerial.println(" ");
  delay(500);
  mySerial.println("Location of Emergency: " + link);
  delay(500);
  mySerial.println(" ");
  delay(500);
  mySerial.println("Time of Emergency: " + time_str);
  delay(500);
  mySerial.println("Date of Emergency: " + date_str);
  delay(500);
  mySerial.println(" ");
  delay(500);
  mySerial.print("Recorded Heart Beats(BPM): ");
  delay(500);
  mySerial.println(BPM);
  delay(500);
  mySerial.print("Recorded Blood Oxygen Levels(SpO2%): ");
  delay(500);
  mySerial.println(SpO2);
  delay(500);
  mySerial.println("Recorded Body Temperature('C): ");
  delay(500);
  mySerial.println(tmp);
  delay(500);
  mySerial.write(26);
  updateSerial();
  delay(10000);


  //CONTACT
  mySerial.println("AT");
  delay(500);
  mySerial.println("AT+CMGF=1");
  delay(500);
  mySerial.println("AT+CMGS=\"+91**********\"\r");
  delay(500);
  mySerial.println("Help, Emergency situation..!!");
  delay(500);
  mySerial.println(" ");
  delay(500);
  mySerial.println("Location of Emergency: " + link);
  delay(500);
  mySerial.println(" ");
  delay(500);
  mySerial.println("Time of Emergency: " + time_str);
  delay(500);
  mySerial.println("Date of Emergency: " + date_str);
  delay(500);
  mySerial.println(" ");
  delay(500);
  mySerial.print("Recorded Heart Beats(BPM): ");
  delay(500);
  mySerial.println(BPM);
  delay(500);
  mySerial.print("Recorded Blood Oxygen Levels(SpO2 %): ");
  delay(500);
  mySerial.println(SpO2);
  delay(500);
  mySerial.println("Recorded Body Temperature('C): ");
  delay(500);
  mySerial.println(tmp);
  delay(500);
  mySerial.write(26);
  updateSerial();
  delay(10000);

}

void onBeatDetected()
{
    Serial.println("Beat Detected!");
    display.drawBitmap( 60, 20, bitmap, 28, 28, 1);
    display.display();
}

//========================CODE ENDS========================
