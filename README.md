# DHT22_web_D1_mini
Affichage des mesures d'un DHT22 sur un écran oled SSD1306 par l'intermédiaire d'un ESP8266.

Cette version diffère des exemples fournis par ESP et Adafruit car elle ne plante pas sous la charge ni sur la durée.

Besoin de conseils pour limiter la consomation energétique.

# installation
 - arduino ide : https://www.arduino.cc/en/Main/Software
 - Dans la case "Additional Boards Manager URLs", entrez l'adresse suivante : http://arduino.esp8266.com/staging/package_esp8266com_index.json
 - installer esp8266 dans le gestionnaire de cartes
 - installer les lib adafruit : dht, sensor, 1306
 - modifier Adafruit_SSD1306.h pour décomenter la bonne dimension en pixel
 - utiliser le script http://railsrescue.com/blog/2015-05-28-step-by-step-setup-to-send-form-data-to-google-sheets/ pour fair l'api web de la spreadsheet
 
pour les ESP "brut" sans usb voir http://www.fais-le-toi-meme.fr/fr/electronique/tutoriel/programmes-arduino-executes-sur-esp8266-arduino-ide

# info utiles
## les API a avoir en tete
 - https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/WiFiClientSecure.cpp
 - https://github.com/esp8266/Arduino/blob/master/cores/esp8266/Esp.h
 
## bon exemple de code
 - https://github.com/electronicsguy/ESP8266 : fourni HTTPSRedirect 
 - http://embedded-lab.com/blog/post-data-google-sheets-using-esp8266/ : google spreadsheet qui marche
 
# evolutions
 - Google credentials
 - lire un am2320 https://github.com/Gozem/am2320/blob/master/am2320.py
 - voir le code de client.stop()
