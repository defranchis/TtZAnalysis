import time
import os
import ROOT
from ROOT import *

gStyle.SetOptStat(1110)

base_dir = 'toys_workdir'
filelist = os.listdir(base_dir)

h_mass = ROOT.TH1F('h_mass','h_mass',700,160.,185.)

firstfile = True
nfile = 0

h_pull = []
h_constr = []
h_contrib = []

maxToys=-1

massFit = True
mttFit = True

if not mttFit:
    h_xsec= ROOT.TH1F('h_xsec','h_xsec',500,730,930)
else:
    h_xsec_1= ROOT.TH1F('h_xsec_1','h_xsec_1',250,200,300)
    h_xsec_2= ROOT.TH1F('h_xsec_2','h_xsec_2',250,280,380)
    h_xsec_3= ROOT.TH1F('h_xsec_3','h_xsec_3',250,150,250)
    h_xsec_4= ROOT.TH1F('h_xsec_4','h_xsec_4',250,0,100)

for texfile in filelist:
    if not texfile.endswith('.tex'): continue
    if massFit:
        if not texfile.startswith('xsecFit_tab_TOPMASS_'): continue #redundant
    else:
        if not texfile.startswith('xsecFit_tab13TeV_'): continue #redundant

    nfile += 1
    if nfile%1000 == 0 : print 'processing file n.', nfile
    if nfile > maxToys and maxToys > 0: break

    f1 = open(base_dir+'/'+texfile,'r')
    l1 = f1.read().splitlines()

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
        if not mttFit: 
            if '13TeV' in line: break
        else : 
            if '13TeV3' in line: break

    name_all = []
    pull_all = []
    constrain_all = []
    contribution_all = []

    for line in l1_short:
        name, pull, constrain, contribution = line.split('&')
        name = str( TString(name).ReplaceAll('\\_',' ') )
        name = str( TString(name).ReplaceAll('$t\\bar{t}$','ttbar') )
        name = str( TString(name).ReplaceAll('$p_{T}$','pT') )
        name = str( TString(name).ReplaceAll('$p_T$','pT') )
        name = str( TString(name).ReplaceAll('$\eta$','eta') )
        name = str( TString(name).ReplaceAll('bar{t}','tbar') )
        name = str( TString(name).ReplaceAll('Electron energy resolution','Electron ER') )
        name = str( TString(name).ReplaceAll('DY background','DY bg') )
        name = str( TString(name).ReplaceAll(' response','') )
        name = str( TString(name).ReplaceAll('PDF','PDF ') )
        name = str( TString(name).ReplaceAll('$m_{t}^{MC}$','top mass') )
        if 'b-tag' in name:
            name = str( TString(name).ReplaceAll('fragmentation','fragm.') )
            name = str( TString(name).ReplaceAll('correction','corr.') )
            name = str( TString(name).ReplaceAll('template','templ.') )
            name = str( TString(name).ReplaceAll('$D \to \mu X$','D to mu') )
            name = str( TString(name).ReplaceAll('splitting','split.') )
            name = str( TString(name).ReplaceAll('production','prod.') )
            name = str( TString(name).ReplaceAll('light to c','l/c') )
            name = str( TString(name).ReplaceAll('dependence','dep.') )
            name = str( TString(name).ReplaceAll('statistical','stat.') )
            name = str( TString(name).ReplaceAll('$','') )
            name = str( TString(name).ReplaceAll('\\','') )
            name = str( TString(name).ReplaceAll('K_s^0','Ks0') )

        contribution = str( TString(contribution).ReplaceAll('\\','') )
        contribution = str( TString(contribution).ReplaceAll('$','') )
        contribution = str( TString(contribution).ReplaceAll('{','') )
        contribution = str( TString(contribution).ReplaceAll('}','') )

        name_all.append(name)
        pull_all.append(float(pull))
        constrain_all.append(float(constrain))
        contribution_all.append(float(contribution))

        if 'mass' in name:
            mass_pull = 172.5+float(pull)*3.
            h_mass.Fill(mass_pull)

        if '13TeV' in name:
            xsec_pull = float(pull)
            if not mttFit: h_xsec.Fill(xsec_pull)
            else:
                if   '13TeV1' in name: h_xsec_1.Fill(250.29+xsec_pull)
                elif '13TeV2' in name: h_xsec_2.Fill(327.93+xsec_pull)
                elif '13TeV3' in name: h_xsec_3.Fill(197.98+xsec_pull)
                elif '13TeV4' in name: h_xsec_4.Fill(55.55+xsec_pull)

    if firstfile:
        firstfile = False
        for name in name_all:
            index = name_all.index(name)
            h_pull.append(ROOT.TH1F(name+'_pull',name+'_pull',700,-5,5))
            h_constr.append(ROOT.TH1F(name+'_constr',name+'_constr',300,0,3))
            h_contrib.append(ROOT.TH1F(name+'_contrib',name+'_contrib',300,-1,1))


    for name in name_all:
        index = name_all.index(name)
        h_pull[index].Fill(pull_all[index])
        h_constr[index].Fill(constrain_all[index])
        h_contrib[index].Fill(contribution_all[index])

print
print round(h_mass.GetMean(),2), round(h_mass.GetRMS(),2)
if not mttFit: print round(h_xsec.GetMean(),1), round(h_xsec.GetRMS(),1), round(h_xsec.GetRMS()/h_xsec.GetMean()*100.,2), '%'
else:
    print round(h_xsec_1.GetMean(),1), round(h_xsec_1.GetRMS(),1), round(h_xsec_1.GetRMS()/h_xsec_1.GetMean()*100.,2), '%'
    print round(h_xsec_2.GetMean(),1), round(h_xsec_2.GetRMS(),1), round(h_xsec_2.GetRMS()/h_xsec_2.GetMean()*100.,2), '%'
    print round(h_xsec_3.GetMean(),1), round(h_xsec_3.GetRMS(),1), round(h_xsec_3.GetRMS()/h_xsec_3.GetMean()*100.,2), '%'
print

c = ROOT.TCanvas('c','c')
h_mass.SetTitle('effect of MC stats on top MC mass;m_{t}^{MC} [GeV];a.u.')
h_mass.DrawNormalized()
c.Print('mass.png','png')

if not mttFit:
    c.Clear()
    h_xsec.SetTitle('effect of MC stats on ttbar cross section;#sigma_{t#bar{t}} [pb];a.u.')
    h_xsec.DrawNormalized()
    c.Print('xsec.png','png')

else:
    c.Clear()
    h_xsec_1.SetTitle('effect of MC stats on ttbar cross section;#sigma_{t#bar{t}} mtt1 [pb];a.u.')
    h_xsec_1.DrawNormalized()
    c.Print('xsec_1.png','png')

    c.Clear()
    h_xsec_2.SetTitle('effect of MC stats on ttbar cross section;#sigma_{t#bar{t}} mtt2 [pb];a.u.')
    h_xsec_2.DrawNormalized()
    c.Print('xsec_2.png','png')

    c.Clear()
    h_xsec_3.SetTitle('effect of MC stats on ttbar cross section;#sigma_{t#bar{t}} mtt3 [pb];a.u.')
    h_xsec_3.DrawNormalized()
    c.Print('xsec_3.png','png')


rootfile = ROOT.TFile('toys.root','recreate')
h_mass.Write()
if not mttFit: h_xsec.Write()
else:
    h_xsec_1.Write()
    h_xsec_2.Write()
    h_xsec_3.Write()


for h in h_pull: h.Write()
for h in h_constr: h.Write()
for h in h_contrib: h.Write()

rootfile.Close()

print
print nfile, 'toys analyzed'
print
