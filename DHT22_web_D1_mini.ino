#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>

#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Adafruit_SSD1306.h>
//#include <WiFiClientSecure.h>
//#include <WiFiClient.h>
#include "HTTPSRedirect.h"
#include <ESP8266WebServer.h>

// protocole bonjour d'apple
//#include <ESP8266mDNS.h>
//MDNSResponder mdns;

const char* ssid = "Livebox-B7B0";
const char* password = "...";
const char* nom_capteur [2] = {"portail", "garage"}; //{"portail", "garage"}; //{"chambre", "radiateur"}; //

#define DHT1PIN D3     // pate vers le data du DHT
#define DHT2PIN D5     // pate vers le data du DHT
#define ALIMDHTPIN D6     // alimentation du DHT2
#define DHTTYPE DHT22 // modèle DHT11 ou DHT22
#define ADR_I2C_SSD1306 0x3C // adresse I2C de l'écran OLED
#define position "Chambre" // titre de la page web

DHT dht1(DHT1PIN, DHTTYPE);
DHT dht2(DHT2PIN, DHTTYPE);
ESP8266WebServer server(80);

int cpt = 0;
int capteur = 0;
float temp [2] = {0.0,0.0};
float hum [2] = {0.0,0.0};
long mil = millis();
long intervale_cap = 5000;
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
HTTPSRedirect google(port);

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
  DHT tdht [2] = {dht1,dht2};
  
  for (int i=0;i<2;i++) {
    temp[i] = tdht[i].readTemperature();
    hum[i] = tdht[i].readHumidity();
    float f = tdht[i].readTemperature(true);
    if (isnan(hum[i]) || isnan(temp[i]) || isnan(f)) 
    {
      Serial.printf("echec lecture capteur %d\n", i);
      temp[i] = 0.0;
      hum[i] = 0.0;
      break;
    }
  }
}

void maj_capteurs_temporise()
{
  if (t1-t0_cap > intervale_cap)
  {
    //Serial.println("maj capteur");
    t0_cap += intervale_cap;
    mil = millis();

    maj_capteurs();
  }
}

void add_ligne (String req) {
  Serial.println(req);
  google.printRedir(req, host, googleRedirHost);
  Serial.println("lecture reponse");
}

void maj_tableau_temporise() {
  if (t1-t0_tab > intervale_tab) {
    t0_tab += intervale_tab;
    maj_tableau();
  }
}

void maj_tableau() {
  int cpt = 0;
  while ((!google.connected()) && (cpt < 5)) {
    google.connect(host, port);
    Serial.println("connection failed");
    Serial.printf("tentative %d\n", cpt);
    cpt++;
  }
  if (!google.connected()) { return; } // pas de mesure envoyée sur cette période

  for(int i = 0; i < 2; i++) {
    String req=String(path) + "?" + parametre(nom_capteur[i],temp[i],hum[i]);
    add_ligne(req);
    delay(1000); // pour tester
  }
}

String reset() {
  return affichePage();
}

void reinitialisecapteur() {
  digitalWrite(ALIMDHTPIN, LOW);
  Serial.println("reset");
  delay(500);
  digitalWrite(ALIMDHTPIN, HIGH);
  delay(500);
  maj_capteurs();
  maj_display();
}

String affichePage() {
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
  return reponse;
}

String info(){
  String reponse = String("info : \n<br />")
  + "\n<br />Vcc :" + ESP.getVcc()
  + "\n<br />Free Heap :" + ESP.getFreeHeap()
  + "\n<br />Frequence :" + ESP.getCpuFreqMHz()
  + "\n<br />taille du code :" + ESP.getSketchSize()
  //+ "\n<br />signature du ode :" + ESP.getSketchMD5()
  + "\n<br />cycle : " + ESP.getCycleCount() +"\n";
  return reponse;
}

void connexion() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println();
    // Connect to WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    int t_wifi = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      if (millis() - t_wifi > 20000) {break;}
    }
  }
  Serial.println("WiFi connected");
}

void alu() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup() 
{
  Serial.begin(115200);

  connexion();
  
  // alim du capteur
  pinMode(ALIMDHTPIN, OUTPUT);
  digitalWrite(ALIMDHTPIN,HIGH);

  // capteur et ecran
  dht1.begin(); 
  dht2.begin(); 
  display.begin(SSD1306_SWITCHCAPVCC, ADR_I2C_SSD1306);
  Serial.println("test hum = 0");

  int cptlectureinitiale = 0;
  while (((hum[0] == 0.0) && (hum[1] == 0.0)) && cptlectureinitiale < 10) {
    cptlectureinitiale++;
    Serial.println("hum = 0");
    maj_capteurs();
    maj_display();
    delay(200);
  }

// bonjour
//  if (mdns.begin("esp8266", WiFi.localIP())) {
//    Serial.println("MDNS responder started");
//  }

  // services
  server.on("/", [](){
    testdrawchar();
    server.send(200, "text/html", affichePage());
  });
  server.on("/reset", [](){
    digitalWrite(LED_BUILTIN, LOW); // tension inversée pour un esp8266 donc allume
    reinitialisecapteur();
    server.send(200, "text/html", reset());
    digitalWrite(LED_BUILTIN, HIGH);
  });
  server.onNotFound([](){
    server.send(404, "text/html", "Command Not Found");
  });
  server.on("/restart",[](){
    server.send(200, "text/html", "reboot");
    ESP.restart();
  });
  server.on("/softrset",[](){
    server.send(200, "text/html", "reset");
    ESP.reset();
  });
  server.on("/info",[](){
    server.send(200, "text/html", info());
  });
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("URL : ");
  Serial.print("http://");
  ip = iptoString(WiFi.localIP());
  Serial.print(ip);
  Serial.println("/");

  // première mesure sauvegardée
  maj_tableau();
  
  alu();
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
  maj_capteurs_temporise();
  
  // OLED
  maj_display();

  // upload
  maj_tableau_temporise();

  server.handleClient();
  delay(100);
}
