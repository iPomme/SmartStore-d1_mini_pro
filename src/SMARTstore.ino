#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "iot"
#define WLAN_PASS       "papapapa"
const char* WiFi_hostname = "iotstore";

WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";

// Variable to save if the buttons are pushed
int buttonUpState = 0;
int buttonDownState = 0;
bool buttonUpTriggered = false;
bool buttonDownTriggered = false;

// Assign output variables to GPIO pins
const int output5 = D2;
const int output4 = D1;
const int inputUpPin = D0;
const int inputDownPin = D8;

/************ Global State (you don't need to change this!) ******************/

WiFiClient client;

/*************************** Sketch Code ************************************/



void setup() {
  Serial.begin(9600);
  Serial.print(F("Setup... "));
  // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  Serial.print(F("OUTPUT pin has been set."));
  // // Initialize the button
  pinMode(inputUpPin, INPUT);
  pinMode(inputDownPin, INPUT);
  Serial.print(F("INPUT pin has been set."));

  // // Set outputs to LOW
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);
  Serial.print(F("OUTPUT pin has been set to LOW."));

  delay(10);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.hostname(WiFi_hostname);
  wifi_station_set_hostname(WiFi_hostname);
  // config static IP
  IPAddress ip(192, 168, 8, 90); // where xx is the desired IP Address
  IPAddress gateway(192, 168, 8, 1); // set gateway to match your network
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  IPAddress subnet(255 , 255, 255, 0); // set subnet mask to match your network
  IPAddress dns(8, 8, 8, 8);  //DNS
  WiFi.config(ip, gateway, subnet, dns);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  //  MDNS.begin(WiFi_hostname);
  while (WiFi.status() != WL_CONNECTED) {
    // wait for 500 milliseconds
    for (int i = 0; i <= 10; i++) {
      Serial.print(".");
      delay(50);
    }  
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  Serial.println("Host name: "); Serial.println(WiFi.hostname());
  server.begin();
}

void loop() {

 checkButtons();

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              output5State = "on";
              output4State = "off";                                           // set the 4  to low
              digitalWrite(output4, LOW);
              delay (100);
              digitalWrite(output5, HIGH);
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
              output5State = "off";
              digitalWrite(output5, LOW);
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              output5State = "off";
              digitalWrite(output5, LOW);                                     // set the 5  to low
              delay (100);
              digitalWrite(output4, HIGH);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, LOW);
            }

            Serial.print(F("\nSending photocell val "));
            Serial.print(digitalRead(output5));
            Serial.print(digitalRead(output4));

            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>Smart Roller Blind</h1>");
            client.println("<h2>powered by ESP8266</h2>");

            // Display current state, and ON/OFF buttons for GPIO 5
            client.println("<p>GPIO 5 - State " + output5State + "</p>");
            // If the output5State is off, it displays the ON button
            if (output5State == "off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">&#8593;</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">&#8593;</button></a></p>");
            }

            // Display current state, and ON/OFF buttons for GPIO 4
            client.println("<p>GPIO 4 - State " + output4State + "</p>");
            // If the output4State is off, it displays the ON button
            if (output4State == "off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">&#8595;</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">&#8595;</button></a></p>");
            }
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

}

void checkButtons() {
  buttonUpState = digitalRead(inputUpPin);
  buttonDownState = digitalRead(inputDownPin);

  // turns the GPIOs on and off
  if (buttonUpState  == HIGH && !buttonUpTriggered) {
    buttonUpTriggered = true;
    output5State = "on";
    output4State = "off";
    digitalWrite(output4, LOW);
    delay (100);
    digitalWrite(output5, HIGH);
  } else if (buttonUpState  == LOW && buttonUpTriggered) {
    buttonUpTriggered = false;
    output5State = "off";
    digitalWrite(output5, LOW);
  }
  if (buttonDownState == HIGH && !buttonDownTriggered) {
    buttonDownTriggered = true;
    output4State = "on";
    output5State = "off";
    digitalWrite(output5, LOW);
    delay (100);
    digitalWrite(output4, HIGH);
  } else if (buttonDownState  == LOW && buttonDownTriggered) {
    buttonDownTriggered = false;
    output4State = "off";
    digitalWrite(output4, LOW);
  }
}

