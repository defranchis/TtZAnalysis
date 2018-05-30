import os
import ROOT
from ROOT import *

inFile = 'toys.root'
outdir = 'plots_toys'

forAN=True

rootFile = ROOT.TFile(inFile,'r')

if not os.path.exists(outdir):
    os.makedirs(outdir)

c = ROOT.TCanvas('c','c')

histolist = rootFile.GetListOfKeys()
for histokey in histolist: 
    if histokey.GetClassName() != 'TH1F': continue
    name = histokey.GetName()
    histo = rootFile.FindObjectAny(name)
    c.Clear()
    histo.Draw()
    outname = TString(name).ReplaceAll(' ','')
    outname.ReplaceAll('\\','')
    outname.ReplaceAll('/','')
    outname.ReplaceAll('$','')
    outname.ReplaceAll('.','_')
    outname.ReplaceAll('(','')
    outname.ReplaceAll(')','')
    outname.ReplaceAll('{','')
    outname.ReplaceAll('}','')
    if outname.EndsWith('_constr'): histo.Rebin()
    outfilename = TString(outdir+'/'+str(outname)+'.pdf')
    c.SaveAs(str(outfilename),'pdf')

    if outname.EndsWith('_pull'):
        title = outname.ReplaceAll('_pull','')
        title.ReplaceAll('_','-')
        title.ReplaceAll('^','')
        if '13TeV' in str(title): continue

        if (forAN):
            print '\\begin{figure}[htbp!]'
            print '  \\begin{center}'
            print '    \\resizebox{0.32 \\textwidth}{!}{\\includegraphics{'+str(outfilename)+'}}'
            outfilename.ReplaceAll('_pull','_constr')
            print '    \\resizebox{0.32 \\textwidth}{!}{\\includegraphics{'+str(outfilename)+'}}'
            outfilename.ReplaceAll('_constr','_contrib')
            print '    \\resizebox{0.32 \\textwidth}{!}{\\includegraphics{'+str(outfilename)+'}}'
            print '\\caption{Pull (left), constraint (middle) and contribution to total uncertainty on \\mtmc (right) for variation '+str(title)+'.'
            print '       \\label{fig:toys_'+str(title)+'}}'
            print '  \\end{center}'
            print '\\end{figure}'
            print

        else:
            print '\\begin{frame}{'+str(title)+'}'
            print '\includegraphics[width=.5\\textwidth]{'+str(outfilename)+'}'
            outfilename.ReplaceAll('_pull','_constr')
            print '\includegraphics[width=.5\\textwidth]{'+str(outfilename)+'} \\\\'
            outfilename.ReplaceAll('_constr','_contrib')
            print '\includegraphics[width=.5\\textwidth]{'+str(outfilename)+'}'
            print '\\end{frame}'
            print



