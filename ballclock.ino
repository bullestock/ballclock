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
const int MAGNET_LO = 64;
const int ANGLE_UP = 112;
const int ANGLE_DOWN = 90;

const int SERVO_DELAY = 500; // ms
const int PICKUP_HI_DELAY = 500; // ms

const int STEP_DELAY = 150; // microseconds
const int MIN_STEP_DELAY = 50; // microseconds

const int x_home = 390;
const int y_home = 560;

// Distance between individual dots in the matrix, in steps
int STEPS_PER_CELL = 100*4;

int magnet_hi_pwr = MAGNET_HI;
int magnet_lo_pwr = MAGNET_LO;

const int MOTOR_X = 0;
const int MOTOR_Y = 1;

int current_x = 0;
int current_y = 0;

bool enable_enabled = true;

Servo servo;

void home(bool _goToZero = true);

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

void step(int m, bool reverse, int steps, bool enable = true, bool slow = false)
{
    if (enable && enable_enabled)
        digitalWrite(ENABLE, LOW);
    const int dir_pin = (m == MOTOR_X) ? X_DIR : Y_DIR;
    const int step_pin = (m == MOTOR_X) ? X_STEP : Y_STEP;
    digitalWrite(dir_pin, !reverse);
    for (int i = 0; i < steps; ++i)
    {
        digitalWrite(step_pin, HIGH);
        delayMicroseconds(STEP_DELAY);
        digitalWrite(step_pin, LOW);
        delayMicroseconds(STEP_DELAY);
    }
    digitalWrite(INTERNAL_LED, LOW);
    if (enable && enable_enabled)
        digitalWrite(ENABLE, HIGH);
}

void step_xy(int x_steps, int y_steps, bool enable = true, bool slow = false)
{
    if (enable && enable_enabled)
        digitalWrite(ENABLE, LOW);
    digitalWrite(X_DIR, x_steps > 0);
    digitalWrite(Y_DIR, y_steps > 0);

    const int common_steps = min(abs(x_steps), abs(y_steps));
    int step_delay = slow ? 2*STEP_DELAY : STEP_DELAY;
    const int step_delay_max = step_delay;
    const int ramp_count = 10;
    const int ramp_length = STEP_DELAY*ramp_count;
    int count = 0;
    int ramp_limit = min(ramp_length, common_steps/2);
    for (int i = 0; i < common_steps; ++i)
    {
        if (++count > ramp_count)
        {
            count = 0;
            if (i < ramp_limit)
            {
                // Ramp up
                if (step_delay > MIN_STEP_DELAY)
                    --step_delay;
            }
            else if (i > common_steps-ramp_limit)
            {
                // Ramp down
                if (step_delay < step_delay_max)
                    ++step_delay;
            }
        }
        digitalWrite(X_STEP, HIGH);
        digitalWrite(Y_STEP, HIGH);
        delayMicroseconds(step_delay);
        digitalWrite(X_STEP, LOW);
        digitalWrite(Y_STEP, LOW);
        delayMicroseconds(step_delay);
    }
    const int steps_left = max(abs(x_steps), abs(y_steps)) - common_steps;
    const int step_pin = (abs(x_steps) > abs(y_steps)) ? X_STEP : Y_STEP;
    ramp_limit = min(ramp_length, steps_left/2);
    for (int i = 0; i < steps_left; ++i)
    {
        if (++count > ramp_count)
        {
            count = 0;
            if (i < ramp_limit)
            {
                // Ramp up
                if (step_delay > MIN_STEP_DELAY)
                    --step_delay;
            }
            else if (i > steps_left-ramp_limit)
            {
                // Ramp down
                if (step_delay < step_delay_max)
                    ++step_delay;
            }
        }
        digitalWrite(step_pin, HIGH);
        delayMicroseconds(step_delay);
        digitalWrite(step_pin, LOW);
        delayMicroseconds(step_delay);
    }
    if (enable && enable_enabled)
        digitalWrite(ENABLE, HIGH);
}

void home(bool _goToZero)
{
    digitalWrite(ENABLE, LOW);
    bool x_limit_hit = digitalRead(X_LIMIT);
    bool y_limit_hit = digitalRead(Y_LIMIT);
    const int steps = 1;
    while (x_limit_hit || y_limit_hit)
    {
        if (x_limit_hit)
            step(MOTOR_X, false, steps, false);
        if (y_limit_hit)
            step(MOTOR_Y, false, steps, false);
        x_limit_hit = digitalRead(X_LIMIT);
        y_limit_hit = digitalRead(Y_LIMIT);
    }

    do
    {
        if (!x_limit_hit)
            step(MOTOR_X, true, steps, false, true);
        if (!y_limit_hit)
            step(MOTOR_Y, true, steps, false, true);
        x_limit_hit = digitalRead(X_LIMIT);
        y_limit_hit = digitalRead(Y_LIMIT);
    }
    while (!x_limit_hit || !y_limit_hit);

    if (_goToZero)
    {
        step(MOTOR_X, false, x_home);
        step(MOTOR_Y, false, y_home);
    }

    digitalWrite(ENABLE, HIGH);
    
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
    delay(SERVO_DELAY);
    //servo.detach();
    magnet_half();
}

void lift()
{
    //servo.attach(SERVO_OUT);
    servo.write(ANGLE_UP);
    delay(SERVO_DELAY);
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
    delay(SERVO_DELAY);
    //servo.detach();
    magnet_off();
    delay(500);
    servo.write(ANGLE_UP);
    delay(SERVO_DELAY);
}

void move(int x, int y)
{
    const int scaleFactor = STEPS_PER_CELL;
    int current_x_step = current_x*scaleFactor;
    int current_y_step = current_y*scaleFactor;
    int x_step = x*scaleFactor;
    int y_step = y*scaleFactor;
    step_xy(x_step - current_x_step, y_step - current_y_step);
    current_x = x;
    current_y = y;
}

void micro_move(int x, int y)
{
    home(false);
    step(MOTOR_X, false, x);
    step(MOTOR_Y, false, y);
}

void move(size_t n, const int* pos)
{
    for (size_t i = 0; i < n; ++i)
    {
        if ((i == 1) && n > 1)
            pickup();
        const int x = *pos++;
        const int y = *pos++;
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

void process_move(const char* buffer, bool microMove)
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
        Serial.println("Error: Odd number of arguments");
        return;
    }

    if (microMove)
        micro_move(coords[0], coords[1]);
    else
        move(coords_index/2, coords);
}

void process(const char* buffer)
{
    switch (buffer[0])
    {
    case 'R':
    case 'r':
        lift();
        home();
        break;

    case 'O':
    case 'o':
        {
            int index;
            x_home = get_int(buffer+1, BUF_SIZE-1, index); 
            y_home = get_int(buffer+index, BUF_SIZE-1, index); 
            home();
        }
        break;

    case 'e':
    case 'E':
        {
            int index;
            enable_enabled = (bool) get_int(buffer+1, BUF_SIZE-1, index); 
            digitalWrite(ENABLE, enable_enabled);
        }
        break;
        
    case 'M':
    case 'm':
        {
            bool microMove = false;
            int offset = 1;
            if ((buffer[1] == 'm') || (buffer[1] == 'M'))
            {
                microMove = true;
                ++offset;
            }
            process_move(buffer+offset, microMove);
        }
        break;

    case 'P':
    case 'p':
        pickup();
        break;

    case 'D':
    case 'd':
        drop();
        break;

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
        break;

    case 'L':
    case 'l':
        lift();
        break;

    default:
        Serial.print("Error: Unknown command '");
        Serial.print(buffer[0]);
        Serial.println("'");
        return;
    }
    Serial.print("OK ");
    Serial.println(buffer);
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
