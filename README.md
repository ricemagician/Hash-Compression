Hash-Compression
================

This project compresses and uncompresses ppm image files into
ppm, pbm and offset files and then back.

Compression will compress a  picture in ppm format into 3 files: an
occupancy file in pbm format, an offset using a custom 8 bit per pixel
format and a ppm file for the hash data. These can in turn be converted
back into the original picture using the uncompress function using the
three resulting images.

The original assignment may be found here:
http://www.cs.rpi.edu/academics/courses/fall12/ds/hw/09_perfect_hashing/hw.pdf
