#!/usr/bin/env python

import sys
import nibabel as nib
import socket
from time import time
from os import path as op, getpid

in_file=sys.argv[1]

def benchmark(task, in_file):
    print("{0},{1},{2},{3},{4}".format(task, in_file, time(), getpid(), socket.gethostname()))


benchmark("open_start", in_file)
im = nib.load(in_file)
benchmark("open_end", in_file)
