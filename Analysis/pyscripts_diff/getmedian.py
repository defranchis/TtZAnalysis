import ROOT
from ROOT import *
import sys

def getmedian (histo, low, high):
    if low==300 : bin=1
    elif low==420 : bin=2
    elif low==550 : bin=3
    else : bin=4
    bin1 = h.FindBin(low)
    bin2 = h.FindBin(high)-1
    if high==9999 : bin2 = h.GetNbinsX() 
    tot=0
    for i in range(bin1,bin2):
        tot+=h.GetBinContent(i)
        if tot>h.Integral(bin1,bin2)/2. : 
            print 'median bin',bin,'=',h.GetBinLowEdge(i)
            break
    return


def getCenterGravity (histo, low, high) :

    bin=0
    if low==300 : bin=1
    elif low==420 : bin=2
    elif low==550 : bin=3
    else : bin=4

    bin1 = h.FindBin(low)
    bin2 = h.FindBin(high)-1
    M=0
    rm = 0
    for i in range(bin1,bin2):
        M  += h.GetBinContent(i)
        rm += h.GetBinContent(i)*h.GetBinCenter(i)

    center=rm/M
    print 'center of gravity bin',bin,'=',round(center,2)

    return



inFile = TFile(sys.argv[1],'READ')
h = inFile.Get('gen_5')
print
getmedian(h,300,420)
getmedian(h,420,550)
getmedian(h,550,810)
getmedian(h,810,9999)
print


getCenterGravity(h,300,420)
getCenterGravity(h,420,550)
getCenterGravity(h,550,810)
getCenterGravity(h,810,9999)
print
