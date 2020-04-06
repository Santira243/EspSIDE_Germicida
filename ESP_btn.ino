
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
// Datos de la red y contraseña

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Instanciamos un servidor en el puerto 80 (puerto de HTTP)
ESP8266WebServer server(80);

// Datos de los pines de la placa
int pinRELE = 4;
int pinLED = 2;
int incomingByte = 0;
// Variable global para manejar el contenido de la pagina (HTML)
String page = "";

const char* ssid = "LUMI_UV";
const char* password = "lumiuvcovid";  // set to "" for open access point w/o password; or any other pw (min length = 8 characters)
const char* WiFi_hostname = "LUMI";

unsigned long ulReqcount;

IPAddress apIP(192, 168, 10, 1);   // if you want to configure another IP address

void setup(void)
{
  WiFi.hostname(WiFi_hostname);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); // if you want to configure another IP address
  WiFi.softAP(ssid, password);
  WiFi.begin();
  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server.begin(); 
  //Ponemos el código HTML en page (va un sólo chorizo de código)
  //page = "<h1>Al COVID con UV</h1><p><a href=\"ReleOn\"><button>ON</button></a>&nbsp;<a href=\"ReleOff\"><button>OFF</button></a></p>";
  page = "<h1 style=\"text-align: center;\"><span style=\"color: #33cccc;\">LUMI-19</span></h1><p style=\"text-align: center;\"><a href=\"ReleOn\"><button>ON</button></a>&nbsp;<a href=\"ReleOff\"><button>OFF</button></a></p><p style=\"text-align: center;\">Control a distancia&nbsp;</p><p style=\"text-align: center;\">www.lumi-uv.com.ar</p>";
  pinMode(pinRELE, OUTPUT);
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinRELE, LOW);
  digitalWrite(pinLED, HIGH); // Inicializamos el pin que va al relé, apagado
  Serial.begin(115200);
  // Página raíz 
    server.on("/", [](){
    server.send(200, "text/html", page);
  });
  // Página de relé encendido
    server.on("/ReleOn", [](){
    server.send(200, "text/html", page);
    //digitalWrite(pinRELE, HIGH);
    Serial.write('P'); //prender-
  });
  // Página 0de relé apagado
    server.on("/ReleOff", [](){
    server.send(200, "text/html", page);
    //server.send(200, "text/html", page + "<p>Lámpara apagada</p>");
    digitalWrite(pinRELE, LOW);
    Serial.write('A'); // apagar- 
  });
  // Arrancamos el servidor
  server.begin();
}
 
void loop(void)
{
  // Solamente hay que correr el handle para el cliente
  // Do work:
  //DNS
  dnsServer.processNextRequest();
  server.handleClient();
  if (Serial.available() > 0)
  {
    // read the incoming byte:
    incomingByte = Serial.read();
    if(incomingByte == 'O')  // "ON" 
    {
      digitalWrite(pinRELE, HIGH);
      delay(500);
    }
    if(incomingByte == 'S') // "STOP"
    {
      digitalWrite(pinRELE, LOW);
      delay(500);
    }
  }
}
