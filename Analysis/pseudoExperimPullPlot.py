import ROOT
from ROOT import *

gStyle.SetOptFit(111)

inFile_norm = ROOT.TFile('xsecFit_pull_13TeV.root')
inFile_abs  = ROOT.TFile('xsecFit_pull_abs.root')

h_norm = inFile_norm.Get('pull')
h_abs  = inFile_abs.Get('pull')

c_norm = ROOT.TCanvas('c_norm','c_norm')
f_norm = ROOT.TF1('norm','gaus(0)',-8,8)
h_norm.Fit(f_norm,'','',-1*h_norm.GetRMS(),h_norm.GetRMS())
c_norm.SaveAs('pull_norm.png')

c_abs = ROOT.TCanvas('c_abs','c_abs')
f_abs  = ROOT.TF1('abs','gaus(0)',-1,1)
h_abs.Fit(f_abs,'','',-1*h_abs.GetRMS(),h_abs.GetRMS())
c_abs.SaveAs('pull_abs.png')

print
print

if f_abs.GetParameter(1) > 0 : print 'm_out = m_in +', round(f_abs.GetParameter(1),2), '+\-', round(f_abs.GetParameter(2),2), 'GeV'
else: print 'm_out = m_in -', abs(round(f_abs.GetParameter(1),2)), '+\-', round(f_abs.GetParameter(2),2), 'GeV'

print

print 'correction factor =', round(f_norm.GetParameter(2),2)

print
print

