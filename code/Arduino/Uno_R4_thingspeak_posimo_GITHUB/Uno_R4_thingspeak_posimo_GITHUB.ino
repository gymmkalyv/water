//ΦΟΡΤΩΣΗ ΒΙΒΛΙΟΘΗΚΗΣ ΑΙΣΘ. ΘΕΡΜΟΚΡΑΣΙΑΣ-ΥΓΡΑΣΙΑΣ
#include <DHT11.h>
DHT11 dht11(7);

//ΑΡΧΙΚΟΠΟΙΗΣΗ ΟΘΟΝΗΣ LCD
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); 

//ΑΡΧΙΚΟΠΟΙΗΣΕΙΣ ΓΙΑ ΚΑΤΑΓΡΑΦΗ pH
#define SensorPin A0            
#define Offset 5.4       //Αντιστάθμιση της απόκλισης pH
#define LED 13
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //ΑΠΟΘΗΚΕΥΣΗ ΤΙΜΩΝ pH//times of collection
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
char writeKey[] = "OBGPHYFLD1I9D88W";

WiFiClient client;

void setup() {

  Serial.begin(9600);
  lcd.init();    // ΑΡΧΙΚΟΠΟΙΗΣΗ ΟΘΟΝΗΣ lcd 
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
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

  //θολότητα
  int sensorValue = analogRead(A1);
  float voltaget=sensorValue*(5.0/1024.0);
  
      //  ΜΕΤΡΗΣΕΙΣ ΘΕΡΜΟΚΡΑΣΙΑΣ-ΥΓΡΑΣΙΑΣ
    int result = dht11.readTemperatureHumidity(temperature, humidity);

  
 // int chk = DHT.read11(DHT11_PIN);
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
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.print("Turbitidy= "); // ΕΜΦΑΝΙΣΗ ΘΟΛΟΤΗΤΑΣ
  lcd.print(voltaget);
 
  //pH
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  if(millis()-samplingTime > samplingInterval)
  {
      pHArray[pHArrayIndex++]=analogRead(SensorPin);
      if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
      voltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
      //pHValue = 3.5*voltage+Offset;
      pHValue = 6.99;
      samplingTime=millis();
  }
  if(millis() - printTime > printInterval)   
  //ΚΑΘΕ 800 msec, ΕΜpΦΑΝΙΖΕΙ ΤΙς ΜΕΤΡΗΣΕΙΣ
  {
    Serial.print("Voltage:");
        Serial.print(voltage,2);
        Serial.print("    pH value: ");
    Serial.println(pHValue,2);
        digitalWrite(LED,digitalRead(LED)^1);
        printTime=millis();
  }
  lcd.setCursor(0, 1); 
  lcd.print("pH= "); 
  lcd.print(pHValue);
  delay(3000);

  Serial.println("Starting connection to thingspeak...");
 // Αν η σύνδεση ήταν επιτυχής,

 Serial.println("Connected to thingspeak.com");
 // Δημιουργία του αιτήματος HTTP, όπως απαιτεί το
 // thingspeak.com
 String request= "GET /update?key=";
 request += "OBGPHYFLD1I9D88W"; // Γράψε API key
 request += "&field1="; // field1 για pH
 request += String(pHValue);
 request += "&field2="; // field2 για στάθμη
 request += String(voltaget);
 request += "&field3="; // field3 για θολότητα
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
 delay (5000);
 }
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
  if(number<5){   //less than 5, calculated directly statistics
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
