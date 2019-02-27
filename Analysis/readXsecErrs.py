import ROOT
from ROOT import TString, TFile


#############

n_mttbins = 4

#############


rFile = TFile('toys.root')


def printXsecErrs(mttbin) :

    inFile = open('xsecFit_tab13TeV_mtt'+str(mttbin)+'.tex','r')
    l = inFile.read().splitlines()
    lextr = []
    tot_vis = ''
    xsec = ''
    for line in l:
        if '(extr)' in line:
            lextr.append(line)
        if 'Total vis' in line:
            tot_vis=line
        if 'Xsec\\_13TeV'+str(mttbin) in line and '\\hline' in line:
            xsec = line

    extr_up = []
    extr_down = []

    extr_name = []

    for line in lextr:
        extr = line.split('&')[3]
        name = line.split('&')[0]
        mp = False
        if '\mp' in line: mp=True
        extr = TString(extr)
        extr.ReplaceAll('}_{',' ')
        extr.ReplaceAll(' $\mp^{','')
        extr.ReplaceAll(' $\pm^{','')
        extr.ReplaceAll('}$ \\\\','')
        extr.ReplaceAll(' \hline','')
        extr = str(extr)
        extr = extr.split()
        extr[0] = float(extr[0])
        extr[1] = float(extr[1])
        if (mp) : extr[0] *= -1
        else :  extr[1] *= -1
        extr_up.append(extr[0])
        extr_down.append(extr[1])

        name = TString(name)
        name.ReplaceAll(' (extr) ','')
        name.ReplaceAll('$t\\bar{t}$ ','')
        name.ReplaceAll('Top','top')
        name.ReplaceAll('$p_{T}$','pt')
        name.ReplaceAll('$m_{t}^{MC}$','mt')
        name.ReplaceAll(' ','_')
        name = '\''+str(name)+'\''
        extr_name.append(name)

    tot_vis = tot_vis.split('&')[3]
    tot_vis = TString(tot_vis)
    tot_vis.ReplaceAll('}_{',' ')
    tot_vis.ReplaceAll(' $\mp^{','')
    tot_vis.ReplaceAll(' $\pm^{','')
    tot_vis.ReplaceAll('}$ \\\\','')
    tot_vis.ReplaceAll(' \hline','')
    tot_vis = str(tot_vis)
    tot_vis = tot_vis.split()
    tot_vis[0] = float(tot_vis[0])
    tot_vis[1] = float(tot_vis[1])

    xsec = xsec.split('&')[3]
    xsec = TString(xsec)
    xsec.ReplaceAll('pb \\\\ \\hline \\hline','')
    xsec = float(str(xsec))

    h=rFile.Get('h_xsec_'+str(mttbin))

    

    print
    print 'xsec_'+str(mttbin)+' =', xsec
    print 'err_xsec_'+str(mttbin)+'_up =', tot_vis[0]
    print 'err_xsec_'+str(mttbin)+'_down =', tot_vis[1]
    print 'extr_'+str(mttbin)+'_up = [',
    for e in extr_up: print e,',',
    print ']'
    print 'extr_'+str(mttbin)+'_down = [',
    for e in extr_down: print e,',',
    print ']'
    print 'err_xsec_toys_'+str(mttbin)+' =', round(h.GetRMS(),3)
    print
    if mttbin == 1 :
        print 'extr_name = [',
        for name in extr_name: print name,',',
        print ']'
        
    
    return


def printCorrelations():
    inFile = open('xsecFit_tabCorr.tex','r')
    l = inFile.read().splitlines()
    corrs = []
    for line in l:
        if 'Xsec\\_13TeV' in line and not '13TeV1' in line:
            corrs.append(line)

    corr_12 = corrs[0].split('&')
    corr_12 = corr_12[len(corr_12)-4]

    corr_32 = corrs[1].split('&')
    corr_32 = corr_32[len(corr_32)-3]

    corr_42 = corrs[2].split('&')
    corr_42 = corr_42[len(corr_42)-3]

    corr_12 = TString(corr_12)
    corr_12.ReplaceAll('\\textbf{','')
    corr_12.ReplaceAll('}','')
    corr_12 = float(str(corr_12))

    corr_32 = TString(corr_32)
    corr_32.ReplaceAll('\\textbf{','')
    corr_32.ReplaceAll('}','')
    corr_32 = float(str(corr_32))

    corr_42 = TString(corr_42)
    corr_42.ReplaceAll('\\textbf{','')
    corr_42.ReplaceAll('}','')
    corr_42 = float(str(corr_42))
    
    print
    print 'corr_1_2 =',corr_12
    print 'corr_3_2 =',corr_32
    print 'corr_4_2 =',corr_42
    print

    return


for i in range (1,n_mttbins+1):
    printXsecErrs(i)

printCorrelations()
