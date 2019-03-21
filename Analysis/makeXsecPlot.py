import ROOT
from ROOT import TString, TFile, TGraph, TGraphAsymmErrors, TCanvas

th_xsec = [258.3, 304.5, 176.8, 50.2]
bins = [0,420,550,810,2000]
scales = [384.0, 476.2, 644.3, 1023.6]

rFile = TFile('toys.root')

def getXsec(mttbin):

    inFile = open('xsecFit_tab13TeV_mtt'+str(mttbin)+'.tex','r')
    l = inFile.read().splitlines()
    xsec = ''
    total = ''
    for line in l:
        if 'Xsec\\_13TeV'+str(mttbin) in line and '\\hline' in line:
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


g_th = TGraphAsymmErrors()
g_exp = TGraphAsymmErrors()

for bin in range(1,5):
    g_th.SetPoint(bin-1, scales[bin-1], th_xsec[bin-1])
    g_th.SetPointError(bin-1,scales[bin-1]-bins[bin-1], bins[bin]-scales[bin-1], 0, 0)
    xsec_err = getXsec(bin)
    g_exp.SetPoint(bin-1, scales[bin-1], xsec_err[0])
    g_exp.SetPointError(bin-1,scales[bin-1]-bins[bin-1], bins[bin]-scales[bin-1], xsec_err[2], xsec_err[1])


g_th.SetMarkerStyle(7)
g_th.SetMarkerColor(ROOT.kRed)
g_th.SetLineColor(ROOT.kRed)

g_exp.SetMarkerStyle(8)


latexLabel1 = ROOT.TLatex()
latexLabel1.SetTextSize(0.06)
latexLabel1.SetNDC()

latexLabel2 = ROOT.TLatex()
latexLabel2.SetTextSize(0.04)
latexLabel2.SetTextFont(42)
latexLabel2.SetNDC()

leg = ROOT.TLegend(.35,.73,.89,.85)
leg.AddEntry(g_exp,'unfolded differential cross section','ple')
leg.AddEntry(g_th,'MCFM-6.8, m_{t}(m_{t})=164GeV, ABMP16_5_nlo','pl')

c = TCanvas()
g_exp.GetXaxis().SetTitle('m_{t#bar{t}} [GeV]')
g_exp.GetYaxis().SetTitle('#frac{d#sigma}{dm_{t#bar{t}}} #Deltam_{t#bar{t}} [pb]')
g_exp.GetYaxis().SetTitleOffset(1.25)
g_exp.GetXaxis().SetTitleSize(0.045)
g_exp.Draw('ap')
g_th.Draw('psame')
leg.Draw('same')

latexLabel1.DrawLatex(0.11, 0.92, "CMS")
latexLabel2.DrawLatex(0.70, 0.92, "35.9 fb^{-1} (13 TeV)")



c.SaveAs('diffXsec.png')
c.SaveAs('diffXsec.pdf')
