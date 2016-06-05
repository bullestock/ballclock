#include <SoftwareSerial.h>

SoftwareSerial plotterSerial(7, 8); // RX, TX

int INTERNAL_LED = 13;
int MAGNET_HI_OUT = A0;
int MAGNET_LO_OUT = 12;

// 1 step = .0025 mm

int x_home = 1000;
int y_home = 1000;
int scale = 300;

void setup()
{
    pinMode(INTERNAL_LED, OUTPUT); 
    pinMode(MAGNET_LO_OUT, OUTPUT);
    pinMode(MAGNET_HI_OUT, OUTPUT);

    Serial.begin(9600);
    Serial.println("Marbleclock ready");

    plotterSerial.begin(9600);
    // Reset plotter to defaults
    plotterSerial.print("IN;");
    delay(100);
}

void pickup()
{
    digitalWrite(MAGNET_HI_OUT, HIGH);
    delay(100);
    digitalWrite(MAGNET_HI_OUT, LOW);
    digitalWrite(MAGNET_LO_OUT, HIGH);
}

void drop()
{
    digitalWrite(MAGNET_LO_OUT, LOW);
    delay(500);
}

void move(int x, int y)
{
    plotterSerial.print("PA");
    plotterSerial.print(x_home+x*scale);
    plotterSerial.print(',');
    plotterSerial.print(y_home+y*scale);
    plotterSerial.print(';');
}

void move(size_t n, const int* pos)
{
    pickup();
    for (size_t i = 0; i < n; ++i)
    {
        const auto x = *pos++;
        const auto y = *pos++;
        move(x, y);
    }
    drop();
}

void loop()
{
    digitalWrite(INTERNAL_LED, HIGH);
    pickup();
    digitalWrite(INTERNAL_LED, LOW);
    delay(5000);
    digitalWrite(INTERNAL_LED, HIGH);
    drop();
    digitalWrite(INTERNAL_LED, LOW);
    delay(5000);
}
