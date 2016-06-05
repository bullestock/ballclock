#include <SoftwareSerial.h>

SoftwareSerial plotterSerial(7, 8); // RX, TX

int INTERNAL_LED = 13;
int MAGNET_HI_OUT = A0;
int MAGNET_LO_OUT = 12;

// 1 step = .0025 mm

int x_home = 0;
int y_home = 0;
int scale = static_cast<int>(8.0/.025);

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
    move(0, 0);
    for (int i = 0; i < 50; ++i)
    {
        digitalWrite(INTERNAL_LED, HIGH);
        delay(100);
        digitalWrite(INTERNAL_LED, LOW);
        delay(100);
    }
}

void pickup()
{
    digitalWrite(INTERNAL_LED, HIGH);
    digitalWrite(MAGNET_HI_OUT, HIGH);
    delay(100);
    digitalWrite(MAGNET_HI_OUT, LOW);
    digitalWrite(MAGNET_LO_OUT, HIGH);
    digitalWrite(INTERNAL_LED, LOW);
}

void drop()
{
    digitalWrite(INTERNAL_LED, HIGH);
    digitalWrite(MAGNET_LO_OUT, LOW);
    delay(500);
    digitalWrite(INTERNAL_LED, LOW);
}

void move(int x, int y)
{
    digitalWrite(INTERNAL_LED, HIGH);
    plotterSerial.print("PA");
    plotterSerial.print(x_home+x*scale);
    plotterSerial.print(',');
    plotterSerial.print(y_home+y*scale);
    plotterSerial.print(';');
    digitalWrite(INTERNAL_LED, LOW);
    delay(1000);
}

void move(size_t n, const int* pos)
{
    pickup();
    for (size_t i = 0; i < n; ++i)
    {
        const auto x = *pos++;
        const auto y = *pos++;
        Serial.print("Go to ");
        Serial.print(x);
        Serial.println(y);
        move(x, y);
    }
    drop();
}

template <typename T, size_t N>
constexpr size_t countof(T const (&)[N]) noexcept
{
    return N;
}

void loop()
{
    int p1[] = { 0, 0, 0, 2, 1, 1, 0, 3, 4, 3, 1, 4, 3, 5, 1, 6 };
    move(countof(p1)/2, p1);
    delay(1000);
}
