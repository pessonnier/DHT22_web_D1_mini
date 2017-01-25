#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>

#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Adafruit_SSD1306.h>
//#include <WiFiClientSecure.h>
//#include <WiFiClient.h>
#include "HTTPSRedirect.h"

const char* ssid = "Livebox-B7B0";
const char* password = "...";
const char* nom_capteur [2] = {"portail", "garage"}; //{"chambre", "fenetre"}; //

#define DHT1PIN D3     // pate vers le data du DHT
#define DHT2PIN D5     // pate vers le data du DHT
#define DHTTYPE DHT22 // modèle DHT11 ou DHT22
#define ADR_I2C_SSD1306 0x3C // adresse I2C de l'écran OLED
#define position "Chambre" // titre de la page web

DHT dht1(DHT1PIN, DHTTYPE);
DHT dht2(DHT2PIN, DHTTYPE);
WiFiServer server(80);

int cpt = 0;
int capteur = 0;
float temp [2] = {0.0,0.0};
float hum [2] = {0.0,0.0};
long mil = millis();
long intervale_cap = 15000;
long t0_cap = 0;
long t1 = millis();

// OLED
Adafruit_SSD1306 display(LED_BUILTIN); // OLED reset = D4 en fait dans le vide
long intervale_disp = 5000;
long t0_disp = 0;

// upload data
long t0_tab = 0;
long intervale_tab = 120000;
const char* path = "/macros/s/.../exec";
const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";
const int port = 443;
HTTPSRedirect client(port);

String ip = "0";

String parametre (String capteur, float temperature, float humidite) {
  return String("capteur=") + capteur + "&temperature=" + temperature + "&humidite=" + humidite; 
}

String urlpost(String param) {
  return String("POST ") + path + " HTTP/1.1\n"
  + "Host: " + host + "\n"
  + "Content-Type: application/x-www-form-urlencoded; charset=UTF-8\n"
  + "Content-Length: " + param.length() + "\n"
  + "Connection: close\n"
  + param + "\n\n\r\n";
}

String urlget(String param) {
  return String("GET ") + path + "?" + param + " HTTP/1.1\n"
  + "Host: " + host + "\n";
}

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
    if (capteur == 1) {
      capteur = 0;
    }
    else {
      capteur = 1;
    }
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
    display.print("T : "); display.print(temp[capteur]);display.println("C");
    display.print("h : "); display.print(hum[capteur]);display.println("%");
    display.setTextSize(1);
    display.println();
    display.print(nom_capteur[capteur]); display.print(" "); display.println(mil);
    display.display();
  }
}

void maj_capteurs()
{
  if (t1-t0_cap > intervale_cap)
  {
    //Serial.println("maj capteur");
    t0_cap += intervale_cap;
    mil = millis();

    temp[0] = dht1.readTemperature();
    hum[0] = dht1.readHumidity();
    float f = dht1.readTemperature(true);
    if (isnan(hum[0]) || isnan(temp[0]) || isnan(f)) 
    {
      temp[0] = 0;
      hum[0] = 0;
      return;
    }

    temp[1] = dht2.readTemperature();
    hum[1] = dht2.readHumidity();
    f = dht2.readTemperature(true);
    if (isnan(hum[1]) || isnan(temp[1]) || isnan(f)) 
    {
      temp[1] = 0;
      hum[1] = 0;
      return;
    }
  }
}

void add_ligne (String req) {
  Serial.println(req);
  client.printRedir(req, host, googleRedirHost); 
  // reponse ignoree
  cpt = 0;
  while (client.connected()) {
    cpt++;
    if (cpt > 1000) {
      Serial.println("1000 lignes");
      break;
    }
    String line = client.readStringUntil('\n');
    if (!line) {
      Serial.println("pas de line");
      break;
    }
    Serial.println(line);
    if (line.length() < 2) {
      break;
    }
  }
  Serial.println("reponse lu");
}

void maj_tableau() {
  if (t1-t0_tab > intervale_tab) {
    int cpt = 0;
    while (!client.connected() && cpt < 5) {
      client.connect(host, port);
      Serial.println("connection failed");
      Serial.printf("tentative %d\n", cpt);
      cpt++;
    }
    if (!client.connected()) { return; }
    
    t0_tab += intervale_tab;
    String req=String(path) + "?" + parametre(nom_capteur[0],temp[0],hum[0]);
    add_ligne(req);
    req=String(path) + "?" + parametre(nom_capteur[1],temp[1],hum[1]);
    add_ligne(req);
  }
}

void setup() 
{
  Serial.begin(115200);
  
  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
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

  // capteur et ecran
  dht1.begin(); 
  dht2.begin(); 
  display.begin(SSD1306_SWITCHCAPVCC, ADR_I2C_SSD1306);
  //while ((temp[0] == 0.0) && (hum[0] == 0.0)) {
    maj_capteurs();
    maj_display();
  //}
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

  // upload
  maj_tableau();
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  testdrawchar();
 
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
  + nom_capteur[0] + " <br />\n"
  + "Temp&eacute;rature (C): " + temp[0] + "\n"
  + "<br />\n"
  + "Humidit&eacute;e (%): " + hum[0] + "\n"
  + "<br />\n"
  + nom_capteur[1] + " <br />\n"
  + "Temp&eacute;rature (C): " + temp[1] + "\n"
  + "<br />\n"
  + "Humidit&eacute;e (%): " + hum[1] + "\n"
  + "<br />\n"
  + "duree : " + millis() + "\n"
  + "<br />\n"
  + "</html>\n";
  client.print(reponse);
  client.flush();
  delay(100);
  //Serial.println("Client disconnected");
}
