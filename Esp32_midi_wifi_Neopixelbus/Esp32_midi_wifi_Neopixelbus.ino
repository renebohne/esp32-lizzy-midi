/* Avoid these pins:
ESP32 has 6 strapping pins:
 MTDI/GPIO12: internal pull-down
 GPIO0: internal pull-up
 GPIO2: internal pull-down
 GPIO4: internal pull-down
 MTDO/GPIO15: internal pull-up
 GPIO5: internal pull-up
*/

// These need to be included when using standard Ethernet
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "AppleMidi.h"
#include <NeoPixelBus.h>


char ssid[] = "lizzymidi"; //  your network SSID (name)
char pass[] = "ledsmidi";    // your network password (use for WPA, or use as key for WEP)

int show_debug = 0;

// This pin drives a 2N2222A transistor (1k base resitor, 100Ohm Collector resitor)
//#define TRANSISTOR_PIN 23
//int transistorstate = 0;

// NEOPIXEL SETUP
#define PINL            25
#define PINR            26
#define NUMPIXELS      3

#define LOWEST_NOTE_L    60
#define LOWEST_NOTE_R    LOWEST_NOTE_L+NUMPIXELS

//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> pixelsL(NUMPIXELS, PINL);
//NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> pixelsR(NUMPIXELS, PINR);

NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt800KbpsMethod> pixelsL (NUMPIXELS, PINL);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt800KbpsMethod> pixelsR (NUMPIXELS, PINR);

unsigned long t0 = millis();
bool isConnected = false;

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h


/*
linker Arm:
oben G#-1 (20)
mitte D2 (50)
unten C-1 (12)

rechten Arm:
oben F#0 (30)
mitte E1 (40)
unten F#-1 (18)
*/
int mapping[] = {12, 50, 20, 18, 40, 30}; 

int getIndex(int note)
{
  for(int i=0; i<6; i++)
  {
    if(mapping[i] == note)
    {
      return i;
    }
  }
  return 0;
}

void setAllLeds(int v)
{
  for(int i=0; i<NUMPIXELS;i++)
  {
    pixelsL.SetPixelColor(i, RgbColor(v,v,v));
    pixelsR.SetPixelColor(i, RgbColor(v,v,v));
  }
  pixelsL.Show();
  pixelsR.Show();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  // Serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  //pinMode(TRANSISTOR_PIN, OUTPUT);


  Serial.print(F("Getting IP address..."));


  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F(""));
  Serial.println(F("WiFi connected"));


  Serial.println();
  Serial.print(F("IP address is "));
  Serial.println(WiFi.localIP());

    // SETUP NeoPixel
  pixelsL.Begin();
  delay(10);
  pixelsL.Show();

  pixelsR.Begin();
  delay(10);
  pixelsR.Show();

  
  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.print(F("Add device named Arduino with Host/Port "));
  Serial.print(WiFi.localIP());
  Serial.println(F(":5004"));
  Serial.println(F("Then press the Connect button"));
  Serial.println(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  // Create a session and wait for a remote host to connect to us
  AppleMIDI.begin("test");

  AppleMIDI.OnConnected(OnAppleMidiConnected);
  AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

  AppleMIDI.OnReceiveNoteOn(OnAppleMidiNoteOn);
  AppleMIDI.OnReceiveNoteOff(OnAppleMidiNoteOff);

  AppleMIDI.OnReceiveControlChange(OnAppleMidiControlChange);

  //Serial.println(F("Sending NoteOn/Off of note 45, every second"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  AppleMIDI.run();

  // send a note every second
  // (dont cáll delay(1000) as it will stall the pipeline)
  //if (isConnected && (millis() - t0) > 1000)
  //{
    //t0 = millis();
    //digitalWrite(TRANSISTOR_PIN, transistorstate);
    //transistorstate = !transistorstate;
    //   Serial.print(".");

    //byte note = 45;
    //byte velocity = 55;
    //byte channel = 1;

    //AppleMIDI.sendNoteOn(note, velocity, channel);
    //AppleMIDI.sendNoteOff(note, velocity, channel);
  //}
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(uint32_t ssrc, char* name) {
  isConnected  = true;
  Serial.print(F("Connected to session "));
  Serial.println(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(uint32_t ssrc) {
  isConnected  = false;
  Serial.println(F("Disconnected"));
}



void OnAppleMidiControlChange(byte channel, byte number, byte value){
  /*
  Serial.print(F("Incoming ControlChange from channel:"));
  Serial.print(channel);
  Serial.print(F(" number:"));
  Serial.print(number);
  Serial.print(F(" value:"));
  Serial.print(value);
  Serial.println();
  */

  if(number == 31)
  {
    setAllLeds(value);
  }
  else if(number == 1)
  {
    show_debug = (value>0);
    
  }
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOn(byte channel, byte note, byte velocity) {
  /*
  Serial.print(F("Incoming NoteOn from channel:"));
  Serial.print(channel);
  Serial.print(F(" note:"));
  Serial.print(note);
  Serial.print(F(" velocity:"));
  Serial.print(velocity);
  Serial.println();
*/

int i = getIndex(note);

/*
    int i = note - LOWEST_NOTE_L;
    if(i<0)
    {
      i=0;
    }
*/

    if(show_debug)
    {
      Serial.println(i);
    }
    
    if(i<NUMPIXELS)
    {
      pixelsL.SetPixelColor(i, RgbColor(100,100,100));
      pixelsL.Show();
    }
    else if(i<2*NUMPIXELS)
    {
      pixelsR.SetPixelColor(i-NUMPIXELS, RgbColor(100,100,100));
      pixelsR.Show();
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOff(byte channel, byte note, byte velocity) {
  /*
  Serial.print(F("Incoming NoteOff from channel:"));
  Serial.print(channel);
  Serial.print(F(" note:"));
  Serial.print(note);
  Serial.print(F(" velocity:"));
  Serial.print(velocity);
  Serial.println();
*/

int i = getIndex(note);


/*
  int i = note - LOWEST_NOTE_L;
  if(i<0)
  {
    i=0;
  }
    */

if(i<NUMPIXELS)
    {
      pixelsL.SetPixelColor(i, RgbColor(0,0,0));
      pixelsL.Show();
    }
    else if(i<2*NUMPIXELS)
    {
      pixelsR.SetPixelColor(i-NUMPIXELS, RgbColor(0,0,0));
      pixelsR.Show();
    }

    
}
