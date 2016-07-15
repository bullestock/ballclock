#! /usr/bin/python
# -*- coding: utf-8 -*-
from __future__ import division
import os, sys, re

from mcdefs import *

SEGMENTS = 48

hole_h = 25.5
hole_w = 16.5
hinge_r = 5
hinge_hole_r = 1.5
th = 4
servo_holder_h = 5

def holder():
    o = translate([-block_w/2, -d/2, flange_h-0.1])(cube([block_w, d, hole_h+th]))
    return o

def holder_hole():
    th = 4
    i = translate([-hole_w/2, -d/2-1, flange_h-0.1])(cube([hole_w, d+2, hole_h]))
    return i

def flange():
    total_w = block_w
    return translate([-total_w/2, -d/2, 0])(cube([total_w, d, flange_h]))

def hinge():
    return translate([block_w/2-hinge_r, d/2, -hinge_r/4])(rotate([90, 0, 0])(cylinder(h = d, r = hinge_r)))

def hinge_hole():
    return translate([block_w/2-hinge_r, d/2+1, -hinge_r/4])(rotate([90, 0, 0])(cylinder(h = d+2, r = hinge_hole_r)))

def servo_holder():
    w = 11
    z_dim = 10
    h = translate([block_w/2+-0.1, 3, -w/2])(cube([w, z_dim, servo_holder_h]))
    return h

def assembly():
    fl = flange()
    ho = holder()
    hoho = holder_hole()
    h = hinge()
    hh = hinge_hole()
    sh1_z = hole_h+th+flange_h
    sh1 = up(sh1_z)(servo_holder())
    sh_dist = 27.5
    sh2 = up(sh1_z-sh_dist)(servo_holder())
    return rotate([90, 0, 0])(fl + ho + h - hh - hoho + sh1 + sh2)

if __name__ == '__main__':
    a = assembly()    
    scad_render_to_file( a, file_header='$fn = %s;'%SEGMENTS, include_orig_code=True)
