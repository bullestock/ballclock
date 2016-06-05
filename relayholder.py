#! /usr/bin/python
# -*- coding: utf-8 -*-
from __future__ import division
import os, sys, re

from mcdefs import *

SEGMENTS = 48

hole_h = 15
hole_w = 14
slot_w = 20
slot_h = 3

def holder():
    th = 4
    o = translate([-block_w/2, -d/2, flange_h-0.1])(cube([block_w, d, hole_h]))
    i = translate([-hole_w/2, -d/2-1, flange_h-0.1])(cube([hole_w, d+2, hole_h+1]))
    slot = translate([-slot_w/2, -d/2-1, flange_h+3])(cube([slot_w, d+2, slot_h]))
    return o-i-slot

def flange():
    total_w = block_w+2*flange_w
    return translate([-total_w/2, -d/2, 0])(cube([total_w, d, flange_h]))

def assembly():
    fl = flange()
    mh = cylinder(h = flange_h+5, r = mounthole_d/2)
    nh = translate([0, 0, flange_h-3])(nuthole())
    h1 = translate([-(block_w/2+flange_w/2), 0, -1])(mh+nh)
    nh2 = nuthole()
    h2 = translate([(block_w/2+flange_w/2), 0, -1])(mh+nh)
    ho = holder()
    return fl + ho - h1 - h2

if __name__ == '__main__':
    a = assembly()    
    scad_render_to_file( a, file_header='$fn = %s;'%SEGMENTS, include_orig_code=True)
