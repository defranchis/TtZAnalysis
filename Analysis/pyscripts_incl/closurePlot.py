import time
import os
import ROOT
from ROOT import *


gStyle.SetOptStat(0000)

f1 = open('xsecFit_tab13TeV.tex','r')
l1 = f1.read().splitlines()

for i in range(0,3): del l1[0]

l1_short = []

for line in l1:
    if '(13TeV)' in line: break
    l1_short.append(line)

name_mod = []
pull_mod = []
constrain_mod = []
contribution_mod = []


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
        name = str( TString(name).ReplaceAll('\\','#') )
        name = str( TString(name).ReplaceAll('#to','#rightarrow') )
        name = str( TString(name).ReplaceAll('K_s^0','Ks^{0}') )
        
    contribution = str( TString(contribution).ReplaceAll('\\','') )
    contribution = str( TString(contribution).ReplaceAll('$','') )
    contribution = str( TString(contribution).ReplaceAll('{','') )
    contribution = str( TString(contribution).ReplaceAll('}','') )
    if ('scale' in name and not 'Electron' in name and not 'Muon' in name) or 'GEN' in name or 'fragm' in name or 'match' in name or 'BR' in name or 'TT' in name or 'tune' in name or 'pT' in name or 'CR' in name or 'NLO' in name or 'mass' in name :
        if not ('tW' in name or 'DY' in name or 'JES' in name or 'b-tag' in name or 'BR' in name):
            name_mod.append(name)
            pull_mod.append(float(pull))
            constrain_mod.append(float(constrain))
            contribution_mod.append(float(contribution))


histo_mod = ROOT.TH1F('histo_mod','histo_mod',len(name_mod),-.5,-.5+len(name_mod))
contrib_mod = ROOT.TH1F('contrib_mod','contrib_mod',len(name_mod),-.5,-.5+len(name_mod))


for i in range(0,len(name_mod)):
    histo_mod.Fill(i,pull_mod[i])
    histo_mod.GetXaxis().SetBinLabel(i+1,name_mod[i])
    histo_mod.SetBinError(i+1,constrain_mod[i])
    contrib_mod.Fill(i,contribution_mod[i])
    contrib_mod.GetXaxis().SetBinLabel(i+1,name_mod[i])

if not os.path.exists('summaryPlots'):
    os.makedirs('summaryPlots')

c1 = ROOT.TCanvas('c1','c1')
c1.SetGrid()
c1.SetBottomMargin(.15)

line_up_mod = ROOT.TLine(histo_mod.GetBinLowEdge(1),1.,histo_mod.GetBinLowEdge(histo_mod.GetNbinsX())+histo_mod.GetBinWidth(1),1.)
line_down_mod = ROOT.TLine(histo_mod.GetBinLowEdge(1),-1.,histo_mod.GetBinLowEdge(histo_mod.GetNbinsX())+histo_mod.GetBinWidth(1),-1.)
line_up_mod.SetLineColor(kGreen+1)
line_down_mod.SetLineColor(kGreen+1)
line_up_mod.SetLineWidth(2)
line_down_mod.SetLineWidth(2)



histo_mod.SetTitle('')
histo_mod.SetMaximum(1.7)
histo_mod.SetMinimum(-1.7)
histo_mod.SetMarkerStyle(8)
histo_mod.SetMarkerColor(kRed)
histo_mod.SetLineWidth(2)
histo_mod.SetLabelSize(.055)
histo_mod.SetLabelOffset(.0115)
histo_mod.Draw('p')
line_up_mod.Draw('same')
line_down_mod.Draw('same')
histo_mod.Draw('psame')


c1.SaveAs('summaryPlots/mod_fitSummary.pdf','pdf')
c1.SaveAs('summaryPlots/mod_fitSummary.png','png')
c1.SaveAs('summaryPlots/mod_fitSummary.root','root')
c1.Clear()


