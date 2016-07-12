#include <Servo.h>

const int X_DIR = 7;
const int X_STEP = 9;
const int Y_DIR = 4;
const int Y_STEP = 5;
const int X_Y_RESET = 8;
const int X_LIMIT = A2;
const int Y_LIMIT = A3;
const int SERVO_OUT = 3;
const int INTERNAL_LED = 13;
const int MAGNET_OUT = 11;
const int MAGNET_LED_OUT = 10;

const int MAGNET_HI = 128;
const int MAGNET_LO = 32;
const int ANGLE_UP = 112;
const int ANGLE_DOWN = 90;

const int DETACH_DELAY = 1500;
const int PICKUP_HI_DELAY = 1000;

const int x_home = 0;
const int y_home = 0;

// Distance between individual dots in the matrix, in mm
const double dot_spacing = 10.0;

// Resolution of plotter coordinate system, in mm
const double resolution = .025;

const int scale = static_cast<int>(dot_spacing/resolution);

int magnet_hi_pwr = MAGNET_HI;
int magnet_lo_pwr = MAGNET_LO;

const int MOTOR_X = 0;
const int MOTOR_Y = 1;

Servo servo;

// TODO:
// - determine initial X move

void setup()
{
    pinMode(INTERNAL_LED, OUTPUT); 
    pinMode(MAGNET_OUT, OUTPUT);
    pinMode(X_DIR, OUTPUT);
    pinMode(X_STEP, OUTPUT);
    pinMode(Y_DIR, OUTPUT);
    pinMode(Y_STEP, OUTPUT);
    pinMode(X_LIMIT, INPUT);
    pinMode(Y_LIMIT, INPUT);
    analogWrite(MAGNET_OUT, 255); // inverted
    pinMode(MAGNET_LED_OUT, OUTPUT);
    pinMode(SERVO_OUT, OUTPUT);

    servo.attach(SERVO_OUT);
    servo.write(ANGLE_UP);
    
    for (int i = 0; i < 3; ++i)
    {
        digitalWrite(INTERNAL_LED, HIGH);
        delay(50);
        digitalWrite(INTERNAL_LED, LOW);
        delay(50);
    }
    Serial.begin(57600);

    Serial.println("*** MOTOR TEST ***");

    digitalWrite(X_Y_RESET, LOW);
    delay(10);
    digitalWrite(X_Y_RESET, HIGH);
    delay(10);
    step(MOTOR_X, false, 100);
    delay(500);
    step(MOTOR_X, true, 100);
    delay(500);

    Serial.println("Ball Clock ready");
    //TODO: Home
}

void magnet_full()
{
    analogWrite(MAGNET_OUT, magnet_hi_pwr);
    analogWrite(MAGNET_LED_OUT, 255);
}

void magnet_half()
{
    analogWrite(MAGNET_OUT, magnet_lo_pwr);
    analogWrite(MAGNET_LED_OUT, 150);
}

void magnet_off()
{
    analogWrite(MAGNET_OUT, 255);
    analogWrite(MAGNET_LED_OUT, 0);
}

void step(int m, bool reverse, int steps)
{
    const int dir_pin = (m == MOTOR_X) ? X_DIR : Y_DIR;
    const int step_pin = (m == MOTOR_X) ? X_STEP : Y_STEP;
    digitalWrite(dir_pin, reverse);
    for (int i = 0; i < steps; ++i)
    {
        // Max is about 500 us
        digitalWrite(step_pin, HIGH);
        delayMicroseconds(900);
        digitalWrite(step_pin, LOW);
        delayMicroseconds(900);
    }
}

void pickup()
{
    digitalWrite(INTERNAL_LED, HIGH);
    //servo.attach(SERVO_OUT);
    servo.write(ANGLE_DOWN);
    delay(500);
    magnet_full();
    delay(PICKUP_HI_DELAY);
    servo.write(ANGLE_UP);
    delay(DETACH_DELAY);
    //servo.detach();
    magnet_half();
    digitalWrite(INTERNAL_LED, LOW);
}

void lift()
{
    //servo.attach(SERVO_OUT);
    servo.write(ANGLE_UP);
    delay(DETACH_DELAY);
    //servo.detach();
}

void hold()
{
    digitalWrite(INTERNAL_LED, HIGH);
    magnet_full();
}

void drop()
{
    digitalWrite(INTERNAL_LED, HIGH);
    delay(500);
    digitalWrite(INTERNAL_LED, LOW);
    //servo.attach(SERVO_OUT);
    servo.write(ANGLE_DOWN);
    delay(DETACH_DELAY);
    //servo.detach();
    magnet_off();
    delay(500);
    servo.write(ANGLE_UP);
    delay(DETACH_DELAY);
}

void move(int x, int y)
{
    digitalWrite(INTERNAL_LED, HIGH);
    //sprintf(buf, "PD%d,%d;", x_home+x*scale, y_home+y*scale);
    digitalWrite(INTERNAL_LED, LOW);
}

void move(size_t n, const int* pos)
{
    for (size_t i = 0; i < n; ++i)
    {
        if ((i == 1) && n > 1)
            pickup();
        const int x = *pos++;
        const int y = *pos++;
        Serial.print("Go to ");
        Serial.print(x);
        Serial.print(", ");
        Serial.println(y);
        move(x, y);
    }
    if (n > 1)
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

int get_int(const char* buffer, int len, int& next)
{
    int index = 0;
    while (buffer[index] && isspace(buffer[index]) && len)
    {
        ++index;
        --len;
    }
    while (buffer[index] && !isspace(buffer[index]) && len)
    {
        ++index;
        --len;
    }
    char intbuf[BUF_SIZE];
    memcpy(intbuf, buffer, index);
    intbuf[index] = 0;
    next = index+1;
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
        // TODO
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

    case 'w':
    case 'W':
        {
            int index;
            magnet_lo_pwr = get_int(buffer+1, BUF_SIZE-1, index); 
            magnet_hi_pwr = get_int(buffer+index, BUF_SIZE-1, index); 
            Serial.print("Power set to ");
            Serial.print(magnet_lo_pwr);
            Serial.print("/");
            Serial.println(magnet_hi_pwr);
        }
        break;
        
    case 't':
    case 'T':
        {
#if 1
            int index;
            int a1 = get_int(buffer+1, BUF_SIZE-1, index); 
            int a2 = get_int(buffer+index, BUF_SIZE-1, index); 
            Serial.print("Run servo test ");
            Serial.print(a1);
            Serial.print("/");
            Serial.println(a2);
            servo.attach(SERVO_OUT);
            for (int i = 0; i < 10; ++i)
            {
                Serial.println(i);
                delay(1000);
                servo.write(a1);
                delay(1000);
                servo.write(a2);
            }
            servo.detach();
#endif
#if 0
            for (int i = 20; i < 100; ++i)
            {
                Serial.println(i);
                analogWrite(MAGNET_LED_OUT, i);
                delay(500);
            }
#endif
        }
        break;
        
    case 'H':
    case 'h':
        hold();
        Serial.println("Holding");
        return;

    case 'L':
    case 'l':
        lift();
        Serial.println("Lifting");
        return;

    case 'x':
    case 'X':
        Serial.println("Run X motor test");
        while (1)
        {
            step(MOTOR_X, false, 100);
            delay(500);
            step(MOTOR_X, true, 100);
            delay(500);
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
