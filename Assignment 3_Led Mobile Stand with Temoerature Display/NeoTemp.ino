// This #include statement was automatically added by the Particle IDE.
#include "SparkFunMicroOLED/SparkFunMicroOLED.h"

// This #include statement was automatically added by the Particle IDE.

#include "OneWire/OneWire.h"
#include "neopixel/neopixel.h"

#define PIXEL_PIN D2
#define PIXEL_COUNT 24
#define PIXEL_TYPE WS2812B

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

MicroOLED oled;

OneWire ds = OneWire(D4);
String pos;
unsigned long lastUpdate = 0;
double sparkTempF;
int x;
void setup()
{
    Serial.begin(9600);
    pinMode(D3, OUTPUT);
    pinMode(D5, OUTPUT);
    digitalWrite(D3, LOW);
    digitalWrite(D5, HIGH);
    strip.begin();
    strip.show();
    Spark.function("getTemp", setTemp);
    Spark.variable("temperature", &sparkTempF, DOUBLE);
    oled.begin();    // Initialize the OLED
    oled.clear(ALL); // Clear the display's internal memory
    oled.display();  // Display what's in the buffer (splashscreen)
    delay(1000);     // Delay 1000 ms
    oled.clear(PAGE); // Clear the buffer.
    
}


void printOnOLED(String title, int font) //displays the value on the OLED screen
{
    oled.clear(PAGE);
    oled.setFontType(font);
    // Try to set the cursor in the middle of the screen
    oled.setCursor(0, 0);
    // Print the title:
    oled.print(title);
    oled.display();
    delay(1000);
    oled.clear(PAGE);
}


int setTemp(String posValue)
{
    pos = posValue;
    Serial.println(pos);
    strip.setBrightness(20);
    int data;
    sscanf(pos, "%data", &data);
    printOnOLED(pos,2);
    for (int i = 0; i < strip.numPixels(); i++) {
        if (i < data) {
            if (data < 10)
                strip.setPixelColor(i, strip.Color(0, 0, 255)); //blue
            else if (data < 15)
                strip.setPixelColor(i, strip.Color(255, 128, 0));//yellow
            else if (data < 20)
                strip.setPixelColor(i, strip.Color(255, 50, 0));//orange
            else
                strip.setPixelColor(i, strip.Color(255, 0, 0));//red
        }
        else 
            strip.setPixelColor(i, strip.Color(0, 0, 0));
        }
        strip.show();
        delay(100);
        return 0;
    }

void loop(void)
{
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    byte addr[8];
    double celsius, fahrenheit;

    if (!ds.search(addr)) {
        ds.reset_search();
        delay(250);
        return;
        
    
    }


    ds.reset(); // first clear the 1-wire bus
    ds.select(addr); // now select the device we just found
    ds.write(0x44, 0); // or start conversion in powered mode (bus finishes low)

    delay(1000); // maybe 750ms is enough, maybe not, wait 1 sec for conversion

    present = ds.reset();
    ds.select(addr);
    ds.write(0xB8, 0); // Recall Memory 0
    ds.write(0x00, 0); // Recall Memory 0

    // now read the scratch pad

    present = ds.reset();
    ds.select(addr);
    ds.write(0xBE, 0); // Read Scratchpad
    if (type_s == 2) {
        ds.write(0x00, 0); // The DS2438 needs a page# to read
    }

    for (i = 0; i < 9; i++) { // we need 9 bytes
        data[i] = ds.read();
    }
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s == 2)
        raw = (data[2] << 8) | data[1];
    byte cfg = (data[4] & 0x60);

    switch (type_s) {
    case 1:
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10) {
            // "count remain" gives full 12 bit resolution
            raw = (raw & 0xFFF0) + 12 - data[6];
        }
        celsius = (int)raw * 0.0625;
        break;
    case 0:
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00)
            raw = raw & ~7; // 9 bit resolution, 93.75 ms
        if (cfg == 0x20)
            raw = raw & ~3; // 10 bit res, 187.5 ms
        if (cfg == 0x40)
            raw = raw & ~1; // 11 bit res, 375 ms
        // default is 12 bit resolution, 750 ms conversion time
        celsius = (int)raw * 0.0625;
        break;

    case 2:
        data[1] = (data[1] >> 3) & 0x1f;
        if (data[2] > 127) {
            celsius = (int)data[2] - ((int)data[1] * .03125);
        }
        else {
            celsius = (int)data[2] + ((int)data[1] * .03125);
        }
    }

    fahrenheit = celsius * 1.8 + 32.0;
    sparkTempF = celsius;
}