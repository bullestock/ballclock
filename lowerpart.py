#! /usr/bin/python
# -*- coding: utf-8 -*-
from __future__ import division
import os, sys, re

from mcdefs import *

SEGMENTS = 48

hinge_th = 2
plate_w = d+2*hinge_th
plate_l = 100
plate_th = 3
hinge_r = 5
hinge_hole_r = 1.5

def plate():
    o = translate([-plate_l, -plate_w/2, 0])(cube([plate_l, plate_w, plate_th]))
    return o

def hinge():
    c = rotate([90, 0, 0])(cylinder(h = hinge_th, r = hinge_r))
    s = translate([-hinge_r, -hinge_th, -hinge_r-1])(cube([2*hinge_r, hinge_th, hinge_r+1]))
    return translate([0, 0, hinge_r+0.9])(c+s)

def hinge_hole():
    return translate([0, 1, hinge_r])(rotate([90, 0, 0])(cylinder(h = hinge_th+2, r = hinge_hole_r)))

def assembly():
    pl = plate()
    h1 = translate([-hinge_r, plate_w/2, plate_th-0.1])(hinge()-hinge_hole())
    h2 = translate([-hinge_r, -plate_w/2+hinge_th, plate_th-0.1])(hinge()-hinge_hole())
    return pl + h1 + h2

if __name__ == '__main__':
    a = assembly()    
    scad_render_to_file( a, file_header='$fn = %s;'%SEGMENTS, include_orig_code=True)
