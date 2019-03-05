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

    for i in range(0,3): del l[0]
    l_short = []

    for line in l:
        if '13TeV' in line: break
        l_short.append(line)

    name_all = []
    contribution_all = []

    for line in l_short:
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
            name = str( TString(name).ReplaceAll('fragmentation','fragm') )
            name = str( TString(name).ReplaceAll('correction','corr') )
            name = str( TString(name).ReplaceAll('template','templ') )
            name = str( TString(name).ReplaceAll('$D \to \mu X$','D to mu') )
            name = str( TString(name).ReplaceAll('splitting','split') )
            name = str( TString(name).ReplaceAll('production','prod') )
            name = str( TString(name).ReplaceAll('light to c','l/c') )
            name = str( TString(name).ReplaceAll('dependence','dep') )
            name = str( TString(name).ReplaceAll('statistical','stat') )
            name = str( TString(name).ReplaceAll('$','') )
            name = str( TString(name).ReplaceAll('\\','#') )
            name = str( TString(name).ReplaceAll('#to','tp') )
            name = str( TString(name).ReplaceAll('K_s^0','Ks0') )

        contribution = str( TString(contribution).ReplaceAll('\\','') )
        contribution = str( TString(contribution).ReplaceAll('$','') )
        contribution = str( TString(contribution).ReplaceAll('{','') )
        contribution = str( TString(contribution).ReplaceAll('}','') )

        name = TString(name).ReplaceAll(' ','_')
        name.ReplaceAll(':_','_')
        name.ReplaceAll('#DeltaR','DR')
        name.ReplaceAll('#mu','mu')
        name.ReplaceAll('(#Lambda)','Lambda')
        name.ReplaceAll('_-_','_')
        name.ReplaceAll('_/_','_')
        name.ReplaceAll('fragm.','fragm')
        name.ReplaceAll('_(','_')
        name.ReplaceAll(')','')
        if name.EndsWith('_') : name = str(name)[0:len(str(name))-1]
        name_all.append('\''+str(name)+'\'')
        contribution_all.append(float(contribution))


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
        print
        counter = 0
        print 'syst_names = [',
        for name in name_all: 
            counter +=1
            if counter < 5: print name,',',
            else:
                print name,','
                counter = 0
        print ']'
        print

    counter = 0
    print 'contribs_'+str(mttbin)+' = [',    
    for contrib in contribution_all:
        counter+=1
        if counter<10 : print contrib,',',
        else :
            print contrib,','
            counter=0
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
