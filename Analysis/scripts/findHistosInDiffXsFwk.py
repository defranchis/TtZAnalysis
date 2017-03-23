import ROOT, os
from ROOT import TFile, TH1, TString

outTxt = open(os.getenv('FIT_WORKDIR')+'/distribsFromDiffXsFwk.txt','w')

with open(os.getenv('FIT_WORKDIR')+'/configs/diffXsFwk/diffXsDirPath.txt','r') as f:
    l = f.readline().splitlines()

diffFwkDir = l[0]

aFile = ROOT.TFile(diffFwkDir+'/selectionRoot/Nominal/emu/emu_run2015D.root')

for key in aFile.GetListOfKeys():
    keyName = TString(key.GetName())

    if keyName.Contains('bcp_'): continue
    if not keyName.Contains('Hyp'): continue
    if keyName.Contains('XSec'): continue

    inFile = ROOT.TFile(diffFwkDir+'/Plots/Nominal/emu/'+str(keyName)+'_source.root')
    if not inFile.IsZombie(): outTxt.write(str(keyName)+'\n')

outTxt.close()
