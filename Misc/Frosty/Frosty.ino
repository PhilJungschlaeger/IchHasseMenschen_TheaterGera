#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

// WiFi credentials
char ssid[] = "NeuMond";         
char pass[] = "affenschaukelarmlehne";

// UDP and OSC setup
WiFiUDP Udp;
const unsigned int localPort = 8888;   // UDP port to listen for OSC

// Solenoid pins
const int SOLENOID_PIN_1 = 16;
const int SOLENOID_PIN_2 = 17;
const int SOLENOID_PIN_3 = 18;

void setup() {
  Serial.begin(115200);

  // Setup solenoid pins as outputs and initialize LOW (off)
  pinMode(SOLENOID_PIN_1, OUTPUT);
  pinMode(SOLENOID_PIN_2, OUTPUT);
  pinMode(SOLENOID_PIN_3, OUTPUT);
  digitalWrite(SOLENOID_PIN_1, LOW);
  digitalWrite(SOLENOID_PIN_2, LOW);
  digitalWrite(SOLENOID_PIN_3, LOW);

  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start UDP listening
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Listening on UDP port: ");
  Serial.println(localPort);
}

// Control solenoid helper
void controlSolenoid(OSCMessage &msg, int pin, int solenoidIndex) {
  if (msg.isFloat(0)) {
    float val = msg.getFloat(0);
    Serial.printf("Solenoid %d received value: %.2f\n", solenoidIndex, val);

    if (val >= 0.5) {
      digitalWrite(pin, HIGH);
      Serial.printf("Solenoid %d ON\n", solenoidIndex);
    } else {
      digitalWrite(pin, LOW);
      Serial.printf("Solenoid %d OFF\n", solenoidIndex);
    }
  } else if (msg.isInt(0)) {
    int val = msg.getInt(0);
    Serial.printf("Solenoid %d received int value: %d\n", solenoidIndex, val);

    if (val != 0) {
      digitalWrite(pin, HIGH);
      Serial.printf("Solenoid %d ON\n", solenoidIndex);
    } else {
      digitalWrite(pin, LOW);
      Serial.printf("Solenoid %d OFF\n", solenoidIndex);
    }
  } else {
    Serial.printf("Solenoid %d: Unsupported data type\n", solenoidIndex);
  }
}

// OSC handlers for each solenoid
void solenoid1(OSCMessage &msg) {
  controlSolenoid(msg, SOLENOID_PIN_1, 1);
}

void solenoid2(OSCMessage &msg) {
  controlSolenoid(msg, SOLENOID_PIN_2, 2);
}

void solenoid3(OSCMessage &msg) {
  controlSolenoid(msg, SOLENOID_PIN_3, 3);
}

void loop() {
  OSCBundle bundle;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      bundle.fill(Udp.read());
    }

    if (!bundle.hasError()) {
      // Route incoming OSC messages to appropriate handlers
      bundle.dispatch("/solenoid/1", solenoid1);
      bundle.dispatch("/solenoid/2", solenoid2);
      bundle.dispatch("/solenoid/3", solenoid3);
    } else {
      OSCErrorCode error = bundle.getError();
      Serial.print("OSC Bundle error: ");
      Serial.println(error);
    }
  }
}
