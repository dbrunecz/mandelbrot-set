#!/bin/bash
BMP="/tmp/tmp-mandelbrot.bmp"
MNDL_WD=4000 ./mandelbrot -1.2 -1 0.2 0.3 > $BMP && eog $BMP &
