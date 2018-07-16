import time
import os
import ROOT
from ROOT import *


gStyle.SetOptStat(0000)
gStyle.SetErrorX(0)

f1 = open('xsecFit_tab13TeV.tex','r')
l1 = f1.read().splitlines()

inFile = ROOT.TFile('toys.root','READ')

# h = inFile.FindObjectAny('top mass _pull')
# print h.GetMean(), h.GetRMS()

for i in range(0,3): del l1[0]

l1_short = []

for line in l1:
    if '(13TeV)' in line: break
    l1_short.append(line)

name_all = []
pull_all = []
constraint_all = []
contribution_all = []
stat_toys_all = []
central_toys_all = []

name_pdf = []
pull_pdf = []
constraint_pdf = []
contribution_pdf = []
stat_toys_pdf = []
central_toys_pdf = []

name_jes = []
pull_jes = []
constraint_jes = []
contribution_jes = []
stat_toys_jes = []
central_toys_jes = []

name_btag = []
pull_btag = []
constraint_btag = []
contribution_btag = []
stat_toys_btag = []
central_toys_btag = []

name_mod = []
pull_mod = []
constraint_mod = []
contribution_mod = []
stat_toys_mod = []
central_toys_mod = []

latexLabel1 = ROOT.TLatex()
latexLabel1.SetTextSize(0.06)
latexLabel1.SetNDC()

latexLabel2 = ROOT.TLatex()
latexLabel2.SetTextSize(0.04)
latexLabel2.SetTextFont(42)
latexLabel2.SetNDC()

latexLabel3 = ROOT.TLatex()
latexLabel3.SetTextSize(0.04)
# latexLabel3.SetTextFont(42)
latexLabel3.SetNDC()


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
    if 'mass' in name: continue
    tmpname = TString(name);
    tmpname.ReplaceAll('#','')
    tmpname.ReplaceAll('rightarrow','to')
    tmpname.ReplaceAll('Ks^{0}','Ks0')
    tmpname=str(tmpname)

    h = inFile.FindObjectAny(tmpname+'_pull')        
    if 'PDF' in name:
        name_pdf.append(name)
        pull_pdf.append(float(pull))
        constraint_pdf.append(float(constrain))
        contribution_pdf.append(float(contribution))
        stat_toys_pdf.append(h.GetRMS())
        central_toys_pdf.append(h.GetMean())
    elif 'b-tag' in name or 'mistag' in name:
        name_btag.append(name)
        pull_btag.append(float(pull))
        constraint_btag.append(float(constrain))
        contribution_btag.append(float(contribution))
        stat_toys_btag.append(h.GetRMS())
        central_toys_btag.append(h.GetMean())
    elif 'JES' in name:
        name_jes.append(name)
        pull_jes.append(float(pull))
        constraint_jes.append(float(constrain))
        contribution_jes.append(float(contribution))
        stat_toys_jes.append(h.GetRMS())
        central_toys_jes.append(h.GetMean())
    elif ('scale' in name and not 'Electron' in name and not 'Muon' in name) or 'GEN' in name or 'fragm' in name or 'match' in name or 'BR' in name or 'TT' in name or 'tune' in name or 'pT' in name or 'CR' in name or 'NLO' in name or 'm_{t}' in name :
        name_mod.append(name)
        pull_mod.append(float(pull))
        constraint_mod.append(float(constrain))
        contribution_mod.append(float(contribution))
        stat_toys_mod.append(h.GetRMS())
        central_toys_mod.append(h.GetMean())
    else:
        name_all.append(name)
        pull_all.append(float(pull))
        constraint_all.append(float(constrain))
        contribution_all.append(float(contribution))
        stat_toys_all.append(h.GetRMS())
        central_toys_all.append(h.GetMean())

histo_all = ROOT.TH1F('histo_all','histo_all',len(name_all),-.5,-.5+len(name_all))
histo_pdf = ROOT.TH1F('histo_pdf','histo_pdf',len(name_pdf),-.5,-.5+len(name_pdf))
histo_jes = ROOT.TH1F('histo_jes','histo_jes',len(name_jes),-.5,-.5+len(name_jes))
histo_btag = ROOT.TH1F('histo_btag','histo_btag',len(name_btag),-.5,-.5+len(name_btag))
histo_mod = ROOT.TH1F('histo_mod','histo_mod',len(name_mod),-.5,-.5+len(name_mod))

histo_stat_all = ROOT.TH1F('histo_stat_all','histo_stat_all',len(name_all),-.5,-.5+len(name_all))
histo_stat_pdf = ROOT.TH1F('histo_stat_pdf','histo_stat_pdf',len(name_pdf),-.5,-.5+len(name_pdf))
histo_stat_jes = ROOT.TH1F('histo_stat_jes','histo_stat_jes',len(name_jes),-.5,-.5+len(name_jes))
histo_stat_btag = ROOT.TH1F('histo_stat_btag','histo_stat_btag',len(name_btag),-.5,-.5+len(name_btag))
histo_stat_mod = ROOT.TH1F('histo_stat_mod','histo_stat_mod',len(name_mod),-.5,-.5+len(name_mod))

histo_central_all = ROOT.TH1F('histo_central_all','histo_central_all',len(name_all),-.5,-.5+len(name_all))
histo_central_pdf = ROOT.TH1F('histo_central_pdf','histo_central_pdf',len(name_pdf),-.5,-.5+len(name_pdf))
histo_central_jes = ROOT.TH1F('histo_central_jes','histo_central_jes',len(name_jes),-.5,-.5+len(name_jes))
histo_central_btag = ROOT.TH1F('histo_central_btag','histo_central_btag',len(name_btag),-.5,-.5+len(name_btag))
histo_central_mod = ROOT.TH1F('histo_central_mod','histo_central_mod',len(name_mod),-.5,-.5+len(name_mod))


for i in range(0,len(name_all)):
    histo_all.Fill(i,pull_all[i])
    histo_all.GetXaxis().SetBinLabel(i+1,name_all[i])
    histo_all.SetBinError(i+1,constraint_all[i])

    histo_stat_all.Fill(i,pull_all[i])
    histo_stat_all.GetXaxis().SetBinLabel(i+1,name_all[i])
    histo_stat_all.SetBinError(i+1,(constraint_all[i]**2+stat_toys_all[i]**2)**.5)

    histo_central_all.Fill(i,central_toys_all[i])
    histo_central_all.GetXaxis().SetBinLabel(i+1,name_all[i])
    histo_central_all.SetBinError(i+1,0.)


for i in range(0,len(name_pdf)):
    histo_pdf.Fill(i,pull_pdf[i])
    histo_pdf.GetXaxis().SetBinLabel(i+1,name_pdf[i])
    histo_pdf.SetBinError(i+1,constraint_pdf[i])

    histo_stat_pdf.Fill(i,pull_pdf[i])
    histo_stat_pdf.GetXaxis().SetBinLabel(i+1,name_pdf[i])
    histo_stat_pdf.SetBinError(i+1,(constraint_pdf[i]**2+stat_toys_pdf[i]**2)**.5)

    histo_central_pdf.Fill(i,central_toys_pdf[i])
    histo_central_pdf.GetXaxis().SetBinLabel(i+1,name_pdf[i])
    histo_central_pdf.SetBinError(i+1,0.)


for i in range(0,len(name_jes)):
    histo_jes.Fill(i,pull_jes[i])
    histo_jes.GetXaxis().SetBinLabel(i+1,name_jes[i])
    histo_jes.SetBinError(i+1,constraint_jes[i])

    histo_stat_jes.Fill(i,pull_jes[i])
    histo_stat_jes.GetXaxis().SetBinLabel(i+1,name_jes[i])
    histo_stat_jes.SetBinError(i+1,(constraint_jes[i]**2+stat_toys_jes[i]**2)**.5)

    histo_central_jes.Fill(i,central_toys_jes[i])
    histo_central_jes.GetXaxis().SetBinLabel(i+1,name_jes[i])
    histo_central_jes.SetBinError(i+1,0.)


for i in range(0,len(name_btag)):
    histo_btag.Fill(i,pull_btag[i])
    histo_btag.GetXaxis().SetBinLabel(i+1,name_btag[i])
    histo_btag.SetBinError(i+1,constraint_btag[i])

    histo_stat_btag.Fill(i,pull_btag[i])
    histo_stat_btag.GetXaxis().SetBinLabel(i+1,name_btag[i])
    histo_stat_btag.SetBinError(i+1,(constraint_btag[i]**2+stat_toys_btag[i]**2)**.5)

    histo_central_btag.Fill(i,central_toys_btag[i])
    histo_central_btag.GetXaxis().SetBinLabel(i+1,name_btag[i])
    histo_central_btag.SetBinError(i+1,0.)


for i in range(0,len(name_mod)):
    histo_mod.Fill(i,pull_mod[i])
    histo_mod.GetXaxis().SetBinLabel(i+1,name_mod[i])
    histo_mod.SetBinError(i+1,constraint_mod[i])

    histo_stat_mod.Fill(i,pull_mod[i])
    histo_stat_mod.GetXaxis().SetBinLabel(i+1,name_mod[i])
    histo_stat_mod.SetBinError(i+1,(constraint_mod[i]**2+stat_toys_mod[i]**2)**.5)

    histo_central_mod.Fill(i,central_toys_mod[i])
    histo_central_mod.GetXaxis().SetBinLabel(i+1,name_mod[i])
    histo_central_mod.SetBinError(i+1,0.)

if not os.path.exists('summaryPlotsToys'):
    os.makedirs('summaryPlotsToys')

c1 = ROOT.TCanvas('c1','c1')
c1.SetGrid()

line_up_all = ROOT.TLine(histo_all.GetBinLowEdge(1),1.,histo_all.GetBinLowEdge(histo_all.GetNbinsX())+histo_all.GetBinWidth(1),1.)
line_down_all = ROOT.TLine(histo_all.GetBinLowEdge(1),-1.,histo_all.GetBinLowEdge(histo_all.GetNbinsX())+histo_all.GetBinWidth(1),-1.)
line_up_all.SetLineColor(kGreen+2)
line_down_all.SetLineColor(kGreen+2)
line_up_all.SetLineWidth(2)
line_down_all.SetLineWidth(2)

line_up_pdf = ROOT.TLine(histo_pdf.GetBinLowEdge(1),1.,histo_pdf.GetBinLowEdge(histo_pdf.GetNbinsX())+histo_pdf.GetBinWidth(1),1.)
line_down_pdf = ROOT.TLine(histo_pdf.GetBinLowEdge(1),-1.,histo_pdf.GetBinLowEdge(histo_pdf.GetNbinsX())+histo_pdf.GetBinWidth(1),-1.)
line_up_pdf.SetLineColor(kGreen+2)
line_down_pdf.SetLineColor(kGreen+2)
line_up_pdf.SetLineWidth(2)
line_down_pdf.SetLineWidth(2)

line_up_jes = ROOT.TLine(histo_jes.GetBinLowEdge(1),1.,histo_jes.GetBinLowEdge(histo_jes.GetNbinsX())+histo_jes.GetBinWidth(1),1.)
line_down_jes = ROOT.TLine(histo_jes.GetBinLowEdge(1),-1.,histo_jes.GetBinLowEdge(histo_jes.GetNbinsX())+histo_jes.GetBinWidth(1),-1.)
line_up_jes.SetLineColor(kGreen+2)
line_down_jes.SetLineColor(kGreen+2)
line_up_jes.SetLineWidth(2)
line_down_jes.SetLineWidth(2)

line_up_btag = ROOT.TLine(histo_btag.GetBinLowEdge(1),1.,histo_btag.GetBinLowEdge(histo_btag.GetNbinsX())+histo_btag.GetBinWidth(1),1.)
line_down_btag = ROOT.TLine(histo_btag.GetBinLowEdge(1),-1.,histo_btag.GetBinLowEdge(histo_btag.GetNbinsX())+histo_btag.GetBinWidth(1),-1.)
line_up_btag.SetLineColor(kGreen+2)
line_down_btag.SetLineColor(kGreen+2)
line_up_btag.SetLineWidth(2)
line_down_btag.SetLineWidth(2)

line_up_mod = ROOT.TLine(histo_mod.GetBinLowEdge(1),1.,histo_mod.GetBinLowEdge(histo_mod.GetNbinsX())+histo_mod.GetBinWidth(1),1.)
line_down_mod = ROOT.TLine(histo_mod.GetBinLowEdge(1),-1.,histo_mod.GetBinLowEdge(histo_mod.GetNbinsX())+histo_mod.GetBinWidth(1),-1.)
line_up_mod.SetLineColor(kGreen+2)
line_down_mod.SetLineColor(kGreen+2)
line_up_mod.SetLineWidth(2)
line_down_mod.SetLineWidth(2)


histo_all.SetTitle('')
histo_all.GetYaxis().SetTitle('normalized pull')
histo_all.SetMaximum(2.5)
histo_all.SetMinimum(-3.0)
histo_all.SetMarkerStyle(8)
histo_all.SetMarkerColor(kBlue+1)
histo_all.SetLineWidth(2)
histo_all.SetLineColor(kBlue+1)

histo_stat_all.SetTitle('')
histo_stat_all.GetYaxis().SetTitle('normalized pull')
histo_stat_all.SetMaximum(2.5)
histo_stat_all.SetMinimum(-3.0)
histo_stat_all.SetMarkerStyle(8)
histo_stat_all.SetMarkerColor(kBlue+1)
histo_stat_all.SetLineWidth(2)
histo_stat_all.SetLineColor(kRed)

histo_central_all.SetTitle('')
histo_central_all.GetYaxis().SetTitle('normalized pull')
histo_central_all.SetMaximum(2.5)
histo_central_all.SetMinimum(-3.0)
histo_central_all.SetMarkerStyle(1)
histo_central_all.SetMarkerColor(kGreen+1)
histo_central_all.SetLineWidth(2)
histo_central_all.SetLineColor(kRed)

histo_central_all.Draw('p')
histo_stat_all.Draw('psameE1')
histo_all.Draw('psameE1')
line_up_all.Draw('same')
line_down_all.Draw('same')
histo_all.Draw('psameE1')

latexLabel1.DrawLatex(0.11, 0.92, "CMS")
latexLabel2.DrawLatex(0.70, 0.92, "35.9 fb^{-1} (13 TeV)")
latexLabel2.DrawLatex(0.59, 0.84, "experimental uncertainties")

c1.SaveAs('summaryPlotsToys/all_fitSummary.pdf','pdf')
c1.SaveAs('summaryPlotsToys/all_fitSummary.png','png')
c1.Clear()

###

histo_pdf.SetTitle('')
histo_pdf.SetMaximum(1.3)
histo_pdf.SetMinimum(-1.3)
histo_pdf.SetMarkerStyle(8)
histo_pdf.SetMarkerColor(kBlue+1)
histo_pdf.SetLineWidth(2)
histo_pdf.SetLineColor(kBlue+1)

histo_stat_pdf.SetTitle('')
histo_stat_pdf.SetMaximum(1.3)
histo_stat_pdf.SetMinimum(-1.3)
histo_stat_pdf.SetMarkerStyle(8)
histo_stat_pdf.SetMarkerColor(kBlue+1)
histo_stat_pdf.SetLineWidth(2)
histo_stat_pdf.SetLineColor(kRed)

histo_central_pdf.SetTitle('')
histo_central_pdf.SetMaximum(1.3)
histo_central_pdf.SetMinimum(-1.3)
histo_central_pdf.SetMarkerStyle(1)
histo_central_pdf.SetMarkerColor(kGreen+1)
histo_central_pdf.SetLineWidth(2)
histo_central_pdf.SetLineColor(kRed)

histo_central_pdf.Draw('p')
histo_stat_pdf.Draw('psameE1')
histo_pdf.Draw('psameE1')
line_up_pdf.Draw('same')
line_down_pdf.Draw('same')
histo_pdf.Draw('psameE1')

latexLabel1.DrawLatex(0.11, 0.92, "CMS")
latexLabel2.DrawLatex(0.70, 0.92, "35.9 fb^{-1} (13 TeV)")
latexLabel2.DrawLatex(0.68, 0.84, "PDF uncertainties")


c1.SaveAs('summaryPlotsToys/pdf_fitSummary.pdf','pdf')
c1.SaveAs('summaryPlotsToys/pdf_fitSummary.png','png')
c1.Clear()

###

histo_jes.SetTitle('')
histo_jes.SetMaximum(2.1)
histo_jes.SetMinimum(-2.8)
histo_jes.SetMarkerStyle(8)
histo_jes.SetMarkerColor(kBlue+1)
histo_jes.SetLineWidth(2)
histo_jes.SetLineColor(kBlue+1)

histo_stat_jes.SetTitle('')
histo_stat_jes.SetMaximum(2.1)
histo_stat_jes.SetMinimum(-2.8)
histo_stat_jes.SetMarkerStyle(8)
histo_stat_jes.SetMarkerColor(kBlue+1)
histo_stat_jes.SetLineWidth(2)
histo_stat_jes.SetLineColor(kRed)

histo_central_jes.SetTitle('')
histo_central_jes.SetMaximum(2.1)
histo_central_jes.SetMinimum(-2.8)
histo_central_jes.SetMarkerStyle(1)
histo_central_jes.SetMarkerColor(kGreen+1)
histo_central_jes.SetLineWidth(2)
histo_central_jes.SetLineColor(kRed)

histo_central_jes.Draw('p')
histo_stat_jes.Draw('psameE1')
histo_jes.Draw('psameE1')
line_up_jes.Draw('same')
line_down_jes.Draw('same')
histo_jes.Draw('psameE1')

latexLabel1.DrawLatex(0.11, 0.92, "CMS")
latexLabel2.DrawLatex(0.70, 0.92, "35.9 fb^{-1} (13 TeV)")
latexLabel2.DrawLatex(0.69, 0.82, "JES uncertainties")


c1.SaveAs('summaryPlotsToys/jes_fitSummary.pdf','pdf')
c1.SaveAs('summaryPlotsToys/jes_fitSummary.png','png')
c1.Clear()

###

histo_btag.SetTitle('')
histo_btag.SetMaximum(1.9)
histo_btag.SetMinimum(-1.3)
histo_btag.SetMarkerStyle(8)
histo_btag.SetMarkerColor(kBlue+1)
histo_btag.SetLineWidth(2)
histo_btag.SetLineColor(kBlue+1)

histo_stat_btag.SetTitle('')
histo_stat_btag.SetMaximum(1.9)
histo_stat_btag.SetMinimum(-1.3)
histo_stat_btag.SetMarkerStyle(8)
histo_stat_btag.SetMarkerColor(kBlue+1)
histo_stat_btag.SetLineWidth(2)
histo_stat_btag.SetLineColor(kRed)

histo_central_btag.SetTitle('')
histo_central_btag.SetMaximum(1.9)
histo_central_btag.SetMinimum(-1.3)
histo_central_btag.SetMarkerStyle(1)
histo_central_btag.SetMarkerColor(kGreen+1)
histo_central_btag.SetLineWidth(2)
histo_central_btag.SetLineColor(kRed)

histo_central_btag.Draw('p')
histo_stat_btag.Draw('psameE1')
histo_btag.Draw('psameE1')
line_up_btag.Draw('same')
line_down_btag.Draw('same')
histo_btag.Draw('psameE1')

latexLabel1.DrawLatex(0.11, 0.92, "CMS")
latexLabel2.DrawLatex(0.70, 0.92, "35.9 fb^{-1} (13 TeV)")
latexLabel2.DrawLatex(0.13, 0.83, "b-tagging uncertainties")


c1.SaveAs('summaryPlotsToys/btag_fitSummary.pdf','pdf')
c1.SaveAs('summaryPlotsToys/btag_fitSummary.png','png')
c1.Clear()

###

histo_mod.SetTitle('')
histo_mod.SetMaximum(1.7)
histo_mod.SetMinimum(-2.3)
histo_mod.SetMarkerStyle(8)
histo_mod.SetMarkerColor(kBlue+1)
histo_mod.SetLineWidth(2)
histo_mod.SetLineColor(kBlue+1)

histo_stat_mod.SetTitle('')
histo_stat_mod.SetMaximum(1.7)
histo_stat_mod.SetMinimum(-2.3)
histo_stat_mod.SetMarkerStyle(8)
histo_stat_mod.SetMarkerColor(kBlue+1)
histo_stat_mod.SetLineWidth(2)
histo_stat_mod.SetLineColor(kRed)

histo_central_mod.SetTitle('')
histo_central_mod.SetMaximum(1.7)
histo_central_mod.SetMinimum(-2.3)
histo_central_mod.SetMarkerStyle(1)
histo_central_mod.SetMarkerColor(kGreen+1)
histo_central_mod.SetLineWidth(2)
histo_central_mod.SetLineColor(kRed)

histo_central_mod.Draw('p')
histo_stat_mod.Draw('psameE1')
histo_mod.Draw('psameE1')
line_up_mod.Draw('same')
line_down_mod.Draw('same')
histo_mod.Draw('psameE1')

latexLabel1.DrawLatex(0.11, 0.92, "CMS")
latexLabel2.DrawLatex(0.70, 0.92, "35.9 fb^{-1} (13 TeV)")
latexLabel2.DrawLatex(0.62, 0.83, "modelling uncertainties")


c1.SaveAs('summaryPlotsToys/mod_fitSummary.pdf','pdf')
c1.SaveAs('summaryPlotsToys/mod_fitSummary.png','png')
c1.Clear()
