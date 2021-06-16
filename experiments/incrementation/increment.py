#!/usr/bin/env python

import sys
import nibabel as nib
import numpy as np
import socket
from time import time, sleep
from os import path as op, getpid

in_file=sys.argv[1]
out_folder=sys.argv[2]
it=int(sys.argv[3])

def benchmark(task, in_file):
    print("{0},{1},{2},{3},{4}".format(task, in_file, time(), getpid(), socket.gethostname()))

for i in range(it):
    if i == 0:
        out_f = op.join(out_folder, '{0}-inc-{1}'.format(i, op.basename(in_file)))
    else:
        out_f = op.join(out_folder, '{0}-{1}'.format(i, '-'.join(op.basename(in_file).split('-')[1:])))
    #print("iteration", i, "saving to", out_f)

    benchmark("read_start", in_file)
    im = nib.load(in_file)
    data = np.asanyarray(im.dataobj)
    data += 0 # make sure it's in memory
    benchmark("read_end", in_file)
    benchmark("inc_start", in_file)
    #sleep(1)
    data += 1
    benchmark("inc_end", in_file)
    inc_im = nib.Nifti1Image(data, im.affine, header=im.header)

    benchmark("save_start", in_file)
    nib.save(inc_im, out_f)
    benchmark("save_end", in_file)
    in_file = out_f
