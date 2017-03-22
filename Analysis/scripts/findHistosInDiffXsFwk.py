import ROOT, os
from ROOT import TFile, TH1, TString


outTxt = open(os.getenv('FIT_WORKDIR')+'/utils/distribsFromDiffXsFwk.txt','w')

diffFwkDir = '/nfs/dust/cms/user/defrancm/TopDiffXS_2015/CMSSW_7_2_2_patch2/src/TopAnalysis/Configuration/analysis/diLeptonic'

aFile = ROOT.TFile(diffFwkDir+'/selectionRoot/Nominal/emu/emu_run2015D.root')

for key in aFile.GetListOfKeys():
    keyName = TString(key.GetName())

    if keyName.Contains('bcp_'): continue
    if not keyName.Contains('Hyp'): continue
    if keyName.Contains('XSec'): continue

    inFile = ROOT.TFile(diffFwkDir+'/Plots/Nominal/emu/'+str(keyName)+'_source.root')
    if not inFile.IsZombie(): outTxt.write(str(keyName)+'\n')


