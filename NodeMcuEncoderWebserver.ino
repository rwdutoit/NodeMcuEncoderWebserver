#include <avr/pgmspace.h>
#include <Ethernet.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "index.h" //Our HTML webpage contents with javascripts

ESP8266WebServer server(80); //instantiate server at port 80 (http port)

const char *ssid = "du_Toit";
const char *password = "Master101!";
const char *host = "Encoder";

const bool pollingNotInterrupt = true; //Interrupts have to be fixed

int encoderPinA = D1;
int encoderPinB = D2;
int outputPinA = D6;
int outputPinB = D7;
volatile int encoderAPos = 0;
volatile int encoderANeg = 0;
volatile int encoderBPos = 0;
volatile int encoderBNeg = 0;
volatile int holdOnCountMax = 8000;
volatile int holdOffCountMax = 100;
volatile int encoderPinAHoldoff = holdOffCountMax; // false;
volatile int encoderPinBHoldoff = holdOffCountMax; // false;
volatile int8_t valueA = LOW;
volatile int8_t valueB = LOW;
volatile int8_t prevValueA = LOW;
volatile int8_t prevValueB = LOW;
volatile uint8_t ledBuiltin = 1;

void clearCounters()
{
  encoderAPos = 0;
  encoderANeg = 0;
  encoderBPos = 0;
  encoderBNeg = 0;
}

void toggleLed()
{
  // ledBuiltin ^= 1;
  // digitalWrite(LED_BUILTIN, ledBuiltin);
}

bool holdingA(uint8_t checkvalue)
{
  volatile uint64_t encoderPinAMaintain = 0;
  volatile uint64_t time = micros();
  bool currentCountBelow = true;
  int8_t valueBstart = digitalRead(encoderPinB);
  do
  {
    valueA = digitalRead(encoderPinA);
    valueB = digitalRead(encoderPinB);
    encoderPinAMaintain = micros() - time;
    currentCountBelow = (encoderPinAMaintain < holdOnCountMax);
  } while (valueA == checkvalue && currentCountBelow && valueBstart == valueB);

  if (valueA == checkvalue && !currentCountBelow)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool holdingB(uint8_t checkvalue)
{
  volatile uint64_t encoderPinBMaintain = 0; // false;
  volatile uint64_t time = micros();
  bool currentCountBelow = true;
  int8_t valueAstart = digitalRead(encoderPinA);
  do
  {
    valueB = digitalRead(encoderPinB);
    valueA = digitalRead(encoderPinA);
    encoderPinBMaintain = micros() - time;
    currentCountBelow = (encoderPinBMaintain < holdOnCountMax);
  } while (valueB == checkvalue && currentCountBelow && valueAstart == valueA);

  if (valueB == checkvalue && !currentCountBelow)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void checkA()
{
  valueB = digitalRead(encoderPinB);
  valueA = digitalRead(encoderPinA);
  if (valueA == 1) //rising edge
  {
    if (holdingA(1))
    {
      toggleLed();
      if (valueB == 1)
      {
        encoderAPos++;
        volumeDown();
      }
      else
      {
        encoderANeg++;
        volumeUp();
      }
    }
  }
  if (valueA == 0) //falling edge
  {
    if (holdingA(0))
    {
      toggleLed();
      if (valueB == 0)
      {
        encoderAPos++;
        volumeDown();
      }
      else
      {
        encoderANeg++;
        volumeUp();
      }
    }
  }
}

void checkB()
{
  valueB = digitalRead(encoderPinB);
  valueA = digitalRead(encoderPinA);
  if (valueB == 1) //rising edge
  {
    if (holdingB(1))
    {
      toggleLed();
      if (valueA == 1)
      {
        encoderBNeg++;
        volumeUp();
      }
      else
      {
        encoderBPos++;
        volumeDown();
      }
    }
  }
  if (valueB == 0) //falling edge
  {
    if (holdingB(0))
    {
      toggleLed();
      if (valueA == 0)
      {
        encoderBNeg++;
        volumeUp();
      }
      else
      {
        encoderBPos++;
        volumeDown();
      }
    }
  }
}

void EncoderLoop()
{
  encoderPinAHoldoff++;
  encoderPinBHoldoff++;

  if (pollingNotInterrupt == true)
  {
    valueB = digitalRead(encoderPinB);
    valueA = digitalRead(encoderPinA);

    int8_t changeA = (valueA ^ prevValueA);
    int8_t changeB = (valueB ^ prevValueB);

    if (valueA == 0 && valueB == 0)
    {
      if (ledBuiltin == 0)
      {
        digitalWrite(LED_BUILTIN, 1);
      }
      ledBuiltin = 1;
    }
    else
    {
      if (ledBuiltin == 1)
      {
        digitalWrite(LED_BUILTIN, 0);
      }
      ledBuiltin = 0;
    }

    if (changeA == 1)
    {
      checkA();
    }

    if (changeB == 1)
    {
      checkB();
    }
    prevValueA = valueA;
    prevValueB = valueB;
  }
}
ICACHE_RAM_ATTR void ChangeA()
{
  noInterrupts();
  checkA();
  interrupts();
}
ICACHE_RAM_ATTR void ChangeB()
{
  noInterrupts();
  checkB();
  interrupts();
}

void loop()
{
  ArduinoOTA.handle();
  EncoderLoop();
  server.handleClient();
}

void handleRoot()
{
  // String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", FPSTR(MAIN_page)); //Send web page
}

void getEncoder()
{
  // read some values and convert to string
  String stringOne = "fowardA: ";
  stringOne += encoderAPos;
  stringOne += ", forwardB: ";
  stringOne += encoderBPos;
  stringOne += "\r\n<br>, backwardA: ";
  stringOne += encoderANeg;
  stringOne += ", backwardB: ";
  stringOne += encoderBNeg;
  // Serial.println("WebRequest: " + stringOne);
  server.send(200, "text/plane", stringOne); //Send value only to client ajax request
}

void handleForm()
{
  String holdOnCount = server.arg("holdOnCount");
  if (holdOnCount.length() > 0)
  {
    Serial.print("holdOnCount:");
    Serial.println(holdOnCount);
    holdOnCountMax = holdOnCount.toDouble();

    String s = "<div>holdOnCountMax set to " + holdOnCount + "</div><a href='/'> Go Back </a>";
    server.send(200, "text/html", s); //Send web page
  }

  String holdOffCount = server.arg("holdOffCount");
  if (holdOffCount.length() > 0)
  {
    Serial.print("holdOffCount:");
    Serial.println(holdOffCount);
    holdOffCountMax = holdOffCount.toDouble();

    String s = "<div>holdOffCountMax set to " + holdOffCount + "</div><a href='/'> Go Back </a>";
    server.send(200, "text/html", s); //Send web page
  }

  clearCounters();
}

void delayCycle()
{
  int countTime = 0;
  int countFirst = micros();
  do
  {
    countTime = micros() - countFirst;
  } while (countTime < (holdOnCountMax));
}

bool volumeBusy = false;
void volumeUpCommand()
{
  volumeUp();
  server.send(200, "text/html", ""); //Send web page
}
void volumeUp()
{
  if (volumeBusy == false)
  {
    volumeBusy = true;

    Serial.println("VoulemUP");

    int valueAcurrent = digitalRead(outputPinA);
    if (valueAcurrent == HIGH)
    {
      digitalWrite(outputPinA, LOW);
      delayCycle();
      digitalWrite(outputPinB, LOW);
      delayCycle();
    }
    else //if(valueAcurrent == LOW)
    {
      digitalWrite(outputPinA, HIGH);
      delayCycle();
      digitalWrite(outputPinB, HIGH);
      delayCycle();
    }

    Serial.println("VoulemUP DONE");

    encoderBPos++;
    encoderAPos++;

    volumeBusy = false;
  }
}

void volumeDownCommand()
{
  volumeDown();
  server.send(200, "text/html", ""); //Send web page
}
void volumeDown()
{
  if (volumeBusy == false)
  {
    volumeBusy = true;

    Serial.println("VoulemDOWN");

    int valueBcurrent = digitalRead(outputPinB);
    if (valueBcurrent == HIGH)
    {
      digitalWrite(outputPinB, LOW);
      delayCycle();
      digitalWrite(outputPinA, LOW);
      delayCycle();
    }
    else //if(valueBcurrent == LOW)
    {
      digitalWrite(outputPinB, HIGH);
      delayCycle();
      digitalWrite(outputPinA, HIGH);
      delayCycle();
    }

    Serial.println("VolumeDOWN DONE");

    encoderBNeg++;
    encoderANeg++;

    volumeBusy = false;
  }
}

void volumeSet()
{
  String message = "";
  if (server.arg("volumestep") == "" || server.arg("upOrDown") == "")
  { //Parameter not found
    message = "volumestep Argument not found";
    server.send(400, "text/plain", message); //Returns the HTTP response
  }
  else
  { //Parameter found
    message = "volumestep Argument = ";
    message += server.arg("volumestep"); //Gets the value of the query parameter
    int volumestep = server.arg("volumestep").toInt();
    String upOrDown = server.arg("upOrDown");
    for (size_t i = 0; i < volumestep; i++)
    {
      if(upOrDown == "true"){
        volumeUp();
      }else{
        volumeDown();
      }
      server.send(200, "text/plain", message); //Returns the HTTP response
    }
  }
}
void setupWifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot); //Which routine to handle at root location. This is display page
  server.on("/getEncoder", getEncoder);
  server.on("/action_page", handleForm);        //form action is handled here
  server.on("/volume_up", volumeUpCommand);     //form action is handled here
  server.on("/volume_down", volumeDownCommand); //form action is handled here
  server.on("/volume_set", volumeSet);          //form action is handled here

  server.begin();
  Serial.println("Web server started!");
}

void setupOta()
{
  Serial.println("setupOta");

  ArduinoOTA.setHostname(host);
  ArduinoOTA.onStart([]() { // switch off all the PWMs during upgrade
    Serial.println("onStart");
  });

  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end
    Serial.println("onEnd");
  });

  ArduinoOTA.onError([](ota_error_t error)
                     {
                       (void)error;
                       Serial.println("onError");
                       ESP.restart();
                     });

  /* setup the OTA server */
  ArduinoOTA.begin();
  Serial.println("Ready");
}

void encoderSetup()
{
  pinMode(outputPinA, OUTPUT_OPEN_DRAIN);
  pinMode(outputPinB, OUTPUT_OPEN_DRAIN);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  if (pollingNotInterrupt == false)
  {
    attachInterrupt(digitalPinToInterrupt(encoderPinA), ChangeA, RISING);
    attachInterrupt(digitalPinToInterrupt(encoderPinB), ChangeB, RISING);
  }
  Serial.println(String(encoderAPos));
  Serial.println(String(valueA));
  Serial.println(String(valueB));
}

void setup()
{
  Serial.begin(115200);
  setupWifi();
  setupOta();
  encoderSetup();
}