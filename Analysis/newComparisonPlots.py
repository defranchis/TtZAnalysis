import sys
import os
import time
import ROOT
from ROOT import *

gStyle.SetOptStat(0000)

if len(sys.argv) < 2:
    print
    print 'ERROR: please provide input directories!'
    print 'example: python makeComparisonPlots.py dir1 dir2'
    print
    quit()

if sys.argv[1] == sys.argv[2]:
    print
    print 'ERROR: directories to compare must be different!'
    print
    quit()

dir1 = sys.argv[1]
dir2 = sys.argv[2]

print
print 'making comparison plots for', dir1, 'and', dir2, '...'
print


f1 = open(dir1+'/xsecFit_tab13TeV.tex','r')
f2 = open(dir2+'/xsecFit_tab13TeV.tex','r')

dir1 = str(TString(dir1).ReplaceAll('/',''))
dir1 = str(TString(dir1).ReplaceAll('..',''))
dir2 = str(TString(dir2).ReplaceAll('/',''))
dir2 = str(TString(dir2).ReplaceAll('..',''))

l1 = f1.read().splitlines()
l2 = f2.read().splitlines()

for i in range(0,3):
    del l1[0]
    del l2[0]

l1_short = []
l2_short = []

for line in l1:
    if 'Xsec (13TeV)' in line:
        break
    l1_short.append(line)

for line in l2:
    if 'Xsec (13TeV)' in line:
        break
    l2_short.append(line)


if len(l1_short) != len(l2_short):
    print 'ERROR: input files must contain the same amount of systematics!'
    print
    quit()


name1_all = []
pull1_all = []
constraint1_all = []
contribution1_all = []

name1_jes = []
pull1_jes = []
constraint1_jes = []
contribution1_jes = []

name1_pdf = []
pull1_pdf = []
constraint1_pdf = []
contribution1_pdf = []

###

name2_all = []
pull2_all = []
constraint2_all = []
contribution2_all = []

name2_jes = []
pull2_jes = []
constraint2_jes = []
contribution2_jes = []

name2_pdf = []
pull2_pdf = []
constraint2_pdf = []
contribution2_pdf = []


for line in l1_short:
    name, pull, constraint, contribution = line.split('&')

    name = str( TString(name).ReplaceAll('\\_',' ') )
    name = str( TString(name).ReplaceAll('$t\\bar{t}$','ttbar') )
    name = str( TString(name).ReplaceAll('$p_{T}$','pT') )
    name = str( TString(name).ReplaceAll('bar{t}','tbar') )
    contribution = str( TString(contribution).ReplaceAll('\\','') )
    contribution = str( TString(contribution).ReplaceAll('$','') )
    contribution = str( TString(contribution).ReplaceAll('{','') )
    contribution = str( TString(contribution).ReplaceAll('}','') )
    if 'PDF' in name:
        name1_pdf.append(name)
        pull1_pdf.append(float(pull))
        constraint1_pdf.append(float(constraint))
        contribution1_pdf.append(float(contribution))
    elif 'JES' in name:
        name1_jes.append(name)
        pull1_jes.append(float(pull))
        constraint1_jes.append(float(constraint))
        contribution1_jes.append(float(contribution))
    else:  
        name1_all.append(name)
        pull1_all.append(float(pull))
        constraint1_all.append(float(constraint))
        contribution1_all.append(float(contribution))

for line in l2_short:
    name, pull, constraint, contribution = line.split('&')

    name = str( TString(name).ReplaceAll('\\_',' ') )
    name = str( TString(name).ReplaceAll('$t\\bar{t}$','ttbar') )
    name = str( TString(name).ReplaceAll('$p_{T}$','pT') )
    name = str( TString(name).ReplaceAll('bar{t}','tbar') )
    contribution = str( TString(contribution).ReplaceAll('\\','') )
    contribution = str( TString(contribution).ReplaceAll('$','') )
    contribution = str( TString(contribution).ReplaceAll('{','') )
    contribution = str( TString(contribution).ReplaceAll('}','') )
    if 'PDF' in name:
        name2_pdf.append(name)
        pull2_pdf.append(float(pull))
        constraint2_pdf.append(float(constraint))
        contribution2_pdf.append(float(contribution))
    elif 'JES' in name:
        name2_jes.append(name)
        pull2_jes.append(float(pull))
        constraint2_jes.append(float(constraint))
        contribution2_jes.append(float(contribution))
    else:  
        name2_all.append(name)
        pull2_all.append(float(pull))
        constraint2_all.append(float(constraint))
        contribution2_all.append(float(contribution))


if len(name1_all) != len(name2_all) or len(name1_pdf) != len(name2_pdf) or len(name1_jes) != len(name2_jes) : 
    print 'ERROR: input files must contain the same systematics!'
    print
    quit()

for i in range(0,len(name1_all)):
    if name1_all[i] != name2_all[i]:
        print 'ERROR: input files must contain the same systematics!'
        print
        quit()

for i in range(0,len(name1_pdf)):
    if name1_pdf[i] != name2_pdf[i]:
        print 'ERROR: input files must contain the same systematics!'
        print
        quit()

for i in range(0,len(name1_jes)):
    if name1_jes[i] != name2_jes[i]:
        print 'ERROR: input files must contain the same systematics!'
        print
        quit()
        
 
outDir = 'comparisonPlots/'+dir1+'_'+dir2

if not os.path.exists(outDir):
    os.makedirs(outDir)
   

#pulls
histo1_all = ROOT.TH1F('histo1_all','histo1_all',len(name1_all),-.5,-.5+len(name1_all))
histo1_jes = ROOT.TH1F('histo1_jes','histo1_jes',len(name1_jes),-.5,-.5+len(name1_jes))
histo1_pdf = ROOT.TH1F('histo1_pdf','histo1_pdf',len(name1_pdf),-.5,-.5+len(name1_pdf))

for i in range(0,len(name1_all)):
    histo1_all.Fill(i,pull1_all[i])
    histo1_all.GetXaxis().SetBinLabel(i+1,name1_all[i])
    histo1_all.SetBinError(i+1,constraint1_all[i])

for i in range(0,len(name1_jes)):
    histo1_jes.Fill(i,pull1_jes[i])
    histo1_jes.GetXaxis().SetBinLabel(i+1,name1_jes[i])
    histo1_jes.SetBinError(i+1,constraint1_jes[i])

for i in range(0,len(name1_pdf)):
    histo1_pdf.Fill(i,pull1_pdf[i])
    histo1_pdf.GetXaxis().SetBinLabel(i+1,name1_pdf[i])
    histo1_pdf.SetBinError(i+1,constraint1_pdf[i])

histo1_all.SetMarkerStyle(8)
histo1_all.SetMarkerColor(kRed)

histo1_jes.SetMarkerStyle(8)
histo1_jes.SetMarkerColor(kRed)

histo1_pdf.SetMarkerStyle(8)
histo1_pdf.SetMarkerColor(kRed)

###

histo2_all = ROOT.TH1F('histo2_all','histo2_all',len(name2_all),-.5,-.5+len(name2_all))
histo2_jes = ROOT.TH1F('histo2_jes','histo2_jes',len(name2_jes),-.5,-.5+len(name2_jes))
histo2_pdf = ROOT.TH1F('histo2_pdf','histo2_pdf',len(name2_pdf),-.5,-.5+len(name2_pdf))

for i in range(0,len(name2_all)):
    histo2_all.Fill(i,pull2_all[i])
    histo2_all.GetXaxis().SetBinLabel(i+1,name2_all[i])
    histo2_all.SetBinError(i+1,constraint2_all[i])

for i in range(0,len(name2_jes)):
    histo2_jes.Fill(i,pull2_jes[i])
    histo2_jes.GetXaxis().SetBinLabel(i+1,name2_jes[i])
    histo2_jes.SetBinError(i+1,constraint2_jes[i])

for i in range(0,len(name2_pdf)):
    histo2_pdf.Fill(i,pull2_pdf[i])
    histo2_pdf.GetXaxis().SetBinLabel(i+1,name2_pdf[i])
    histo2_pdf.SetBinError(i+1,constraint2_pdf[i])

histo2_all.SetMarkerStyle(8)
histo2_all.SetMarkerColor(kBlue)
histo2_jes.SetMarkerStyle(8)
histo2_jes.SetMarkerColor(kBlue)
histo2_pdf.SetMarkerStyle(8)
histo2_pdf.SetMarkerColor(kBlue)

line_up_all = ROOT.TLine(histo1_all.GetBinLowEdge(1),1.,histo1_all.GetBinLowEdge(histo1_all.GetNbinsX())+histo1_all.GetBinWidth(1),1.)
line_down_all = ROOT.TLine(histo1_all.GetBinLowEdge(1),-1.,histo1_all.GetBinLowEdge(histo1_all.GetNbinsX())+histo1_all.GetBinWidth(1),-1.)
line_up_all.SetLineColor(kGreen+1)
line_down_all.SetLineColor(kGreen+1)
line_up_all.SetLineWidth(2)
line_down_all.SetLineWidth(2)

line_up_pdf = ROOT.TLine(histo1_pdf.GetBinLowEdge(1),1.,histo1_pdf.GetBinLowEdge(histo1_pdf.GetNbinsX())+histo1_pdf.GetBinWidth(1),1.)
line_down_pdf = ROOT.TLine(histo1_pdf.GetBinLowEdge(1),-1.,histo1_pdf.GetBinLowEdge(histo1_pdf.GetNbinsX())+histo1_pdf.GetBinWidth(1),-1.)
line_up_pdf.SetLineColor(kGreen+1)
line_down_pdf.SetLineColor(kGreen+1)
line_up_pdf.SetLineWidth(2)
line_down_pdf.SetLineWidth(2)

line_up_jes = ROOT.TLine(histo1_jes.GetBinLowEdge(1),1.,histo1_jes.GetBinLowEdge(histo1_jes.GetNbinsX())+histo1_jes.GetBinWidth(1),1.)
line_down_jes = ROOT.TLine(histo1_jes.GetBinLowEdge(1),-1.,histo1_jes.GetBinLowEdge(histo1_jes.GetNbinsX())+histo1_jes.GetBinWidth(1),-1.)
line_up_jes.SetLineColor(kGreen+1)
line_down_jes.SetLineColor(kGreen+1)
line_up_jes.SetLineWidth(2)
line_down_jes.SetLineWidth(2)


leg = ROOT.TLegend(.85,.85,1.,1.)
leg.AddEntry(histo1_all,dir1)
leg.AddEntry(histo2_all,dir2)
leg.AddEntry(line_up_all,'prefit','l')

c1 = ROOT.TCanvas('c1','c1')
c1.SetGrid()
histo1_all.SetTitle('fit summary')
histo1_all.SetMaximum(3.5)
histo1_all.SetMinimum(-3.5)
# histo1_all.SetMaximum(max(histo1_all.GetMaximum(),histo2_all.GetMaximum())*2)
# histo1_all.SetMinimum(min(histo1_all.GetMinimum(),histo2_all.GetMinimum())*1.25)
histo1_all.Draw('p')
histo2_all.Draw('psame')
line_up_all.Draw('same')
line_down_all.Draw('same')
histo1_all.Draw('psame')
histo2_all.Draw('psame')
leg.Draw('same')
c1.SaveAs(outDir+'/all_summary.pdf','pdf')
c1.SaveAs(outDir+'/all_summary.png','png')

c1.Clear()
c1.SetGrid()
histo1_pdf.SetTitle('summary - PDF')
histo1_pdf.SetMaximum(1.3)
histo1_pdf.SetMinimum(-1.3)
# histo1_pdf.SetMaximum(max(histo1_pdf.GetMaximum(),histo2_pdf.GetMaximum())*3.5)
# histo1_pdf.SetMinimum(min(histo1_pdf.GetMinimum(),histo2_pdf.GetMinimum())*2.5)
histo1_pdf.Draw('p')
histo2_pdf.Draw('psame')
line_up_pdf.Draw('same')
line_down_pdf.Draw('same')
histo1_pdf.Draw('psame')
histo2_pdf.Draw('psame')
leg.Draw('same')
c1.SaveAs(outDir+'/pdf_summary.pdf','pdf')
c1.SaveAs(outDir+'/pdf_summary.png','png')

c1.Clear()
c1.SetGrid()
histo1_jes.SetTitle('summary - JES')
histo1_jes.SetMaximum(1.7)
histo1_jes.SetMinimum(-1.7)
# histo1_jes.SetMaximum(max(histo1_jes.GetMaximum(),histo2_jes.GetMaximum())*2)
# histo1_jes.SetMinimum(min(histo1_jes.GetMinimum(),histo2_jes.GetMinimum())*2)
histo1_jes.Draw('p')
histo2_jes.Draw('psame')
line_up_jes.Draw('same')
line_down_jes.Draw('same')
histo1_jes.Draw('psame')
histo2_jes.Draw('psame')
leg.Draw('same')
c1.SaveAs(outDir+'/jes_summary.pdf','pdf')
c1.SaveAs(outDir+'/jes_summary.png','png')

#constraints
histo1_all.Reset()
histo2_all.Reset()
histo1_pdf.Reset()
histo2_pdf.Reset()
histo1_jes.Reset()
histo2_jes.Reset()
c1.Clear()

leg = ROOT.TLegend(.85,.9,1.,1.)
leg.AddEntry(histo1_all,dir1)
leg.AddEntry(histo2_all,dir2)

# for i in range(0,len(name1_all)):
#     histo1_all.Fill(i,constraint1_all[i])
#     histo1_all.GetXaxis().SetBinLabel(i+1,name1_all[i])
#     histo1_all.SetBinError(i+1,0)

# for i in range(0,len(name1_jes)):
#     histo1_jes.Fill(i,constraint1_jes[i])
#     histo1_jes.GetXaxis().SetBinLabel(i+1,name1_jes[i])
#     histo1_jes.SetBinError(i+1,0)

# for i in range(0,len(name1_pdf)):
#     histo1_pdf.Fill(i,constraint1_pdf[i])
#     histo1_pdf.GetXaxis().SetBinLabel(i+1,name1_pdf[i])
#     histo1_pdf.SetBinError(i+1,0)

# ###

# for i in range(0,len(name2_all)):
#     histo2_all.Fill(i,constraint2_all[i])
#     histo2_all.GetXaxis().SetBinLabel(i+1,name2_all[i])
#     histo2_all.SetBinError(i+1,0)

# for i in range(0,len(name2_jes)):
#     histo2_jes.Fill(i,constraint2_jes[i])
#     histo2_jes.GetXaxis().SetBinLabel(i+1,name2_jes[i])
#     histo2_jes.SetBinError(i+1,0)

# for i in range(0,len(name2_pdf)):
#     histo2_pdf.Fill(i,constraint2_pdf[i])
#     histo2_pdf.GetXaxis().SetBinLabel(i+1,name2_pdf[i])
#     histo2_pdf.SetBinError(i+1,0)

# max_global = -999.

# for i in range(0,len(name1_all)):
#     histo1_all.Fill(i,constraint1_all[i])
#     histo1_all.GetXaxis().SetBinLabel(i+1,name1_all[i])
#     histo1_all.SetBinError(i+1,0)
#     if constraint1_all[i] > max_global: max_global = constraint1_all[i]

# for i in range(0,len(name1_jes)):
#     histo1_jes.Fill(i,constraint1_jes[i])
#     histo1_jes.GetXaxis().SetBinLabel(i+1,name1_jes[i])
#     histo1_jes.SetBinError(i+1,0)
#     if constraint1_jes[i] > max_global: max_global = constraint1_jes[i]

# for i in range(0,len(name1_pdf)):
#     histo1_pdf.Fill(i,constraint1_pdf[i])
#     histo1_pdf.GetXaxis().SetBinLabel(i+1,name1_pdf[i])
#     histo1_pdf.SetBinError(i+1,0)
#     if constraint1_pdf[i] > max_global: max_global = constraint1_pdf[i]


# ###

# for i in range(0,len(name2_all)):
#     histo2_all.Fill(i,constraint2_all[i])
#     histo2_all.GetXaxis().SetBinLabel(i+1,name2_all[i])
#     histo2_all.SetBinError(i+1,0)
#     if constraint2_all[i] > max_global: max_global = constraint2_all[i]

# for i in range(0,len(name2_jes)):
#     histo2_jes.Fill(i,constraint2_jes[i])
#     histo2_jes.GetXaxis().SetBinLabel(i+1,name2_jes[i])
#     histo2_jes.SetBinError(i+1,0)
#     if constraint2_jes[i] > max_global: max_global = constraint2_jes[i]

# for i in range(0,len(name2_pdf)):
#     histo2_pdf.Fill(i,constraint2_pdf[i])
#     histo2_pdf.GetXaxis().SetBinLabel(i+1,name2_pdf[i])
#     histo2_pdf.SetBinError(i+1,0)
#     if constraint2_pdf[i] > max_global: max_global = constraint2_pdf[i]


# c1.SetGrid()
# histo1_all.SetTitle('constraints')
# histo1_all.SetMaximum(max_global*1.1)
# histo1_all.SetMinimum(-.1)
# histo1_all.Draw('p')
# histo2_all.Draw('psame')
# leg.Draw('same')
# c1.SaveAs(outDir+'/all_constraints.pdf','pdf')
# c1.SaveAs(outDir+'/all_constraints.png','png')

# c1.Clear()
# c1.SetGrid()
# histo1_pdf.SetTitle('constraints - PDF')
# histo1_pdf.SetMaximum(1.1)
# histo1_pdf.SetMinimum(.5)
# histo1_pdf.Draw('p')
# histo2_pdf.Draw('psame')
# leg.Draw('same')
# c1.SaveAs(outDir+'/pdf_constraints.pdf','pdf')
# c1.SaveAs(outDir+'/pdf_constraints.png','png')

# c1.Clear()
# c1.SetGrid()
# histo1_jes.SetTitle('constraints - JES')
# histo1_jes.SetMaximum(1.2)
# histo1_jes.SetMinimum(-.1)
# histo1_jes.Draw('p')
# histo2_jes.Draw('psame')
# leg.Draw('same')
# c1.SaveAs(outDir+'/jes_constraints.pdf','pdf')
# c1.SaveAs(outDir+'/jes_constraints.png','png')

#contributions
histo1_all.Reset()
histo2_all.Reset()
histo1_pdf.Reset()
histo2_pdf.Reset()
histo1_jes.Reset()
histo2_jes.Reset()
c1.Clear()

# min_all = min_pdf = min_jes = 1000.
# max_all = max_pdf = max_jes = -1000.

# for i in range(0,len(name1_all)):
#     histo1_all.Fill(i,contribution1_all[i])
#     histo1_all.GetXaxis().SetBinLabel(i+1,name1_all[i])
#     histo1_all.SetBinError(i+1,0)
#     if contribution1_all[i] > max_all: max_all = contribution1_all[i]
#     if contribution1_all[i] < min_all: min_all = contribution1_all[i]

# for i in range(0,len(name1_jes)):
#     histo1_jes.Fill(i,contribution1_jes[i])
#     histo1_jes.GetXaxis().SetBinLabel(i+1,name1_jes[i])
#     histo1_jes.SetBinError(i+1,0)
#     if contribution1_jes[i] > max_jes: max_jes = contribution1_jes[i]
#     if contribution1_jes[i] < min_jes: min_jes = contribution1_jes[i]

# for i in range(0,len(name1_pdf)):
#     histo1_pdf.Fill(i,contribution1_pdf[i])
#     histo1_pdf.GetXaxis().SetBinLabel(i+1,name1_pdf[i])
#     histo1_pdf.SetBinError(i+1,0)
#     if contribution1_pdf[i] > max_pdf: max_pdf = contribution1_pdf[i]
#     if contribution1_pdf[i] < min_pdf: min_pdf = contribution1_pdf[i]


# ###

# for i in range(0,len(name2_all)):
#     histo2_all.Fill(i,contribution2_all[i])
#     histo2_all.GetXaxis().SetBinLabel(i+1,name2_all[i])
#     histo2_all.SetBinError(i+1,0)
#     if contribution2_all[i] > max_all: max_all = contribution2_all[i]
#     if contribution2_all[i] < min_all: min_all = contribution2_all[i]


# for i in range(0,len(name2_jes)):
#     histo2_jes.Fill(i,contribution2_jes[i])
#     histo2_jes.GetXaxis().SetBinLabel(i+1,name2_jes[i])
#     histo2_jes.SetBinError(i+1,0)
#     if contribution2_jes[i] > max_jes: max_jes = contribution2_jes[i]
#     if contribution2_jes[i] < min_jes: min_jes = contribution2_jes[i]

# for i in range(0,len(name2_pdf)):
#     histo2_pdf.Fill(i,contribution2_pdf[i])
#     histo2_pdf.GetXaxis().SetBinLabel(i+1,name2_pdf[i])
#     histo2_pdf.SetBinError(i+1,0)
#     if contribution2_pdf[i] > max_pdf: max_pdf = contribution2_pdf[i]
#     if contribution2_pdf[i] < min_pdf: min_pdf = contribution2_pdf[i]

max_global = -999.

for i in range(0,len(name1_all)):
    histo1_all.Fill(i,abs(contribution1_all[i]))
    histo1_all.GetXaxis().SetBinLabel(i+1,name1_all[i])
    histo1_all.SetBinError(i+1,0)
    if abs(contribution1_all[i]) > max_global: max_global = abs(contribution1_all[i])

for i in range(0,len(name1_jes)):
    histo1_jes.Fill(i,abs(contribution1_jes[i]))
    histo1_jes.GetXaxis().SetBinLabel(i+1,name1_jes[i])
    histo1_jes.SetBinError(i+1,0)
    if abs(contribution1_jes[i]) > max_global: max_global = abs(contribution1_jes[i])

for i in range(0,len(name1_pdf)):
    histo1_pdf.Fill(i,abs(contribution1_pdf[i]))
    histo1_pdf.GetXaxis().SetBinLabel(i+1,name1_pdf[i])
    histo1_pdf.SetBinError(i+1,0)
    if abs(contribution1_pdf[i]) > max_global: max_global = abs(contribution1_pdf[i])


###

for i in range(0,len(name2_all)):
    histo2_all.Fill(i,abs(contribution2_all[i]))
    histo2_all.GetXaxis().SetBinLabel(i+1,name2_all[i])
    histo2_all.SetBinError(i+1,0)
    if abs(contribution2_all[i]) > max_global: max_global = abs(contribution2_all[i])

for i in range(0,len(name2_jes)):
    histo2_jes.Fill(i,abs(contribution2_jes[i]))
    histo2_jes.GetXaxis().SetBinLabel(i+1,name2_jes[i])
    histo2_jes.SetBinError(i+1,0)
    if abs(contribution2_jes[i]) > max_global: max_global = abs(contribution2_jes[i])

for i in range(0,len(name2_pdf)):
    histo2_pdf.Fill(i,abs(contribution2_pdf[i]))
    histo2_pdf.GetXaxis().SetBinLabel(i+1,name2_pdf[i])
    histo2_pdf.SetBinError(i+1,0)
    if abs(contribution2_pdf[i]) > max_global: max_global = abs(contribution2_pdf[i])


c1.SetGrid()
histo1_all.SetTitle('contributions')
histo1_all.SetMaximum(max_global+.25)
histo1_all.SetMinimum(-.1)
histo1_all.Draw('p')
histo2_all.Draw('psame')
leg.Draw('same')
c1.SaveAs(outDir+'/all_contributions.pdf','pdf')
c1.SaveAs(outDir+'/all_contributions.png','png')

c1.Clear()
c1.SetGrid()
histo1_pdf.SetTitle('contributions - PDF')
histo1_pdf.SetMaximum(.5)
histo1_pdf.SetMinimum(-.05)
histo1_pdf.Draw('p')
histo2_pdf.Draw('psame')
leg.Draw('same')
c1.SaveAs(outDir+'/pdf_contributions.pdf','pdf')
c1.SaveAs(outDir+'/pdf_contributions.png','png')

c1.Clear()
c1.SetGrid()
histo1_jes.SetTitle('contributions - JES')
histo1_jes.SetMaximum(.2)
histo1_jes.SetMinimum(-.05)
histo1_jes.Draw('p')
histo2_jes.Draw('psame')
leg.Draw('same')
c1.SaveAs(outDir+'/jes_contributions.pdf','pdf')
c1.SaveAs(outDir+'/jes_contributions.png','png')
