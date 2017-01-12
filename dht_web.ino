#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>

#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Adafruit_SSD1306.h>
 
const char* ssid = "Livebox-B7B0";
const char* password = "...";
 
#define DHTPIN D3     // pate du vers le data du DHT
#define DHTTYPE DHT11 // modèle DHT11 ou DHT22
#define ADR_I2C_SSD1306 0x3C // adresse I2C de l'écran OLED
#define position "Chambre" // titre de la page web

DHT dht(DHTPIN, DHTTYPE);
WiFiServer server(80);

int cpt;
float temp = 0;
float hum = 0;
long mil = millis();
long intervale_cap = 15000;
long t0_cap = 0;
long t1 = millis();

// OLED
Adafruit_SSD1306 display(LED_BUILTIN); // OLED reset = D4 en fait dans le vide
long intervale_disp = 5000;
long t0_disp = 0;

String ip = "0";

void testdrawchar(void) {
  Serial.println("table de char");
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  for (uint8_t i=0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }    
  display.display();
  delay(1);
}

void maj_display()
{
  if (t1-t0_disp > intervale_disp)
  {
    //Serial.println("maj display");
    t0_disp += intervale_disp;
    display.clearDisplay();
    //delay(10);
    display.setTextColor(WHITE);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("IP : "); display.println(ip);
    display.println();
    display.setTextSize(2);
    display.print("T : "); display.print(temp);display.println("C");
    display.print("h : "); display.print(hum);display.println("%");
    display.setTextSize(1);
    display.println();
    display.print("t : "); display.println(mil);
    display.display();
  }
}

void maj_capteurs()
{
  if (t1-t0_cap > intervale_cap)
  {
    //Serial.println("maj capteur");
    t0_cap += intervale_cap;
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    float f = dht.readTemperature(true);
    mil = millis();
    if (isnan(hum) || isnan(temp) || isnan(f)) 
    {
      temp = 0;
      hum = 0;
      return;
    }
  }
}

void setup() 
{
  Serial.begin(115200);
  cpt=0;
  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
  dht.begin(); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("URL : ");
  Serial.print("http://");
  ip = iptoString(WiFi.localIP());
  Serial.print(ip);
  Serial.println("/");

  display.begin(SSD1306_SWITCHCAPVCC, ADR_I2C_SSD1306);
  maj_capteurs();
  maj_display();
  Serial.println("setup ok");
}

String iptoString (IPAddress ip)
{
  String adr = String("")
  + ip[0]
  + "."
  + ip[1]
  + "."
  + ip[2]
  + "."
  + ip[3];
  return adr;
}
void loop() 
{
  //Serial.println("loop");
  t1 = millis();
  maj_capteurs();
  // OLED
  maj_display();
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  testdrawchar();
 
//  Serial.printf("new client %d.%d.%d.%d:%d <-> %d.%d.%d.%d:%d\n", 
//    client.remoteIP()[0],
//    client.remoteIP()[1],
//    client.remoteIP()[2],
//    client.remoteIP()[3],
//    client.remotePort(),
//    client.localIP()[0],
//    client.localIP()[1],
//    client.localIP()[2],
//    client.localIP()[3],
//    client.localPort());
//  Serial.printf("t = %d\n",mil);

//  Serial.printf("new client %s:%d <-> %s:%d\n", 
//    iptoString(client.remoteIP()),
//    client.remotePort(),
//    iptoString(client.localIP()),
//    client.localPort());

  long t0_client = millis();
  while(!client.available()){
    //Serial.println("client casse");
    if (millis()-t0_client > 2000)
    {
      Serial.println("client timeout");
      //delay(2000);
      return;
    }
    delay(10);
  }

// Jeter le reste de la Requete HTTP à la poubelle
//  char c;
//  while( client.available() ){
//    c = client.read();
//  }
    
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  //cpt++;
  //Serial.printf("it : %d \n", cpt);
  client.flush(); // ? ca fait quoi
 
  // Return the response
  String reponse = String("HTTP/1.1 200 OK\n")
  + "Content-Type: text/html\n"
  + "Connnection: close\n"
  + "\n"
  + "<!DOCTYPE HTML>\n"
  + "<html>\n"
  //+ "<meta http-equiv=\"refresh\" content=\"30\">\n" // rafraichir la page
  + "<meta content=\"text/html; charset=utf-8\">\n"
  + "<title>ESP8266 " + position + "</title>\n"
  + "<br />\n"
  + position + " <br />\n";

 
  reponse += "Temp&eacute;rature (C): ";
  reponse += temp;
  reponse += "\n";
  reponse += "<br />\n";
  reponse += "Humidit&eacute;e (%): ";
  reponse += hum;
  reponse += "\n";
  reponse += "<br />\n";
  reponse += "duree : ";
  reponse += millis();
  reponse += "\n";
  reponse += "<br />\n";
  reponse += "</html>\n";
  client.print(reponse);
  client.flush();
  delay(100);
  //Serial.println("Client disconnected");
}
