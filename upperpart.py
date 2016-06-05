#! /usr/bin/python
# -*- coding: utf-8 -*-
from __future__ import division
import os, sys, re

from mcdefs import *

SEGMENTS = 48

hole_h = 25.5
hole_w = 16.5

def holder():
    th = 4
    o = translate([-block_w/2, -d/2, flange_h-0.1])(cube([block_w, d, hole_h+th]))
    i = translate([-hole_w/2, -d/2-1, flange_h-0.1])(cube([hole_w, d+2, hole_h]))
    return o-i

def flange():
    total_w = block_w+2*flange_w
    return translate([-total_w/2, -d/2, 0])(cube([total_w, d, flange_h]))

def assembly():
    fl = flange()
    mh1 = cylinder(h = flange_h+5, r = mounthole_d/2)
    h1 = translate([-(block_w/2+flange_w/2), 0, -1])(mh1)
    mh2 = cylinder(h = flange_h+5, r = mounthole_d/2)
    h2 = translate([(block_w/2+flange_w/2), 0, -1])(mh2)
    ho = holder()
    return fl + ho - h1 - h2

if __name__ == '__main__':
    a = assembly()    
    scad_render_to_file( a, file_header='$fn = %s;'%SEGMENTS, include_orig_code=True)
