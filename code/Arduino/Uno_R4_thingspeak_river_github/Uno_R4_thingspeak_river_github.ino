// ΓΙΑ ARDUINO UNO R4 WiFi

//ΓΙΑ ΤΗ ΣΤΑΘΜΗ ΝΕΡΟΥ
#define POWER_PIN  7
#define SIGNAL_PIN A1

int value = 0; // ΜΕΤΑΒΛΗΤΗ ΓΙΑ ΜΕΤΡΗΣΗ ΣΤΑΘΜΗΣ ΝΕΡΟΥ

//ΦΟΡΤΩΣΗ ΒΙΒΛΙΟΘΗΚΗΣ ΑΙΣΘ. ΘΕΡΜΟΚΡΑΣΙΑΣ-ΥΓΡΑΣΙΑΣ
#include <DHT11.h> 
DHT11 dht11(7);

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

//ΑΡΧΙΚΟΠΟΙΗΣΗ ΟΘΟΝΗΣ LCD
LiquidCrystal_I2C lcd(0x27,16,2); 

//ΑΡΧΙΚΟΠΟΙΗΣΕΙΣ ΓΙΑ ΚΑΤΑΓΡΑΦΗ pH
#define SensorPin A0            
#define Offset 5.4      //Αντιστάθμιση της απόκλισης pH       
#define LED 13
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40   
//ΑΠΟΘΗΚΕΥΣΗ ΤΙΜΩΝ pH
int pHArray[ArrayLenth];   
int pHArrayIndex=0;
#include <WiFiS3.h>

#include "arduino_secrets.h" 
// ΑΝΤΛΗΣΗ ΔΕΔΟΜΕΝΩΝ ΠΡΟΣΒΑΣΗΣ ΣΤΟ WiFi ΑΠΟ ΤΟ ΑΡΧΕΙΟ tab/arduino_secrets.h
char ssid[] = SECRET_SSID;    // TO ONOMA ΔΙΚΤΥΟΥ ΣΑΣ
char pass[] = SECRET_PASS;    // ΚΩΔΙΚΟΣ ΠΡΟΣΒΑΣΗΣ ΤΟΥ ΔΙΚΤΥΟΥ ΣΑΣ
int status = WL_IDLE_STATUS;  // ΤΟ radio status ΤΟΥ WiFi 


// IP του thingspeak.com
char server[] = "184.106.153.149";
// Write API Key του λογαριασμού https://thingspeak.com/channels/2476379 
// στο thingspeak 
char writeKey[] = "U8171KYAAFT5Z6MO";

WiFiClient client;

void setup() {

  Serial.begin(9600);
  lcd.init();   // ΑΡΧΙΚΟΠΟΙΗΣΗ ΟΘΟΝΗΣ lcd 
  lcd.init();
  lcd.backlight();
  
  pinMode(POWER_PIN, OUTPUT);   // ΟΡΙΣΜΟΣ pin D7 pin ΩΣ ΕΞΟΔΟΥ
  digitalWrite(POWER_PIN, LOW); // ΣΒΗΝΕΙ ΤΟΝ ΑΙΣΘΗΤΗΡΑ
  
  pinMode(LED,OUTPUT);
  
  lcd.setCursor(3,0);
  lcd.print(" GYMNASIO");
  lcd.setCursor(0,1);
  lcd.print("MEGALWN KALYVIWN");
  delay(2000);

  

  while (!Serial) {
    ; // ANAMΟΝΗ ΓΙΑ ΣΥΝΔΕΣΗ
  }

  // ΕΛΕΓΧΟΣ WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // ΠΡΟΣΠΑΘΕΙΑ ΣΥΝΔΕΣΗΣ ΣΤΟ ΔΙΚΤΥΟ WiFi
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // ΣΥΝΔΕΣΗ ΣΤΟ ΔΙΚΤΥΟ WPA/WPA2
    status = WiFi.begin(ssid, pass);

    // ΑΝΑΜΟΝΗ 10 ΔΕΥΤΕΡΟΛΕΠΤΑ ΓΙΑ ΣΥΝΔΕΣΗ:
    delay(10000);
  }

  // ΕΧΕΙ ΓΙΝΕΙ Η ΣΥΝΔΕΣΗ ΣΕ ΑΥΤΟ ΤΟ ΣΗΜΕΙΟ
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();

}

void loop() 
{
  // ΕΛΕΓΧΟΣ ΤΗΣ ΣΥΝΔΕΣΗΣ ΣΤΟ ΔΙΚΤΥΟ ΚΑΘΕ 10"
  delay(10000);
  printCurrentNet();

  //  ΜΕΤΡΗΣΕΙΣ ΘΕΡΜΟΚΡΑΣΙΑΣ-ΥΓΡΑΣΙΑΣ
  int temperature = 0;
  int humidity = 0;
  // ΑΝΑΓΝΩΣΗ ΘΕΡΜΟΚΡΑΣΙΑΣ-ΥΓΡΑΣΙΑΣ ΜΕΣΩ ΑΙΣΘΗΤΗΡΑ DHT11
  int result = dht11.readTemperatureHumidity(temperature, humidity);
  Serial.print("Temperature = ");
  Serial.println(temperature);
  Serial.print("Humidity = ");
  Serial.println(humidity);

  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("Temp= "); // You can make spaces using well... spaces
  lcd.print(temperature);
  lcd.print("oC");
  lcd.setCursor(0, 1); 
  lcd.print("Hum= "); // You can make spaces using well... spaces
  lcd.print(humidity);
  lcd.print("%");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("STATHMI:");
  digitalWrite(POWER_PIN, HIGH);  // turn the sensor ON
  delay(10);                      // wait 10 milliseconds
  value = analogRead(SIGNAL_PIN); // read the analog value from sensor
  digitalWrite(POWER_PIN, LOW);   // turn the sensor OFF
  lcd.setCursor(8, 0); 
  if (value <10)
    lcd.print(" 0%  ");  
  else if (value <200)
    lcd.print(" 20% ");
  else if (value <250)
    lcd.print(" 40% ");
  else if (value <280)
    lcd.print(" 60% ");
  else if (value <310)
    lcd.print(" 80% ");
  else
    lcd.print(" 100%");
 //delay(5000); 
  //pH
 static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  if(millis()-samplingTime > samplingInterval)
  {
      pHArray[pHArrayIndex++]=analogRead(SensorPin);
      if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
      voltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
      pHValue = 3.5*voltage+Offset;
      samplingTime=millis();
  }
  if(millis() - printTime > printInterval)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {
    Serial.print("Voltage:");
        Serial.print(voltage,2);
        Serial.print("    pH value: ");
    Serial.println(pHValue,2);
        digitalWrite(LED,digitalRead(LED)^1);
        printTime=millis();
  }
  // lcd.clear();
  lcd.setCursor(0, 1); 
    lcd.print("pH= "); // You can make spaces using well... spaces
  lcd.print(pHValue);
  delay(3000);

  Serial.println("Starting connection to thingspeak...");
 // Αν η σύνδεση ήταν επιτυχής,

 Serial.println("Connected to thingspeak.com");
 // Δημιουργία του αιτήματος HTTP, όπως απαιτεί το
 // thingspeak.com
 String request= "GET /update?key=";
 request += "U8171KYAAFT5Z6MO"; // Γράψε API key
 request += "&field1="; // field1 για pH
 request += String(pHValue);
 request += "&field2="; // field2 για στάθμη
 request += String(value);
 request += "&field3="; // field3 για θερμοκρασία
 request += String(temperature);
  request += "&field4="; // field4 για υγρασία
 request += String(humidity);

 if( client.connect(server, 80) )
 {
 Serial.println(request);
 // Εμφάνιση στο serial monitor για debugging
 // Αποστολή του αιτήματος
 client.println(request);
 // client.println("GET /asciilogo.txt HTTP/1.1");
 client.println("Host: 184.106.153.149");
 delay (5000);}
}

void printWifiData() {
  // ΕΜΦΑΝΙΣΗ ΔΙΕΥΘΥΝΣΗΣ IP
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  
  Serial.println(ip);

  // ΕΜΦΑΝΙΣΗ ΔΙΕΥΘΥΝΣΗΣ MAC
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // ΕΜΦΑΝΙΣΗ ΔΙΕΥΘΥΝΣΗΣ SSID ΣΥΝΔΕΔΕΜΕΝΟΥ ΔΙΚΤΥΟΥ
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // ΕΜΦΑΝΙΣΗ ΔΙΕΥΘΥΝΣΗΣ MAC ΠΟΥ ΕΙΝΑΙ ΣΥΝΔΕΔΕΜΕΝΟ ΤΟ ROUTER
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // ΕΜΦΑΝΙΣΗ ΠΟΙΟΤΗΤΑΣ ΣΗΜΑΤΟΣ
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // ΕΜΦΑΝΙΣΗ ΤΥΠΟΥ ΚΡΥΠΤΟΓΡΑΦΙΣΗΣ
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 0; i < 6; i++) {
    if (i > 0) {
      Serial.print(":");
    }
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
  }
  Serial.println();
}


double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}
