#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayManager
{
public:
    const int SCREEN_WIDTH = 128;
    const int SCREEN_HEIGHT = 64;
    const int OLED_RESET = -1;
    const int SCREEN_ADDRESS = 0x3C;

    Adafruit_SSD1306 display;

    inline DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 4000000, 4000000)
    {

    }

    inline void Init()
    {
        if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
        {
            Serial.println(F("SSD1306 allocation failed"));
        }
    }

    inline void Transfer()
    {
        display.display();
    }

    inline void Clear()
    {
        display.clearDisplay();
    }

    inline Adafruit_SSD1306* GetDisplay()
    {
        return &display;
    }
    
    inline void DrawChars() 
    {
        int a = micros();
        display.clearDisplay();

        display.setTextSize(1);      // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE); // Draw white text
        display.setCursor(0, 0);     // Start at top-left corner
        display.cp437(true);         // Use full 256 char 'Code Page 437' font

        // Not all the characters will fit on the display. This is normal.
        // Library will draw what it can and the rest will be clipped.
        for (int16_t i = 0; i < 256; i++) {
            if (i == '\n') display.write(' ');
            else          display.write(i);
        }
        int b = micros();
        Serial.print("Draw Char 1: ");
        Serial.println(b - a);

        a = micros();
        display.display();
        b = micros();
        Serial.print("Draw Char 2: ");
        Serial.println(b - a);
        delay(4000);
    }

    inline void DrawStyles()
    {
        int a = micros();
        display.clearDisplay();

        display.setTextSize(1);             // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);        // Draw white text
        display.setCursor(0, 0);            // Start at top-left corner
        display.println(F("Hello, world!"));

        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
        display.println(3.141592);

        display.setTextSize(2);             // Draw 2X-scale text
        display.setTextColor(SSD1306_WHITE);
        display.print(F("0x")); display.println(0xDEADBEEF, HEX);

        int b = micros();
        Serial.print("Draw Style 1: ");
        Serial.println(b - a);

        a = micros();
        display.display();
        b = micros();
        Serial.print("Draw Style 2: ");
        Serial.println(b - a);

        delay(4000);
    }

    inline void loop()
    {
        DrawChars();
        DrawStyles();
    }
};
