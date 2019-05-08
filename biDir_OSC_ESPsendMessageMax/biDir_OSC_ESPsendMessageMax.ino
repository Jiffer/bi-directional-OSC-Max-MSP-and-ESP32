/*---------------------------------------------------------------------------------------------
  Based on OSC send/receive examples
  bi-directional OSC - w/ Max/MSP (or other)
  by Jiffer Harriman
  --------------------------------------------------------------------------------------------- */
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>

char ssid[] = "things";          // your network SSID (name)
char password[] = "connected";   // your network password

WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
const IPAddress outIp(10, 0, 1, 2);  // remote IP to send messages to
// use to request a static IP:
IPAddress ip(10, 0, 1, 100); // my IP address (requested)
IPAddress gateway(10, 0, 1, 1);
IPAddress subnet(255, 255, 255, 0);
// local and remote port numbers
const unsigned int outPort = 9999;          // remote port to receive OSC
const unsigned int localPort = 8888;        // local port to listen for OSC packets (actually not used for sending)

// gate how fast OSC messages may go out in ms
int sendInterval = 50;
unsigned long timeToSend = 0; // time its ok to send again

bool toggleValue = false;

// led variables
bool ledState = false;

#ifndef BUILTIN_LED
#ifdef LED_BUILTIN
#define BUILTIN_LED LED_BUILTIN
#else
#define BUILTIN_LED 13
#endif
#endif

void setup() {
  Serial.begin(115200);
  pinMode(BUILTIN_LED, OUTPUT);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  int i = 0;
  int state = true;
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 10) {
      state = false;
      break;
    }
    i++;
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
#ifdef ESP32
  Serial.println(localPort);
#else
  Serial.println(Udp.localPort());
#endif
}

void loop() {
  if (millis() > timeToSend) {
    timeToSend = millis() + sendInterval;
    sendOSC();
  }
  // handle incoming messages

  receiveOSC();
} // end of loop()


void sendOSC() {
  // toggle value to write
  toggleValue = !toggleValue;
  //  Serial.print("sending message /digital ");
  //  Serial.println(toggleValue);
  OSCMessage msg("/digital");
  msg.add(toggleValue);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();

  // send another
  //  Serial.print("sending message /analog ");
  OSCMessage msg2("/analog");
  msg2.add(analogRead(A0));
  Udp.beginPacket(outIp, outPort);
  msg2.send(Udp);
  Udp.endPacket();
  msg2.empty();

}

void receiveOSC() {
  OSCBundle bundle;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      bundle.fill(Udp.read());
    }
    if (!bundle.hasError()) {
      bundle.dispatch("/led", led);
    } else {
      OSCErrorCode error = bundle.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
}

// OSC Message handler functions:
void led(OSCMessage &msg) {
  ledState = msg.getInt(0);
  digitalWrite(BUILTIN_LED, ledState); // GPIO 2 is built in LED on DOIT ESP32 board
}
