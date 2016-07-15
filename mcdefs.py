# Assumes SolidPython is in site-packages or elsewhwere in sys.path
from solid import *
from solid.utils import *

block_w = 24
flange_w = 10
flange_h = 3
mounthole_d = 4
d = 13

def screwhole():
    return up(-1)(cylinder(h = bracket_d+2, r = mounthole_d/2))

def hexagon(size, height):
  boxWidth = size/1.75;
  h = None
  for r in range(-60, 120, 60):
      c = rotate([0,0,r])(cube([boxWidth, size, height], True))
      if h == None:
          h = c
      else:
          h = h + c
  return h

def nuthole():
    return translate([0,0,3])(hexagon(7.5, 5))
