#include <SoftwareSerial.h>

SoftwareSerial plotterSerial(7, 8); // RX, TX

int INTERNAL_LED = 13;
int MAGNET_HI_OUT = A0;
int MAGNET_LO_OUT = 12;

int x_home = 0;
int y_home = 0;

// Distance between individual dots in the matrix, in mm
const double dot_spacing = 10.0;

// Resolution of plotter coordinate system, in mm
const double resolution = .025;

const int scale = static_cast<int>(dot_spacing/resolution);

// TODO:
// - determine initial X move

void setup()
{
    pinMode(INTERNAL_LED, OUTPUT); 
    pinMode(MAGNET_LO_OUT, OUTPUT);
    pinMode(MAGNET_HI_OUT, OUTPUT);

    for (int i = 0; i < 10; ++i)
    {
        digitalWrite(INTERNAL_LED, HIGH);
        delay(50);
        digitalWrite(INTERNAL_LED, LOW);
        delay(50);
    }
    Serial.begin(57600);
    Serial.println("Ball Clock ready");

    plotterSerial.begin(9600);
    delay(1000);
    // Reset plotter to defaults
    plotterSerial.print("IN;");
    delay(100);
    plotterSerial.print("VS5;");
    delay(100);
    plotterSerial.print("PA;");
    delay(100);
    move(0, 0);
    delay(100);
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
    char buf[30];
    sprintf(buf, "PD%d,%d;", x_home+x*scale, y_home+y*scale);
    Serial.print("Send ");
    Serial.println(buf);
    plotterSerial.println(buf);
    delay(100);
    digitalWrite(INTERNAL_LED, LOW);
    delay(1000);
}

void move(size_t n, const int* pos)
{
    for (size_t i = 0; i < n; ++i)
    {
        if (i == 1)
            pickup();
        const int x = *pos++;
        const int y = *pos++;
        Serial.print("Go to ");
        Serial.print(x);
        Serial.print(", ");
        Serial.println(y);
        move(x, y);
    }
    drop();
}

const int BUF_SIZE = 200;
const int MAX_COORDS = 32;

int get_int(const char* buffer, int len)
{
    char intbuf[BUF_SIZE];
    memcpy(intbuf, buffer, max(BUF_SIZE-1, len));
    intbuf[len] = 0;
    return atoi(intbuf);
}

void process_move(const char* buffer)
{
    int coords[MAX_COORDS];
    int index = 0;
    int coords_index = 0;
    // Parse coords
    while (buffer[index])
    {
        while (buffer[index] && isspace(buffer[index]))
            ++index;
        if (!buffer[index])
        {
            break;
        }
        int start = index;
        while (buffer[index] && !isspace(buffer[index]))
            ++index;
        if (index == start)
        {
            break;
        }
        if (coords_index >= MAX_COORDS)
        {
            Serial.println("Error: Too many coords");
            return;
        }
        coords[coords_index++] = get_int(buffer+start, index-start);
        if (!buffer[index])
        {
            break;
        }
        ++index;
    }
    if (coords_index % 2)
    {
        Serial.println(coords_index);
        Serial.println(index);
        Serial.println("Error: Odd number of arguments");
        return;
    }

    move(coords_index/2, coords);
}

void process(const char* buffer)
{
    switch (buffer[0])
    {
    case 'R':
    case 'r':
        Serial.println("Resetting...");
        plotterSerial.print("IN;");
        delay(100);
        plotterSerial.print("VS2;");
        delay(100);
        move(0, 0);
        delay(100);
        Serial.println("Reset done");
        return;

    case 'M':
    case 'm':
        process_move(buffer+1);
        return;

    case 'P':
    case 'p':
        pickup();
        Serial.println("Picked up");
        return;

    case 'D':
    case 'd':
        drop();
        Serial.println("Dropped");
        return;

    case 's':
    case 'S':
        {
            int speed = atoi(buffer+1);
            char buf[30];
            sprintf(buf, "VS%d;", speed);
            plotterSerial.print(buf);
            delay(500);
            Serial.print("Speed set to ");
            Serial.println(speed);
        }
        break;
        
    case 't':
    case 'T':
        Serial.println("Run speed test");
        for (int speed = 2; speed < 27; ++speed)
        {
            char buf[30];
            sprintf(buf, "VS%d;", speed);
            plotterSerial.print(buf);
            delay(500);
            Serial.println(speed);
            plotterSerial.println("PD0,0;");
            delay(100);
            plotterSerial.println("PD500,0;");
            delay(100);
            plotterSerial.println("PD500,500;");
            delay(100);
            plotterSerial.println("PD0,500;");
            delay(100);
            plotterSerial.println("PD0,0;");
            delay(100);
        }
        break;
        
    default:
        Serial.print("Error: Unknown command '");
        Serial.print(buffer[0]);
        Serial.println("'");
        break;
    }
}

int index = 0;

char buffer[BUF_SIZE];

void loop()
{
    if (Serial.available())
    {
       // Command from PC
       char c = Serial.read();
       if ((c == '\r') || (c == '\n'))
       {
           buffer[index] = 0;
           index = 0;
           process(buffer);
       }
       else
       {
           if (index >= BUF_SIZE)
           {
               Serial.println("Error: Line too long");
               index = 0;
               return;
           }
           buffer[index++] = c;
       }
    }
    else
    {
        delay(10);
    }
    delay(10);
}
