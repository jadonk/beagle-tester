#!/usr/bin/env python

# http://stackoverflow.com/questions/19131556/how-to-get-rgb888-24-bit-and-rgb565-16-bit-framebuffer-dump-from-a-jpg-ima

import sys
from PIL import Image

if len(sys.argv) == 4:
    # print "\nReading: " + sys.argv[1]
    out = open(sys.argv[2], "wb")
    mode = sys.argv[3]
elif len(sys.argv) == 3:
    out = open(sys.argv[2], "wb")
    mode = "-16"
elif len(sys.argv) == 2:
    out = sys.stdout
    mode = "-16"
else:
    print "Usage: png2fb.py infile [outfile [-32]]"
    sys.exit(1)

im = Image.open(sys.argv[1])

if im.mode == "RGB":
    pixelSize = 3
elif im.mode == "RGBA":
    pixelSize = 4
else:
    sys.exit('not supported pixel mode: "%s"' % (im.mode))

pixels = im.tobytes()
pixels2 = ""
if mode == "-32":
    print "Writing in 32-bit mode"
    for i in range(0, len(pixels) - 1, pixelSize):
        pixels2 += chr(ord(pixels[i + 1]))
        pixels2 += chr(ord(pixels[i]))
        pixels2 += chr(ord(pixels[i + 2]))
        pixels2 += chr(0)
elif mode == "-24":
    print "Writing in 24-bit mode"
    for i in range(0, len(pixels) - 1, pixelSize):
        pixels2 += chr(ord(pixels[i + 1]))
        pixels2 += chr(ord(pixels[i]))
        pixels2 += chr(ord(pixels[i + 2]))
else:
    print "Writing in 16-bit mode"
    for i in range(0, len(pixels) - 1, pixelSize):
        pixels2 += chr(ord(pixels[i + 2]) >> 3 | (ord(pixels[i + 1]) << 3 & 0xe0))
        pixels2 += chr(ord(pixels[i]) & 0xf8 | (ord(pixels[i + 1]) >> 5 & 0x07))
out.write(pixels2)
out.close()
