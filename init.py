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
import time
import RPi.GPIO as GPIO

import Adafruit_GPIO.SPI as SPI
import Adafruit_SSD1306
import subprocess

from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont

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

# Draw a black filled box to clear the image.
draw.rectangle((0,0,width,height), outline=0, fill=0)

padding = -2
top = padding
bottom = height-padding

# Load font.
font = ImageFont.truetype("/usr/share/fonts/truetype/arkpandora/AerialMono.ttf", 32)

digits = [ 0, 0, 0, 0 ]
maxval = [ 2, 9, 5, 9 ]

NOF_DIGITS = 4
BLINK_DELAY = 0.25
KEY_DELAY = 0.5

while True:
    cur_digit = 0
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
                    # Next digit
                    v = cur_digit + 1
                    if v >= NOF_DIGITS:
                        v = 0
                    cur_digit = v

        if k4:
            # Start
            break

        s = ''
        for i in range(0, NOF_DIGITS):
            if (i == cur_digit) and blink_state:
                s = s + "_"
            else:
                s = s + ("%d" % digits[i])
            if i == 1:
                s = s + ":"
            

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

    subprocess.call("/usr/bin/ruby clock.rb -t %d%d%d%d -s %d" % (digits[0], digits[1], digits[2], digits[3], nof_storage), shell = True)

    while not GPIO.input(K4_pin):
        time.sleep(1)
