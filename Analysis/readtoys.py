import time
import os
import ROOT
from ROOT import *

# gStyle.SetOptStat(0000)

# base_dir = 'toys_workdir/06_newDY_mlb'
base_dir = 'toys_workdir'
# base_dir = 'toys_workdir/07_newDY_mlb'
filelist = os.listdir(base_dir)

h_mass = ROOT.TH1F('h_mass','h_mass',700,160.,185.)
h_xsec = ROOT.TH1F('h_xsec','h_xsec',500,730,930)

nfile = 0

for texfile in filelist:
    if not texfile.endswith('.tex'): continue
    if not texfile.startswith('xsecFit_tab_TOPMASS_'): continue #redundant

    nfile += 1
    if nfile%1000 == 0 : print 'processing file n.', nfile

    f1 = open(base_dir+'/'+texfile,'r')
    l1 = f1.read().splitlines()
    
    # print texfile
    brokenfile = False
    for i in range(0,3): 
        try: del l1[0]
        except IndexError:
            print texfile, 'is broken'
            brokenfile = True
            break
            
    if brokenfile: continue

    l1_short = []

    for line in l1:
        l1_short.append(line)
        if '(13TeV)' in line: break

    name_all = []
    pull_all = []
    constrain_all = []
    contribution_all = []

    for line in l1_short:
        name, pull, constrain, contribution = line.split('&')
        name = str( TString(name).ReplaceAll('\\_',' ') )
        name = str( TString(name).ReplaceAll('$t\\bar{t}$','ttbar') )
        name = str( TString(name).ReplaceAll('$p_{T}$','pT') )
        name = str( TString(name).ReplaceAll('bar{t}','tbar') )
        contribution = str( TString(contribution).ReplaceAll('\\','') )
        contribution = str( TString(contribution).ReplaceAll('$','') )
        contribution = str( TString(contribution).ReplaceAll('{','') )
        contribution = str( TString(contribution).ReplaceAll('}','') )

        name_all.append(name)
        pull_all.append(float(pull))
        constrain_all.append(float(constrain))
        contribution_all.append(float(contribution))

        if 'm_{t}' in name:
            mass_pull = 172.5+float(pull)*3.
            h_mass.Fill(mass_pull)

        if '(13TeV)' in name:
            xsec_pull = 831.76+float(pull)
            h_xsec.Fill(xsec_pull)

print
print round(h_mass.GetMean(),2), round(h_mass.GetRMS(),2)
print round(h_xsec.GetMean(),1), round(h_xsec.GetRMS(),1), round(h_xsec.GetRMS()/h_xsec.GetMean()*100.,2), '%'
print

c = ROOT.TCanvas('c','c')
h_mass.Draw()
c.Print('mass.png','png')

c.Clear()
h_xsec.Draw()
c.Print('xsec.png','png')

rootfile = ROOT.TFile('toys.root','recreate')
h_mass.Write()
h_xsec.Write()
rootfile.Close()
