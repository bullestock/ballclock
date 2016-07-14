#include <Servo.h>

const int ENABLE = 6;
const int X_DIR = 8;
const int X_STEP = 9;
const int Y_DIR = 4;
const int Y_STEP = 5;
const int X_Y_RESET = 7;
const int X_LIMIT = A3;
const int Y_LIMIT = A2;
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

const int STEP_DELAY = 500;

const int x_home = 0;
const int y_home = 0;

// Distance between individual dots in the matrix, in steps
int STEPS_PER_CELL = 100;

int magnet_hi_pwr = MAGNET_HI;
int magnet_lo_pwr = MAGNET_LO;

const int MOTOR_X = 0;
const int MOTOR_Y = 1;

int current_x = 0;
int current_y = 0;

Servo servo;

void setup()
{
    pinMode(INTERNAL_LED, OUTPUT); 
    pinMode(MAGNET_OUT, OUTPUT);
    pinMode(ENABLE, OUTPUT);
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

    digitalWrite(ENABLE, LOW);
    
    digitalWrite(X_Y_RESET, LOW);
    delay(10);
    digitalWrite(X_Y_RESET, HIGH);
    delay(10);

    home();
    
    Serial.println("Ball Clock ready");
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
    digitalWrite(ENABLE, LOW);
    const int dir_pin = (m == MOTOR_X) ? X_DIR : Y_DIR;
    const int step_pin = (m == MOTOR_X) ? X_STEP : Y_STEP;
    digitalWrite(dir_pin, !reverse);
    bool led_state = LOW;
    int n = 0;
    for (int i = 0; i < steps; ++i)
    {
        if (++n > 10)
        {
            n = 0;
            led_state = !led_state;
            digitalWrite(INTERNAL_LED, led_state);
        }
        // Max is about 500 us
        digitalWrite(step_pin, HIGH);
        delayMicroseconds(STEP_DELAY);
        digitalWrite(step_pin, LOW);
        delayMicroseconds(STEP_DELAY);
    }
    digitalWrite(INTERNAL_LED, LOW);
    digitalWrite(ENABLE, HIGH);
}

void home()
{
    Serial.println("Homing");
    bool x_limit_hit = digitalRead(X_LIMIT);
    bool y_limit_hit = digitalRead(Y_LIMIT);
    Serial.print("X ");
    Serial.print(digitalRead(X_LIMIT));
    Serial.print(" Y ");
    Serial.println(digitalRead(Y_LIMIT));
    const int steps = 10;
    while (x_limit_hit || y_limit_hit)
    {
        if (x_limit_hit)
        {
            Serial.println("--x");
            step(MOTOR_X, false, steps);
        }
        if (y_limit_hit)
        {
            Serial.println("--y");
            step(MOTOR_Y, false, steps);
        }
        x_limit_hit = digitalRead(X_LIMIT);
        y_limit_hit = digitalRead(Y_LIMIT);
    }

    do
    {
        Serial.print("X ");
        Serial.print(x_limit_hit);
        Serial.print(" Y ");
        Serial.println(y_limit_hit);
        if (!x_limit_hit)
        {
            Serial.println("++x");
            step(MOTOR_X, true, steps);
        }
        if (!y_limit_hit)
        {
            Serial.println("++y");
            step(MOTOR_Y, true, steps);
        }
        x_limit_hit = digitalRead(X_LIMIT);
        y_limit_hit = digitalRead(Y_LIMIT);
    }
    while (!x_limit_hit || !y_limit_hit);

    Serial.println("Done");
    
    current_x = 0;
    current_y = 0;
}

void pickup()
{
    //servo.attach(SERVO_OUT);
    servo.write(ANGLE_DOWN);
    delay(500);
    magnet_full();
    delay(PICKUP_HI_DELAY);
    servo.write(ANGLE_UP);
    delay(DETACH_DELAY);
    //servo.detach();
    magnet_half();
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
    magnet_full();
}

void drop()
{
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
    while ((current_x != x) || (current_y != y))
    {
        if (current_x != x)
        {
            bool reverse = current_x > x;
            step(MOTOR_X, reverse, STEPS_PER_CELL);
            current_x += reverse ? -1 : 1;
        }
        if (current_y != y)
        {
            bool reverse = current_y > y;
            step(MOTOR_Y, reverse, STEPS_PER_CELL);
            current_y += reverse ? -1 : 1;
        }
    }
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
        lift();
        home();
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
