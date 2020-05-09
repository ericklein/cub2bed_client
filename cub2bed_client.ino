/*
  Project Name:   cub2bed_client
  Developer:      Eric Klein Jr. (temp2@ericklein.com)
  Description:    client for cub2bed solution

  See README.md for target information, revision history, feature requests, etc.
*/

#define DEBUG       // Output to the serial port

//button
#include <buttonhandler.h>
#define sendMessageButtonPin  9
#define longButtonPressDelay  3000
ButtonHandler buttonSendMessage(sendMessageButtonPin,longButtonPressDelay);
// globals related to buttons
enum { BTN_NOPRESS = 0, BTN_SHORTPRESS, BTN_LONGPRESS };

//LED
//#define ledPin  10

//NeoPixel
#include <Adafruit_NeoPixel.h>
#define neoPixelPin  10
#define ledCount  1
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(ledCount, neoPixelPin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(ledCount, neoPixelPin);

// 900mhz radio
#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
// Conditional code compile for radio pinout
#if defined(ARDUINO_SAMD_ZERO)
  #define RFM69_CS      8
  #define RFM69_INT     3
  #define RFM69_RST     4
#endif
#if defined (__AVR_ATmega328P__)
  #define RFM69_INT     3
  #define RFM69_CS      4
  #define RFM69_RST     2
#endif
// server address
#define SERVER_ADDRESS  1
// unique addresses for each client, can not be server address
#define MY_ADDRESS      2
// Instantiate radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);
// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);
// 400-[433]-600 for LoRa radio, 850-[868,915]-950 for 900mhz, must match transmitter
#define RF69_FREQ 915.0
// 14-20dbm for RFM69HW power
#define RF69_POWER 20
// packet counter, incremented per transmission

void setup() 
{
  #ifdef DEBUG
    Serial.begin(115200);
    while (!Serial) 
      {
        delay(1);
      }
  #endif

  // radio setup
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

   // radio reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  if (!rf69_manager.init())
  {
    #ifdef DEBUG
      Serial.println("RFM69 radio init failed");
    #endif
    while (1);
  }
  
  // Default after radio init is 434.0MHz
  if (!rf69.setFrequency(RF69_FREQ))
  {
    #ifdef DEBUG
      Serial.println("RFM69 set frequency failed");
    #endif
    while (1);
  }
  
  // Defaults after radio init are modulation GFSK_Rb250Fd250, +13dbM (for low power module), no encryption
  // For RFM69HW, 14-20dbm for power, 2nd arg must be true for 69HCW
  rf69.setTxPower(RF69_POWER, true);

  // The encryption key has to match the transmitter
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  
  // radio successfully set up
  #ifdef DEBUG
    Serial.print("RFM69 radio OK @ ");
    Serial.print((int)RF69_FREQ);
    Serial.println("MHz. This node is client.");
  #endif

  // Setup push button
  buttonSendMessage.init();

  //LED
  // pinMode(ledPin, OUTPUT);
  // digitalWrite(ledPin, LOW);

  // neopixels
  strip.begin();
  strip.setPixelColor(0,0,255,0);
  strip.setBrightness(50);
  strip.show();
}

bool waitingForServerAction = false;
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

void loop() 
{
  // if radio is ready and we're waiting for the second message
  if ((rf69_manager.available()) && (waitingForServerAction))
  {
    // Wait for a message from a client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (rf69_manager.recvfromAck(buf, &len, &from))
    {
      #ifdef DEBUG
        Serial.print("Got packet from #");
        Serial.print(SERVER_ADDRESS);
        Serial.print(" [RSSI :");
        Serial.print(rf69.lastRssi());
        Serial.print("] : ");
        Serial.println((char*)buf);
      #endif
      if (strcmp((char *)buf,"ontheway") == 0)
      {
        // visually indicate that request was successful
        for (int i=0;i<10;i++)
          {
            strip.setPixelColor(0,0,255,0); // green
            strip.show();
            delay(100);
            strip.setPixelColor(0,0,0,0);
            strip.show();
            delay(100);
          }
          // return to normal state indicator
          strip.setPixelColor(0,0,255,0);
          strip.show();
        #ifdef DEBUG
          Serial.println("On the way message processed!");
        #endif
      }
      else if (strcmp((char *)buf,"needtowork") == 0)
      {
      // visually indicate that request was unsuccessful
      for (int i=0;i<10;i++)
          {
            strip.setPixelColor(0,255,0,0); // red
            strip.show();
            delay(100);
            strip.setPixelColor(0,0,0,0);
            strip.show();
            delay(100);
          }
          // return to normal status indicator
          strip.setPixelColor(0,0,255,0);
          strip.show();
      #ifdef DEBUG
        Serial.println("Need to work message processed");
      #endif
      }
      buf[len] = 0; // zero out remaining string
      // release second message blocker
      waitingForServerAction = false;
    }
  }
  resolveButtons();
}

void resolveButtons()
{
  switch (buttonSendMessage.handle()) 
  {
    case BTN_SHORTPRESS:
      // only handle button press if we aren't already processing a request
      if (!waitingForServerAction)
      {
        uint8_t data[] = "c2b";
        // Send message to the server
        if (rf69_manager.sendtoWait(data, sizeof(data),SERVER_ADDRESS))
        {
          #ifdef DEBUG
            Serial.print("button short press; server acknowledged receiving ");
            Serial.println((char*)data);
          #endif
          // led
          // digitalWrite(ledPin,HIGH);
          // visually let the sender know the server got the message
          strip.setPixelColor(0, 255, 255, 0); // yellow
          strip.show();
          waitingForServerAction = true;
        } 
        else
        {
          #ifdef DEBUG
            Serial.println("Retries exhausted, is server available?");
          #endif
        }
      }
      else
      {
        #ifdef DEBUG
          Serial.println("button press ignored");
        #endif
      }
    break;
    case BTN_LONGPRESS:
      #ifdef DEBUG
        Serial.println("button long press -> tbd");
      #endif
    break;
  }
}