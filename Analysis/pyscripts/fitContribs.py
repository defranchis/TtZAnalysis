import time
import os
import ROOT
from ROOT import *


gStyle.SetOptStat(0000)

latexLabel2 = TLatex()                                                      
latexLabel2.SetTextSize(0.04)                                               
latexLabel2.SetTextFont(42)                                                 
latexLabel2.SetNDC()                                                        


def plotContribs(b_mass):

	if b_mass: f1 = open('xsecFit_tab_TOPMASS.tex','r')
	else : f1 = open('xsecFit_tab13TeV.tex','r')
	l1 = f1.read().splitlines()

	for i in range(0,3): del l1[0]

	l1_short = []

	for line in l1:
                if '(13TeV)' in line and not b_mass: break
                if 'Stat' in line and not 'JES' in line and b_mass: break
                l1_short.append(line)


	name_all = []
	contribution_all = []
	name_pdf = []
	contribution_pdf = []
	name_jes = []
	contribution_jes = []
	name_btag = []
	contribution_btag = []
	name_mod = []
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
            if 'TeV' in name: name = "t#bar{t} cross section"
        
	    contribution = str( TString(contribution).ReplaceAll('\\','') )
	    contribution = str( TString(contribution).ReplaceAll('$','') )
	    contribution = str( TString(contribution).ReplaceAll('{','') )
	    contribution = str( TString(contribution).ReplaceAll('}','') )
	    # if 'mass' in name or 'DY GEN' in name: continue
	    # if 'm_{t}' in name or 'Lumi' in name or 'DY GEN' in name: continue
	    # if 'Lumi' in name or 'DY GEN' in name: continue
	    if 'PDF' in name:
		name_pdf.append(name)
		contribution_pdf.append(float(contribution))
	    elif 'b-tag' in name or 'mistag' in name:
		name_btag.append(name)
		contribution_btag.append(float(contribution))
	    elif 'JES' in name:
		name_jes.append(name)
		contribution_jes.append(float(contribution))
	    elif ('scale' in name and not 'Electron' in name and not 'Muon' in name) or 'GEN' in name or 'fragm' in name or 'match' in name or 'BR' in name or 'TT' in name or 'tune' in name or 'pT' in name or 'CR' in name or 'NLO' in name or 'mass' in name :
		name_mod.append(name)
		contribution_mod.append(float(contribution))
	    else:
		name_all.append(name)
		contribution_all.append(float(contribution))


	contrib_all = ROOT.TH1F('contrib_all','contrib_all',len(name_all),-.5,-.5+len(name_all))
	contrib_pdf = ROOT.TH1F('contrib_pdf','contrib_pdf',len(name_pdf),-.5,-.5+len(name_pdf))
	contrib_jes = ROOT.TH1F('contrib_jes','contrib_jes',len(name_jes),-.5,-.5+len(name_jes))
	contrib_btag = ROOT.TH1F('contrib_btag','contrib_btag',len(name_btag),-.5,-.5+len(name_btag))
	contrib_mod = ROOT.TH1F('contrib_mod','contrib_mod',len(name_mod),-.5,-.5+len(name_mod))


	for i in range(0,len(name_all)):
	    contrib_all.Fill(i,contribution_all[i])
	    contrib_all.GetXaxis().SetBinLabel(i+1,name_all[i])

	for i in range(0,len(name_pdf)):
	    contrib_pdf.Fill(i,contribution_pdf[i])
	    contrib_pdf.GetXaxis().SetBinLabel(i+1,name_pdf[i])

	for i in range(0,len(name_jes)):
	    contrib_jes.Fill(i,contribution_jes[i])
	    contrib_jes.GetXaxis().SetBinLabel(i+1,name_jes[i])

	for i in range(0,len(name_btag)):
	    contrib_btag.Fill(i,contribution_btag[i])
	    contrib_btag.GetXaxis().SetBinLabel(i+1,name_btag[i])

	for i in range(0,len(name_mod)):
	    contrib_mod.Fill(i,contribution_mod[i])
            temp_name=name_mod[i]
            temp_name = str( TString(temp_name).ReplaceAll('ttbar','t#bar{t}') )
            temp_name = str( TString(temp_name).ReplaceAll('Top pT','Top quark p_{T}') )
            temp_name = str( TString(temp_name).ReplaceAll('fragmentation','fragm. Bowler-Lund') )
            temp_name = str( TString(temp_name).ReplaceAll(' BR',' BF') )
            temp_name = str( TString(temp_name).ReplaceAll('fragm.','Fragm.') )

	    contrib_mod.GetXaxis().SetBinLabel(i+1,temp_name)

        outdir = 'contributions'
	if not os.path.exists(outdir):
	    os.makedirs(outdir)
        outdir+='/'

	c1 = ROOT.TCanvas('c1','c1')
	c1.SetGrid()
	c1.SetBottomMargin(.15)

        suffix = '_xsec'
        if b_mass: suffix = '_mass'

        ytitle = 'Contribution [%]'
        if b_mass: ytitle = 'Contribution [GeV]'
	###################################

	contrib_all.SetTitle('')
	contrib_all.GetYaxis().SetTitle(ytitle)
	contrib_all.GetYaxis().SetTitleSize(.05)
	contrib_all.GetYaxis().SetTitleOffset(.98)
	contrib_all.SetFillColor(kYellow)
	contrib_all.SetLineColor(kRed)
	contrib_all.Draw('hist')
        latexLabel2.DrawLatex(0.7, 0.92, "35.9 fb^{-1} (13 TeV)") 
        latexLabel2.DrawLatex(0.15, 0.92, "experimental uncertainties")

	c1.SaveAs(outdir+'all_contribution'+suffix+'.pdf','pdf')
	c1.SaveAs(outdir+'all_contribution'+suffix+'.png','png')
	c1.Clear()

	###

	contrib_pdf.SetTitle('')
	contrib_pdf.GetYaxis().SetTitle(ytitle)
	contrib_pdf.GetYaxis().SetTitleSize(.05)
	contrib_pdf.GetYaxis().SetTitleOffset(.98)
	contrib_pdf.SetFillColor(kYellow)
	contrib_pdf.SetLineColor(kRed)
	contrib_pdf.Draw('hist')
        latexLabel2.DrawLatex(0.7, 0.92, "35.9 fb^{-1} (13 TeV)") 
        latexLabel2.DrawLatex(0.15, 0.92, "PDF uncertainties")

	c1.SaveAs(outdir+'pdf_contribution'+suffix+'.pdf','pdf')
	c1.SaveAs(outdir+'pdf_contribution'+suffix+'.png','png')
	c1.Clear()

	###

	contrib_jes.SetTitle('')
	contrib_jes.GetYaxis().SetTitle(ytitle)
	contrib_jes.GetYaxis().SetTitleSize(.05)
	contrib_jes.GetYaxis().SetTitleOffset(.98)
	contrib_jes.SetFillColor(kYellow)
	contrib_jes.SetLineColor(kRed)
	contrib_jes.Draw('hist')
        latexLabel2.DrawLatex(0.7, 0.92, "35.9 fb^{-1} (13 TeV)") 
        latexLabel2.DrawLatex(0.15, 0.92, "JES uncertainties")

	c1.SaveAs(outdir+'jes_contribution'+suffix+'.pdf','pdf')
	c1.SaveAs(outdir+'jes_contribution'+suffix+'.png','png')
	c1.Clear()

	###

	contrib_btag.SetTitle('')
	contrib_btag.GetYaxis().SetTitle(ytitle)
	contrib_btag.GetYaxis().SetTitleSize(.05)
	contrib_btag.GetYaxis().SetTitleOffset(.98)
	contrib_btag.SetFillColor(kYellow)
	contrib_btag.SetLineColor(kRed)
	contrib_btag.Draw('hist')
        latexLabel2.DrawLatex(0.7, 0.92, "35.9 fb^{-1} (13 TeV)") 
        latexLabel2.DrawLatex(0.15, 0.92, "b tagging uncertainties")

	c1.SaveAs(outdir+'btag_contribution'+suffix+'.pdf','pdf')
	c1.SaveAs(outdir+'btag_contribution'+suffix+'.png','png')
	c1.Clear()

	###

	contrib_mod.SetTitle('')
	contrib_mod.GetYaxis().SetTitle(ytitle)
	contrib_mod.GetYaxis().SetTitleSize(.05)
	contrib_mod.GetYaxis().SetTitleOffset(.98)
	contrib_mod.SetFillColor(kYellow)
	contrib_mod.SetLineColor(kRed)
	contrib_mod.Draw('hist')
        latexLabel2.DrawLatex(0.7, 0.92, "35.9 fb^{-1} (13 TeV)") 
        latexLabel2.DrawLatex(0.15, 0.92, "modelling uncertainties")

	c1.SaveAs(outdir+'mod_contribution'+suffix+'.pdf','pdf')
	c1.SaveAs(outdir+'mod_contribution'+suffix+'.png','png')
	c1.Clear()

plotContribs(False)
plotContribs(True)
