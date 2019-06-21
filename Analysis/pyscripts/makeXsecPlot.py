import ROOT
from ROOT import TString, TFile, TGraph, TGraphAsymmErrors, TCanvas

th_xsec_164 = [261.4, 304.4, 177.3, 46.6]
th_xsec_162 = [294.1, 314.8, 181.0, 47.3]
th_xsec_166 = [231.4, 294.7, 173.3, 46.2]

bins = [345,420,550,810,2000]
scales = [384.0, 476.2, 644.3, 1023.6]

rFile = TFile('toys.root')

def getXsec(mttbin):

    inFile = open('xsecFit_tab13TeV_mtt'+str(mttbin)+'.tex','r')
    l = inFile.read().splitlines()
    xsec = ''
    total = ''
    for line in l:
        if '$\\sigma_{t\\bar{t}}^{(\\mu_'+str(mttbin) in line and '\\hline' in line:
            xsec = line
        if 'Total' in line and not 'vis' in line:
            total=line

    xsec = xsec.split('&')[3]
    xsec = TString(xsec)
    xsec.ReplaceAll('pb \\\\ \\hline \\hline','')
    xsec = float(str(xsec))

    total = total.split('&')[3]
    total = TString(total)
    total.ReplaceAll('}_{',' ')
    total.ReplaceAll(' $\mp^{','')
    total.ReplaceAll(' $\pm^{','')
    total.ReplaceAll('}$ \\\\','')
    total.ReplaceAll(' \hline','')
    total = str(total)
    total = total.split()
    err_up = float(total[0])
    err_down = float(total[1])

    err_up *= xsec/100.
    err_down *= xsec/100.

    h=rFile.Get('h_xsec_'+str(mttbin))
    err_toys = h.GetRMS()

    err_up = (err_up**2+err_toys**2)**.5
    err_down = (err_down**2+err_toys**2)**.5

    err_up = round(err_up,2)
    err_down = round(err_down,2)

    return [xsec, err_up, err_down]


g_th_164 = TGraphAsymmErrors()
g_th_162 = TGraphAsymmErrors()
g_th_166 = TGraphAsymmErrors()
g_exp = TGraphAsymmErrors()

for bin in range(1,5):
    g_th_164.SetPoint(bin-1, scales[bin-1], th_xsec_164[bin-1])
    g_th_164.SetPointError(bin-1,scales[bin-1]-bins[bin-1], bins[bin]-scales[bin-1], 0, 0)
    g_th_162.SetPoint(bin-1, scales[bin-1], th_xsec_162[bin-1])
    g_th_162.SetPointError(bin-1,scales[bin-1]-bins[bin-1], bins[bin]-scales[bin-1], 0, 0)
    g_th_166.SetPoint(bin-1, scales[bin-1], th_xsec_166[bin-1])
    g_th_166.SetPointError(bin-1,scales[bin-1]-bins[bin-1], bins[bin]-scales[bin-1], 0, 0)

    xsec_err = getXsec(bin)
    g_exp.SetPoint(bin-1, scales[bin-1], xsec_err[0])
    g_exp.SetPointError(bin-1,scales[bin-1]-bins[bin-1], bins[bin]-scales[bin-1], xsec_err[2], xsec_err[1])


g_th_164.SetMarkerStyle(0)
g_th_164.SetMarkerColor(ROOT.kRed)
g_th_164.SetLineColor(ROOT.kRed)

g_th_162.SetMarkerStyle(0)
g_th_162.SetMarkerColor(ROOT.kBlue)
g_th_162.SetLineColor(ROOT.kBlue)

g_th_166.SetMarkerStyle(0)
g_th_166.SetMarkerColor(ROOT.kGreen+2)
g_th_166.SetLineColor(ROOT.kGreen+2)

g_exp.SetMarkerStyle(8)
g_exp.SetLineWidth(1)


latexLabel1 = ROOT.TLatex()
latexLabel1.SetTextSize(0.06)
latexLabel1.SetNDC()

latexLabel2 = ROOT.TLatex()
latexLabel2.SetTextSize(0.04)
latexLabel2.SetTextFont(42)
latexLabel2.SetNDC()

leg = ROOT.TLegend(.30,.65,.89,.85)
leg.SetBorderSize(0)
leg.AddEntry(g_exp,'unfolded differential cross section','ple')
leg.AddEntry(g_th_162,'NLO prediction, m_{t}(m_{t})=162GeV, ABMP16','pl')
leg.AddEntry(g_th_164,'NLO prediction, m_{t}(m_{t})=164GeV, ABMP16','pl')
leg.AddEntry(g_th_166,'NLO prediction, m_{t}(m_{t})=166GeV, ABMP16','pl')

c = TCanvas()
c.SetLeftMargin(0.12)
c.SetBottomMargin(0.12)
g_exp.GetXaxis().SetTitle('m_{t#bar{t}} [GeV]')
g_exp.GetYaxis().SetTitle('#frac{d#sigma}{dm_{t#bar{t}}} #Deltam_{t#bar{t}} [pb]')
g_exp.GetYaxis().SetTitleOffset(1.1)
g_exp.GetXaxis().SetTitleSize(0.048)
g_exp.GetYaxis().SetTitleSize(0.045)

g_exp.Draw('ap')
g_th_164.Draw('psame')
g_th_162.Draw('psame')
g_th_166.Draw('psame')
g_exp.Draw('psame')
leg.Draw('same')

latexLabel1.DrawLatex(0.12, 0.92, "CMS")
latexLabel2.DrawLatex(0.70, 0.92, "35.9 fb^{-1} (13 TeV)")



c.SaveAs('diffXsec.png')
c.SaveAs('diffXsec.pdf')
c.SaveAs('diffXsec.root')
