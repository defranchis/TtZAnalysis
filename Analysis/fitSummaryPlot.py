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
    if 'Xsec (13TeV)' in line: break
    l1_short.append(line)

name_all = []
pull_all = []
constrain_all = []
contribution_all = []

name_pdf = []
pull_pdf = []
constrain_pdf = []
contribution_pdf = []

name_jes = []
pull_jes = []
constrain_jes = []
contribution_jes = []


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
    if 'PDF' in name:
        name_pdf.append(name)
        pull_pdf.append(float(pull))
        constrain_pdf.append(float(constrain))
        contribution_pdf.append(float(contribution))
    elif 'JES' in name:
        name_jes.append(name)
        pull_jes.append(float(pull))
        constrain_jes.append(float(constrain))
        contribution_jes.append(float(contribution))
    else:
        name_all.append(name)
        pull_all.append(float(pull))
        constrain_all.append(float(constrain))
        contribution_all.append(float(contribution))


histo_all = ROOT.TH1F('histo_all','histo_all',len(name_all),-.5,-.5+len(name_all))
histo_pdf = ROOT.TH1F('histo_pdf','histo_pdf',len(name_pdf),-.5,-.5+len(name_pdf))
histo_jes = ROOT.TH1F('histo_jes','histo_jes',len(name_jes),-.5,-.5+len(name_jes))

contrib_all = ROOT.TH1F('contrib_all','contrib_all',len(name_all),-.5,-.5+len(name_all))
contrib_pdf = ROOT.TH1F('contrib_pdf','contrib_pdf',len(name_pdf),-.5,-.5+len(name_pdf))
contrib_jes = ROOT.TH1F('contrib_jes','contrib_jes',len(name_jes),-.5,-.5+len(name_jes))


for i in range(0,len(name_all)):
    histo_all.Fill(i,pull_all[i])
    histo_all.GetXaxis().SetBinLabel(i+1,name_all[i])
    histo_all.SetBinError(i+1,constrain_all[i])
    contrib_all.Fill(i,contribution_all[i])
    contrib_all.GetXaxis().SetBinLabel(i+1,name_all[i])

for i in range(0,len(name_pdf)):
    histo_pdf.Fill(i,pull_pdf[i])
    histo_pdf.GetXaxis().SetBinLabel(i+1,name_pdf[i])
    histo_pdf.SetBinError(i+1,constrain_pdf[i])
    contrib_pdf.Fill(i,contribution_pdf[i])
    contrib_pdf.GetXaxis().SetBinLabel(i+1,name_pdf[i])

for i in range(0,len(name_jes)):
    histo_jes.Fill(i,pull_jes[i])
    histo_jes.GetXaxis().SetBinLabel(i+1,name_jes[i])
    histo_jes.SetBinError(i+1,constrain_jes[i])
    contrib_jes.Fill(i,contribution_jes[i])
    contrib_jes.GetXaxis().SetBinLabel(i+1,name_jes[i])

if not os.path.exists('summaryPlots'):
    os.makedirs('summaryPlots')

c1 = ROOT.TCanvas('c1','c1')
c1.SetGrid()

line_up_all = ROOT.TLine(histo_all.GetBinLowEdge(1),1.,histo_all.GetBinLowEdge(histo_all.GetNbinsX())+histo_all.GetBinWidth(1),1.)
line_down_all = ROOT.TLine(histo_all.GetBinLowEdge(1),-1.,histo_all.GetBinLowEdge(histo_all.GetNbinsX())+histo_all.GetBinWidth(1),-1.)
line_up_all.SetLineColor(kGreen+1)
line_down_all.SetLineColor(kGreen+1)
line_up_all.SetLineWidth(2)
line_down_all.SetLineWidth(2)

line_up_pdf = ROOT.TLine(histo_pdf.GetBinLowEdge(1),1.,histo_pdf.GetBinLowEdge(histo_pdf.GetNbinsX())+histo_pdf.GetBinWidth(1),1.)
line_down_pdf = ROOT.TLine(histo_pdf.GetBinLowEdge(1),-1.,histo_pdf.GetBinLowEdge(histo_pdf.GetNbinsX())+histo_pdf.GetBinWidth(1),-1.)
line_up_pdf.SetLineColor(kGreen+1)
line_down_pdf.SetLineColor(kGreen+1)
line_up_pdf.SetLineWidth(2)
line_down_pdf.SetLineWidth(2)

line_up_jes = ROOT.TLine(histo_jes.GetBinLowEdge(1),1.,histo_jes.GetBinLowEdge(histo_jes.GetNbinsX())+histo_jes.GetBinWidth(1),1.)
line_down_jes = ROOT.TLine(histo_jes.GetBinLowEdge(1),-1.,histo_jes.GetBinLowEdge(histo_jes.GetNbinsX())+histo_jes.GetBinWidth(1),-1.)
line_up_jes.SetLineColor(kGreen+1)
line_down_jes.SetLineColor(kGreen+1)
line_up_jes.SetLineWidth(2)
line_down_jes.SetLineWidth(2)


histo_all.SetTitle('fit summary')
histo_all.SetMaximum(3.5)
histo_all.SetMinimum(-3.5)
histo_all.SetMarkerStyle(8)
histo_all.SetMarkerColor(kRed)
histo_all.SetLineWidth(2)
histo_all.Draw('p')
line_up_all.Draw('same')
line_down_all.Draw('same')
histo_all.Draw('psame')

c1.SaveAs('summaryPlots/all_fitSummary.pdf','pdf')
c1.SaveAs('summaryPlots/all_fitSummary.png','png')
c1.Clear()

###

histo_pdf.SetTitle('fit summary - PDFs')
histo_pdf.SetMaximum(1.3)
histo_pdf.SetMinimum(-1.3)
histo_pdf.SetMarkerStyle(8)
histo_pdf.SetMarkerColor(kRed)
histo_pdf.SetLineWidth(2)
histo_pdf.Draw('p')
line_up_pdf.Draw('same')
line_down_pdf.Draw('same')
histo_pdf.Draw('psame')


c1.SaveAs('summaryPlots/pdf_fitSummary.pdf','pdf')
c1.SaveAs('summaryPlots/pdf_fitSummary.png','png')
c1.Clear()

###

histo_jes.SetTitle('fit summary - JES')
histo_jes.SetMaximum(1.7)
histo_jes.SetMinimum(-1.7)
histo_jes.SetMarkerStyle(8)
histo_jes.SetMarkerColor(kRed)
histo_jes.SetLineWidth(2)
histo_jes.Draw('p')
line_up_jes.Draw('same')
line_down_jes.Draw('same')
histo_jes.Draw('psame')


c1.SaveAs('summaryPlots/jes_fitSummary.pdf','pdf')
c1.SaveAs('summaryPlots/jes_fitSummary.png','png')
c1.Clear()


###################################

contrib_all.SetTitle('contribution')
contrib_all.SetFillColor(kYellow)
contrib_all.SetLineColor(kRed)
contrib_all.Draw('hist')

c1.SaveAs('summaryPlots/all_contribution.pdf','pdf')
c1.SaveAs('summaryPlots/all_contribution.png','png')
c1.Clear()

###

contrib_pdf.SetTitle('contribution -PDFs')
contrib_pdf.SetFillColor(kYellow)
contrib_pdf.SetLineColor(kRed)
contrib_pdf.Draw('hist')

c1.SaveAs('summaryPlots/pdf_contribution.pdf','pdf')
c1.SaveAs('summaryPlots/pdf_contribution.png','png')
c1.Clear()

###

contrib_jes.SetTitle('contribution -JES')
contrib_jes.SetFillColor(kYellow)
contrib_jes.SetLineColor(kRed)
contrib_jes.Draw('hist')

c1.SaveAs('summaryPlots/jes_contribution.pdf','pdf')
c1.SaveAs('summaryPlots/jes_contribution.png','png')
c1.Clear()
