#!/usr/bin/python

import numpy as np
from libtiff import TIFFfile
from libtiff import TIFF

def read(fileName):
    """
    Script to import tif file from imageJ,
    usage: zstack =  tiff.read(inFileName)
    PTW 2015/01/29
    """
    tiff = TIFFfile(fileName)
    samples, sample_names = tiff.get_samples()
    
    outList = []
    for sample in samples:
        outList.append(np.copy(sample)[...,np.newaxis])
        
    out = np.concatenate(outList,axis=-1)
    out = np.rollaxis(out,0,3)
    out = np.flipud(out)
    
    tiff.close()
    
    return out

print(read('a.tiff'))
