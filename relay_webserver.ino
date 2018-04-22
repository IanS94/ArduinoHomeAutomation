#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,2);

EthernetServer server(80);

// Relay default state and pin
String relay1State = "OFF";
String relay2State = "OFF";
const int relay1 = 12;
const int relay2 = 13;

// Client variables 
char linebuf[80];
int charcount=0;

void setup() { 
  // Relay module prepared
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  
  // Open serial communication at a baud rate of 9600
  Serial.begin(9600);
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
}

void dashboardPage(EthernetClient &client) {
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("  <head>");
  client.println("    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("    <link rel=\"apple-touch-icon\" href=\"http://png.icons8.com/ios/150/3584fc/video-projector-filled.png\">");
  // Set styles for each media type (phone-tablet or desktop)
  client.println("    <style>");
  client.println("      @media only screen and (max-device-width: 800px) {");
  mobileCSS(client);
  client.println("      }");
  client.println("      @media only screen and (min-device-width: 801px) {");
  desktopCSS(client);
  client.println("      }");
  client.println("    </style>");
  //Declare functions for button clicks.
  client.println("    <script>");
  client.println("      function clickAction(relay,state) {");
  //..new thread
  client.println("        var xhr = new XMLHttpRequest();");
  //..open page using onclick parameters
  client.println("        xhr.open(\"GET\", ('/?' + relay + state), true);");
  client.println("        xhr.send();");
  //..refresh page so updated data is present
  client.println("        window.location.href = window.location.href;");
  client.println("      }");
  client.println("    </script>");
  
  client.println("  </head>");
  client.println("  <body>");
  
  client.println("    <h2>Command Center</h2>");
  
  client.println("    <table>");
  client.println("      <tbody>");
  
  // Relay 1
  client.println("        <tr>");
  client.println("          <th>Projector</th>");
  client.println("          <th class='right'>" + relay1State + "</th>");
  client.println("        </tr>");
  client.println("        <tr>");
  if(relay1State == "OFF"){
    client.println("          <th colspan=\"2\"><button onclick=\"clickAction('relay1','on');\">TURN ON</button></th>");
  }
  else if(relay1State == "ON"){
    client.println("          <th colspan=\"2\"><button onclick=\"clickAction('relay1','off');\">TURN OFF</button></th>");
  }
  client.println("        </tr>");

  // Relay 2
  client.println("        <tr>");
  client.println("          <th>Next Thing</th>");
  client.println("          <th class='right'>" + relay2State + "</th>");
  client.println("        </tr>");
  client.println("        <tr>");
  if(relay2State == "OFF"){
    client.println("          <th colspan=\"2\"><button onclick=\"clickAction('relay2','on');\">TURN ON</button></th>");
  }
  else if(relay2State == "ON"){
    client.println("          <th colspan=\"2\"><button onclick=\"clickAction('relay2','off');\">TURN OFF</button></th>");
  }
  client.println("        </tr>");

  client.println("      </tbody>");
  client.println("    </table>");
  
  client.println("  </body>");
  client.println("</html>"); 
}

void desktopCSS(EthernetClient &client) {
  client.println("        button{font-size:14px;width:300px;margin-bottom:12px;padding:4px 0px 4px 0px;}");
  client.println("        button#red{background-color: #FF7773;background: #FF7773 -webkit-gradient(linear, left top, left bottom, from(#D90700), to(#FF7773)) no-repeat;background: #FF7773 -moz-linear-gradient(top, #D90700, #FF7773) no-repeat;filter: progid:DXImageTransform.Microsoft.gradient(startColorstr=#D90700, endColorstr=#FF7773) no-repeat;-ms-filter: \"progid:DXImageTransform.Microsoft.gradient(startColorstr=#D90700, endColorstr=#D90700)\" no-repeat;}");
  client.println("        h2{text-align:left;}");
  client.println("        table{width:300px;text-align:left;}");
  client.println("        th.right{width:50px;text-align:right;}");
}

void mobileCSS(EthernetClient &client) {
  client.println("        button{font-size:14px;width:100%;margin-bottom:12px;padding:4px 0px 4px 0px;}");
  client.println("        button#red{background-color: #FF7773;background: #FF7773 -webkit-gradient(linear, left top, left bottom, from(#D90700), to(#FF7773)) no-repeat;background: #FF7773 -moz-linear-gradient(top, #D90700, #FF7773) no-repeat;filter: progid:DXImageTransform.Microsoft.gradient(startColorstr=#D90700, endColorstr=#FF7773) no-repeat;-ms-filter: \"progid:DXImageTransform.Microsoft.gradient(startColorstr=#D90700, endColorstr=#D90700)\" no-repeat;}");
  client.println("        h2{text-align:center;}");
  client.println("        table{width:100%;text-align:left;}");
  client.println("        th.right{width:20%;text-align:right;}");
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.print("New client hitting: ");
    memset(linebuf,0,sizeof(linebuf));
    charcount=0;
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
       char c = client.read();
       //read char by char HTTP request
        linebuf[charcount]=c;
        if (charcount<sizeof(linebuf)-1) charcount++;
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          dashboardPage(client);
          Serial.println("index.html");
          break;
        }
        if (c == '\n') {
          if (strstr(linebuf,"GET /?relay1off") > 0){
            digitalWrite(relay1, LOW);
            relay1State = "OFF";
            Serial.println("Projector OFF");
          }
          else if (strstr(linebuf,"GET /?relay1on") > 0){
            digitalWrite(relay1, HIGH);
            relay1State = "ON";
            Serial.println("Projector ON");
          }
          else if (strstr(linebuf,"GET /?relay2on") > 0){
            digitalWrite(relay2, HIGH);
            relay2State = "ON";
            Serial.println("Next Thing ON");
          }
          else if (strstr(linebuf,"GET /?relay2off") > 0){
            digitalWrite(relay2, LOW);
            relay2State = "OFF";
            Serial.println("Next Thing OFF");
          }
          else if (strstr(linebuf,"GET /desktop.css") > 0){
            desktopCSS(client);
            Serial.println("Desktop CSS");
            break;
          }
          else if (strstr(linebuf,"GET /mobile.css") > 0){
            mobileCSS(client);
            Serial.println("Mobile CSS");
            break;
          }
          // you're starting a new line
          currentLineIsBlank = true;
          memset(linebuf,0,sizeof(linebuf));
          charcount=0;          
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    // Serial.println("client disonnected");
  }
}
