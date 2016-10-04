#! /usr/bin/python
# -*- coding: utf-8 -*-
from __future__ import division
import os, sys, re

SEGMENTS = 48

from solid import *
from solid.utils import *

hole_h = 26
hole_w = 17
block_w = 25
block_upper_th = 4
block_lower_th = 1
width = 13

rail_w = 3
rail_th = 5

move_distance = 7

frame_outer_w = rail_w*2
frame_w = block_w+2*frame_outer_w
frame_h = hole_h+block_lower_th+block_upper_th+move_distance+frame_outer_w
frame_front_th = 3

servo_holder_w = 5
servo_holder_h = 11
servo_holder_z = 10
shh_base_h = 3
shh_support_h = 10

def ccube(a):
    return translate([ -a[0]/2, -a[1]/2, -a[2]/2 ])(cube(a))

def holder():
    o = up((block_upper_th-block_lower_th)/2)(ccube([block_w, width, hole_h+block_lower_th+block_upper_th]))
    return o

def holder_hole():
    i = ccube([hole_w, width+2, hole_h])
    return i

def rail():
    r = up((block_upper_th-block_lower_th)/2)(ccube([rail_w, rail_th, hole_h+block_lower_th+block_upper_th]))
    return r

def frame():
    o = back(frame_front_th/2)(ccube([frame_w, width+frame_front_th, frame_h]))
    return o

def frame_hole():
    i = up(frame_outer_w-4)(ccube([block_w+0.2, width+frame_front_th, frame_h]))
    return i

def notch():
    r = ccube([rail_w+1, rail_th+0.6, frame_h])
    return r

def servo_block():
    h = ccube([servo_holder_w, servo_holder_z, servo_holder_h])
    return h

def servo_holder():
    sh_dist = 27.5
    sh1 = left(sh_dist/2)(servo_block())
    sh2 = right(sh_dist/2)(servo_block())
    shh_base = down(servo_holder_h/2+shh_base_h/2-0.1)(ccube([sh_dist+servo_holder_w, servo_holder_z, shh_base_h]))
    return shh_base + sh1 + sh2

def shh_support():
    return forward(3)(ccube([15, 20, shh_support_h]) - forward(12.3)(rotate([60, 0, 0])(ccube([20, 20, shh_support_h]))))

def part1():
    ho = holder()
    hoho = holder_hole()
    r1 = left(block_w/2+rail_w/2-0.1)(rail())
    r2 = right(block_w/2+rail_w/2-0.1)(rail())
    shhs = shh_support()
    a = ho + r1 + r2 - hoho + up(shh_support_h+shh_support_h/2+hole_h/2+block_upper_th+shh_base_h)(forward(7.5)(servo_holder()))
    return a + up(shh_support_h/2+hole_h/2+block_upper_th)(shhs)

def part2():
    fr = frame()
    fh = frame_hole()
    n1 = left(block_w/2+rail_w/2)(up(frame_outer_w)(notch()))
    n2 = right(block_w/2+rail_w/2)(up(frame_outer_w)(notch()))
    return fr - n1 - n2 - fh

if __name__ == '__main__':
    scad_render_to_file(part1(), 'part1.scad')
    scad_render_to_file(part2(), 'part2.scad')
