# Copyright (c) 2017 Adafruit Industries
# Author: Tony DiCola & James DeVito
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
import time, sys
import RPi.GPIO as GPIO

import Adafruit_GPIO.SPI as SPI
import Adafruit_SSD1306
import subprocess

from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont

class Scroller:
    disp = None
    font = None
    text = None
    offset = 0

    def __init__(self, disp, font, s):
        self.disp = disp
        self.font = font
        self.text = s
        self.offset = 0
        width = disp.width
        height = disp.height
        image = Image.new('1', (width, height))
        draw = ImageDraw.Draw(image)
        padding = -2
        top = padding
        bottom = height-padding
        draw.rectangle((0, 0, width, height), outline=0, fill=0)
        draw.text((0, top), s, font = self.font, fill = 255)
        self.disp.image(image)
        self.disp.display()

    def update(self):
        self.offset = self.offset + 1
        if self.offset >= len(self.text):
            self.offset = 0
        width = self.disp.width
        height = self.disp.height
        image = Image.new('1', (width, height))
        draw = ImageDraw.Draw(image)
        padding = -2
        top = padding
        bottom = height-padding
        draw.rectangle((0, 0, width, height), outline=0, fill=0)
        draw.text((0, top), self.text[self.offset:], font = font, fill = 255)
        self.disp.image(image)
        self.disp.display()
    
K1_pin = 18
K2_pin = 24 
K3_pin = 22
K4_pin = 23 


GPIO.setmode(GPIO.BCM) 

GPIO.setup(K1_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP) # Input with pull-up
GPIO.setup(K2_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP) # Input with pull-up
GPIO.setup(K3_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP) # Input with pull-up
GPIO.setup(K4_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP) # Input with pull-up



# Raspberry Pi pin configuration:
RST = None     # on the PiOLED this pin isnt used
# Note the following are only used with SPI:
DC = 23
SPI_PORT = 0
SPI_DEVICE = 0

# 128x32 display with hardware I2C:
disp = Adafruit_SSD1306.SSD1306_128_32(rst=RST)

# Initialize library.
disp.begin()

# Clear display.
disp.clear()
disp.display()

# Create blank image for drawing.
# Make sure to create image with mode '1' for 1-bit color.
width = disp.width
height = disp.height
image = Image.new('1', (width, height))

# Get drawing object to draw on image.
draw = ImageDraw.Draw(image)

padding = -2
top = padding
bottom = height-padding

# Load font.
font = ImageFont.truetype("/usr/share/fonts/truetype/arkpandora/AerialMono.ttf", 24)

# Show current time and wait for key press
while True:
    cur_time = subprocess.check_output("/bin/date +%H:%M:%S", shell = True)
    draw.rectangle((0,0,width,height), outline=0, fill=0)
    draw.text((0, top), cur_time.replace('\n', ''), font = font, fill = 255)
    disp.image(image)
    disp.display()
    k1 = not GPIO.input(K1_pin)
    k2 = not GPIO.input(K2_pin)
    k3 = not GPIO.input(K3_pin)
    k4 = not GPIO.input(K4_pin)
    if k1 or k2 or k3 or k4:
        break
    time.sleep(0.01)

# Wait for key release
while True:
    k1 = not GPIO.input(K1_pin)
    k2 = not GPIO.input(K2_pin)
    k3 = not GPIO.input(K3_pin)
    k4 = not GPIO.input(K4_pin)
    if not k1 and not k2 and not k3 and not k4:
        break
    time.sleep(0.1)
    
digits = [ 0, 0, 0, 0 ]
maxval = [ 2, 9, 5, 9 ]

NOF_DIGITS = 4
MAX_STORAGE = 20
BLINK_DELAY = 0.25
KEY_DELAY = 0.5

while True:

    # Let user set parameters
    
    cur_digit = 0
    digits_mode = True
    nof_storage = 10

    blink_state = True

    last_blink = time.time()

    last_keypress = time.time()

    while True:
        k1 = not GPIO.input(K1_pin)
        k2 = not GPIO.input(K2_pin)
        k3 = not GPIO.input(K3_pin)
        k4 = not GPIO.input(K4_pin)

        # Draw a black filled box to clear the image.
        draw.rectangle((0,0,width,height), outline=0, fill=0)

        now = time.time()
    
        if k1 or k2 or k3:
            if now - last_keypress >= KEY_DELAY:
                last_keypress = now
                if digits_mode:
                    if k1:
                        # Up
                        v = digits[cur_digit] + 1
                        if v > maxval[cur_digit]:
                            v = 0
                        digits[cur_digit] = v
                    elif k2:
                        # Down
                        v = digits[cur_digit] - 1
                        if v < 0:
                            v = maxval[cur_digit]
                        digits[cur_digit] = v
                    elif k3:
                        # Next digit, or enter storage setting mode
                        v = cur_digit + 1
                        if v >= NOF_DIGITS:
                            digits_mode = False
                        else:
                            cur_digit = v
                else:
                    if k1:
                        # Up
                        if nof_storage < MAX_STORAGE:
                            v = nof_storage + 1
                        else:
                            v = 0
                        nof_storage = v
                    elif k2:
                        # Down
                        v = nof_storage - 1
                        if v < 0:
                            v = MAX_STORAGE
                        nof_storage = v
                    elif k3:
                        # Enter time setting mode
                        digits_mode = True
                        cur_digit = 0
                    
        if k4:
            # Start
            break

        s = ''
        for i in range(0, NOF_DIGITS):
            if digits_mode and (i == cur_digit) and blink_state:
                s = s + "_"
            else:
                s = s + ("%d" % digits[i])
            if i == 1:
                s = s + ":"
        s += "/"
        if not digits_mode and blink_state:
            s += "_" * len(("%d" % nof_storage))
        else:
            s += "%d" % nof_storage
            

        # Dynamically adjust max for digit 2
        if digits[0] > 1:
            maxval[1] = 3
        else:
            maxval[1] = 9

        draw.text((0, top), s, font = font, fill = 255)

        disp.image(image)
        disp.display()

        if time.time() - last_blink > BLINK_DELAY:
            blink_state = not blink_state
            last_blink = time.time()

    # Done setting time
    s = ''
    for i in range(0, NOF_DIGITS):
        s = s + ("%d" % digits[i])
        if i == 1:
            s = s + ":"
    s = s + "S"
    draw.text((0, top), s, font = font, fill = 255)
    disp.image(image)
    disp.display()

    cmd = "/usr/bin/ruby /home/pi/ballclock/clock.rb -t %d%d%d%d -s %d" % (digits[0], digits[1], digits[2], digits[3], nof_storage)
    print('Run %s' % cmd)
    subprocess.call(cmd, shell = True)
    print('Done')

    disp.clear()
    s = '?'
    try:
        with open('/home/pi/ballclock/error.txt', 'r') as myfile:
            s = myfile.read().replace('\n', '')
            scroller = Scroller(disp, font, s)
        # Wait for K4 release
        while not GPIO.input(K4_pin):
            time.sleep(0.1)
        # Show error until K4 press
        while GPIO.input(K4_pin):
            scroller.update()
            time.sleep(0.2)
        # Wait for K4 release
        while not GPIO.input(K4_pin):
            time.sleep(0.1)
    except FileNotFoundError:
        s = 'No error'
    
    

