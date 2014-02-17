#! /usr/bin/python
#
# This program will take a file name from the command line and analyze its entropy, using many of the same algorithms
# as the ent program from hotbits
import sys
import struct
import math
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
import scipy.stats as stats

# This array contains the number of 1's contained in each byte value; 0-255
ones = [0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8]

byte2bin = ["00000000","00000001","00000010","00000011","00000100","00000101","00000110","00000111",
            "00001000","00001001","00001010","00001011","00001100","00001101","00001110","00001111",
            "00010000","00010001","00010010","00010011","00010100","00010101","00010110","00010111",
            "00011000","00011001","00011010","00011011","00011100","00011101","00011110","00011111",
            "00100000","00100001","00100010","00100011","00100100","00100101","00100110","00100111",
            "00101000","00101001","00101010","00101011","00101100","00101101","00101110","00101111",
            "00110000","00110001","00110010","00110011","00110100","00110101","00110110","00110111",
            "00111000","00111001","00111010","00111011","00111100","00111101","00111110","00111111",
            "01000000","01000001","01000010","01000011","01000100","01000101","01000110","01000111",
            "01001000","01001001","01001010","01001011","01001100","01001101","01001110","01001111",
            "01010000","01010001","01010010","01010011","01010100","01010101","01010110","01010111",
            "01011000","01011001","01011010","01011011","01011100","01011101","01011110","01011111",
            "01100000","01100001","01100010","01100011","01100100","01100101","01100110","01100111",
            "01101000","01101001","01101010","01101011","01101100","01101101","01101110","01101111",
            "01110000","01110001","01110010","01110011","01110100","01110101","01110110","01110111",
            "01111000","01111001","01111010","01111011","01111100","01111101","01111110","01111111",
            "10000000","10000001","10000010","10000011","10000100","10000101","10000110","10000111",
            "10001000","10001001","10001010","10001011","10001100","10001101","10001110","10001111",
            "10010000","10010001","10010010","10010011","10010100","10010101","10010110","10010111",
            "10011000","10011001","10011010","10011011","10011100","10011101","10011110","10011111",
            "10100000","10100001","10100010","10100011","10100100","10100101","10100110","10100111",
            "10101000","10101001","10101010","10101011","10101100","10101101","10101110","10101111",
            "10110000","10110001","10110010","10110011","10110100","10110101","10110110","10110111",
            "10111000","10111001","10111010","10111011","10111100","10111101","10111110","10111111",
            "11000000","11000001","11000010","11000011","11000100","11000101","11000110","11000111",
            "11001000","11001001","11001010","11001011","11001100","11001101","11001110","11001111",
            "11010000","11010001","11010010","11010011","11010100","11010101","11010110","11010111",
            "11011000","11011001","11011010","11011011","11011100","11011101","11011110","11011111",
            "11100000","11100001","11100010","11100011","11100100","11100101","11100110","11100111",
            "11101000","11101001","11101010","11101011","11101100","11101101","11101110","11101111",
            "11110000","11110001","11110010","11110011","11110100","11110101","11110110","11110111",
            "11111000","11111001","11111010","11111011","11111100","11111101","11111110","11111111"]

def calcscc( dt_array, tc ):
    sccfirst = 1              # Mark first time for serial correlation
    scct1 = scct2 = scct3 = sccun = sccu0 = 0.0   # Clear serial correlation terms 
    dt_size = len(dt_array)
    for idx in range(dt_size):
      sccun = dt_array[idx] + 0.0
      if (sccfirst):
         sccfirst = 0
         scclast = 0
         sccu0 = sccun
      else:
         scct1 = scct1 + scclast * sccun
      scct2 = scct2 + sccun
      scct3 = scct3 + (sccun * sccun)
      scclast = sccun
    scct1 = scct1 + scclast * sccu0;
    scct2 = scct2 * scct2
    scc = tc * scct3 - scct2
    if (scc == 0.0):
        scc = -100000
    else:
        scc = (tc * scct1 - scct2) / scc
    return (scc)

def calcent( hist_array, tc ):
    ent = 0.0
    for idx in range(256):
        prob = hist_array[idx] / (tc * 1.0)
        if (prob > 0.0):
            ent += prob * math.log((1/prob),2)
    return (ent)


def ent_bytes( original_array, hist_array ):
    bitsRead = 0
    totalOnes = 0
    totalc = 0
    for idx in range(256):
        totalc += hist_array[idx]
        totalOnes += hist_array[idx]*ones[idx]
        bitsRead += hist_array[idx]*8
    mean = totalOnes / float(bitsRead)
    cexp = totalc / 256.0
    chisq = 0.0
    datasum = 0
    for idx in range(256):
        a = hist_array[idx] - cexp
        chisq += (a * a) / cexp
        datasum += idx * hist_array[idx]
    entropy = calcent(hist_array, totalc)
    compression = ((8-entropy)/8)
    chisqProbability = 1.0 - stats.distributions.chi2.cdf(chisq, 255)
    serCorCoef = calcscc(original_array,totalc)
    arithmeticMean = datasum/(totalc*1.0)
    return({'bitsRead': bitsRead, 'totalOnes': totalOnes, 'totalc': totalc, 'cexp': cexp, 'chisq': chisq, 'entropy': entropy, 'compression': compression, 'chisqProbability': chisqProbability, 'serCorCoef': serCorCoef, 'arithmeticMean': arithmeticMean})

try:
    filename = sys.argv[1]
except:
    print "Must provide a filename to process! ./analyze.py filename"


try:
    data = np.fromfile(filename,dtype=np.uint8)
    fig = plt.figure(figsize=(8,10), dpi=100)
    ax = fig.add_subplot(211)
    n, bins, patches = ax.hist(data, bins=256)
    ax.set_xlabel('Byte Values')
    ax.set_ylabel('Frequency')
    ax.set_xlim(0,255)
    ax.set_title('Histogram of '+filename)
    ax.grid(False)
    #fn = filename + '.hist.png'
    #plt.savefig(fn,format='png')
    b = np.reshape(data[:len(data ) - len(data)%2], (-1, 2))
    bx = fig.add_subplot(212)
    bx.scatter(b[:-1],b[1:],c='0.9999',marker='.')
    bx.set_xlabel('Byte Values')
    bx.set_ylabel('Byte Values')
    bx.set_xlim(0,255)
    bx.set_ylim(0,255)
    bx.set_title('Scatter Plot of '+filename)
    bx.grid(False)
    fn = filename + '.png'
    plt.savefig(fn,format='png')
    
    entropy =  ent_bytes( data, n )
    totalZeroes = entropy['bitsRead'] - entropy['totalOnes']
    totalZeroesPercent = (entropy['bitsRead'] - entropy['totalOnes'])/(entropy['bitsRead']*1.0)
    totalOnesPercent = (entropy['totalOnes']/(entropy['bitsRead']*1.0))
    totalPercent = (entropy['bitsRead']/(entropy['totalc']*8.0))
    print " "
    print "Value Char Occurrences Fraction"
    print "{:4d}      {:11d}  {:12.10f}".format(0,totalZeroes,totalZeroesPercent)
    print "{:4d}      {:11d}  {:12.10f}".format(1,entropy['totalOnes'],totalOnesPercent)
    print "Total:    {:11d}  {:12.10f}".format(entropy['bitsRead'],totalPercent) 
    print " "
    print " "

    print "Value Char Occurrences     Fraction Expectation    Deviation"
    cumdev = 0.0
    for idx in range(256):
        cumdev += math.fabs(n[idx]-(entropy['totalc']/256.0))
        print "{:4d}      {:11,d}  {:12.9%} {:11,.2f} {:12,.4f}".format(idx,n[idx],(n[idx]/(entropy['totalc']*1.0)),(entropy['totalc']/256.0),math.fabs(n[idx]-(entropy['totalc']/256.0)))
    print "Total:    {:11,d}  {:12.7%}      Mean ={:13,.4f}".format(entropy['totalc'],(entropy['totalc']/(entropy['totalc']*1.0)),(cumdev/256.0))
    print " "
    print "Entropy = {:8.6f} bits per byte.".format(entropy['entropy'])
    print " "
    print "Optimum compression would reduce the size"
    print "of this {:,} byte file by {:.2%}".format(entropy['totalc'],entropy['compression'])
    print " "
    print "Chi square distribution for {:,} samples is {:.2f}, and randomly".format(entropy['totalc'],entropy['chisq'])
    print "would exceed this value {:.2%} percent of the time.".format(entropy['chisqProbability'])
    print " "
    print "Arithetic mean value of data bytes is {:.4f} (127.5 = random)".format(entropy['arithmeticMean'])
    print "Serial correlation coefficient is {:.6f} (totally uncorrelated = 0.0).".format(entropy['serCorCoef'])
except:
    print "Failed!"
