#include <iostream>
#include <fstream>
#include <iomanip>

#include "TString.h"
#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TKey.h"
#include "TDirectoryFile.h"                        
#include "TList.h"
#include "TROOT.h"
#include "TError.h"

#include "TtZAnalysis/Tools/interface/histo1D.h"
#include "TtZAnalysis/Tools/interface/histoStack.h"
#include "TtZAnalysis/Tools/interface/applicationMainMacro.h"
#include "TtZAnalysis/Tools/interface/plotterMultiplePlots.h"

TString findPath ( const TString typeName, const TString histName );

invokeApplication(){

    using namespace std  ;
    using namespace ztop ;

    cout<<endl;

    bool symmetrize = true;

    TString workDir = getenv("FIT_WORKDIR"); 
    TString histName = "HypTopMass";

    histo1D ttsignal ;
    histo1D ttother ;
    histo1D singlet ;
    histo1D diboson ;
    histo1D wjets ;
    histo1D zjets ;

    ttsignal.readFromFile( (string) findPath("ttsignal",histName) );
    ttother.readFromFile( (string) findPath("ttother",histName) );
    singlet.readFromFile( (string) findPath("singlet",histName) );
    diboson.readFromFile( (string) findPath("diboson",histName) );
    wjets.readFromFile( (string) findPath("wjets",histName) );
    zjets.readFromFile( (string) findPath("zjets",histName) );

    histo1D signal = ttsignal + ttother ;
    histo1D bgsum = singlet + diboson + zjets + wjets ;

    signal.addErrorContainer("signalxsec_up",signal*1.5);
    signal.addErrorContainer("signalxsec_down",signal*0.5);

    bgsum.addErrorContainer("bgnorm_up",bgsum*1.3);
    bgsum.addErrorContainer("bgnorm_down",bgsum*0.7);


    histo1D mcsum = signal + bgsum;





    // string pathInFile = (string) (workDir+"/ztopFiles/rawHistos/emu/ttsignal/emu_ttsignal_HypToppT.ztop");  
    // string pathInFile = (string) (workDir+"/ztopFiles/rawHistos/emu/ttsignal/emu_ttsignal_HypLeptonBjetMass.ztop");  

    // histo1D h ;
    // h.readFromFile( pathInFile );
    
    // size_t sistSize = h.getSystSize() ;
    // cout<<sistSize<<endl;

    std::vector<TString> systName_vec = mcsum.getSystNameList();

    for ( TString systName : systName_vec ){    

        // histo1D h_data = h.getDataContainer();
        // histo1D h_sig  = h.getSignalContainer();
        // histo1D h_nkg  = h.getBackgroundContainer();
        
        // int bin = 10;
        TH1D * temp = mcsum.getTH1D();
        int bin = temp->GetMaximumBin();



        // cout<<mcsum.getBinContent(bin)<<endl;
        // cout<<mcsum.getBinErrorUp(bin,false,systName)<<endl;
        // cout<<mcsum.getBinErrorUp(bin,true,"")<<endl;

        double err_u = pow(pow(mcsum.getBinErrorUp(bin,false,systName),2.) - pow(mcsum.getBinErrorUp(bin,true,""),2.),.5);
        double err_d = pow(pow(mcsum.getBinErrorDown(bin,false,systName),2.) - pow(mcsum.getBinErrorDown(bin,true,""),2.),.5);

        if (symmetrize){
            cout<<setprecision(2);
            if (systName =="had" || systName =="mod" )
                cout<<systName<<" & "<<(err_u+err_d)/mcsum.getBinContent(bin)*100<<" \\\\ "<<endl;

            else
                cout<<systName<<" & "<<0.5*(err_u+err_d)/mcsum.getBinContent(bin)*100<<" \\\\ "<<endl;
        }

        else
            cout<<setw(15)<<"syst:\t"<<setw(15)<<systName<<"\t"<<setw(15)<<err_u/mcsum.getBinContent(bin)*100<<"\t"<<setw(15)<<err_d/mcsum.getBinContent(bin)*100<<endl;


        // if (symmetrize){
        //     if (systName =="had" || systName =="mod" )
        //         cout<<setw(15)<<"syst:\t"<<setw(15)<<systName<<"\t"<<setw(15)<<(err_u+err_d)/mcsum.getBinContent(bin)*100<<endl;

        //     else
        //         cout<<setw(15)<<"syst:\t"<<setw(15)<<systName<<"\t"<<setw(15)<<0.5*(err_u+err_d)/mcsum.getBinContent(bin)*100<<endl;
        // }

        // else
        //     cout<<setw(15)<<"syst:\t"<<setw(15)<<systName<<"\t"<<setw(15)<<err_u/mcsum.getBinContent(bin)*100<<"\t"<<setw(15)<<err_d/mcsum.getBinContent(bin)*100<<endl;


        // cout<<setw(15)<<"syst:\t"<<setw(15)<<systName<<"\t"<<setw(15)<<err_u<<"\t"<<setw(15)<<err_d<<endl;

        delete temp;

    }
    // for ( size_t iSyst = 0; iSyst < sistSize; ++iSyst ){

    //     TString systName = h.getSystErrorName(iSyst);
        
    //     double err_u = -1.;
    //     double err_d = -1.;

    //     if (systName.Contains("_up")) continue;
        
    //     systName.ReplaceAll("_down","");

    //         err_u = pow(pow(h.getBinErrorUp(1,false,systName),2.) - pow(h.getBinErrorUp(1,true,""),2.),.5);
    //         err_d = pow(pow(h.getBinErrorDown(1,false,systName),2.) - pow(h.getBinErrorDown(1,true,""),2.),.5);

    //         // double err_symm = .5 * pow(err_u*err_u + err_d*err_d,.5);
    //         double err_symm = .5 * (err_u + err_d);

    //         cout<<setw(15)<<systName<<"\t"<<setw(15)<<h.getBinContent(1)<<"\t"<<setw(15)<<err_u<<"\t"<<setw(15)<<err_d<<"\t"<<setw(15)<<err_symm<<endl;

    // }


    // double err_u = pow(pow(h.getBinErrorUp(1,false,""),2.) - pow(h.getBinErrorUp(1,true,""),2.),.5);
    // double err_d = pow(pow(h.getBinErrorDown(1,false,""),2.) - pow(h.getBinErrorDown(1,true,""),2.),.5);
    // double err_symm = .5 * (err_u + err_d);

    // cout<<setw(15)<<"total\t"<<setw(15)<<h.getBinContent(1)<<"\t"<<setw(15)<<err_u<<"\t"<<setw(15)<<err_d<<"\t"<<setw(15)<<err_symm<<endl;

    // cout<<endl;

    // h = h_sig;

    // err_u = pow(pow(h.getBinErrorUp(1,false,""),2.) - pow(h.getBinErrorUp(1,true,""),2.),.5);
    // err_d = pow(pow(h.getBinErrorDown(1,false,""),2.) - pow(h.getBinErrorDown(1,true,""),2.),.5);
    // err_symm = .5 * (err_u + err_d);

    // cout<<setw(15)<<"signal\t"<<setw(15)<<h.getBinContent(1)<<"\t"<<setw(15)<<err_u<<"\t"<<setw(15)<<err_d<<"\t"<<setw(15)<<err_symm<<endl;

    // err_u = pow(pow(h.getBinErrorUp(1,false,"mass"),2.) - pow(h.getBinErrorUp(1,true,""),2.),.5);
    // err_d = pow(pow(h.getBinErrorDown(1,false,"mass"),2.) - pow(h.getBinErrorDown(1,true,""),2.),.5);
    // err_symm = .5 * (err_u + err_d);


    // cout<<setw(15)<<"mass\t"<<setw(15)<<h.getBinContent(1)<<"\t"<<setw(15)<<err_u<<"\t"<<setw(15)<<err_d<<"\t"<<setw(15)<<err_symm<<endl;

    // h = h_oth;

    // err_u = pow(pow(h.getBinErrorUp(1,false,""),2.) - pow(h.getBinErrorUp(1,true,""),2.),.5);
    // err_d = pow(pow(h.getBinErrorDown(1,false,""),2.) - pow(h.getBinErrorDown(1,true,""),2.),.5);
    // err_symm = .5 * (err_u + err_d);

    // cout<<setw(15)<<"other\t"<<setw(15)<<h.getBinContent(1)<<"\t"<<setw(15)<<err_u<<"\t"<<setw(15)<<err_d<<"\t"<<setw(15)<<err_symm<<endl;

    // err_u = pow(pow(h.getBinErrorUp(1,false,"mass"),2.) - pow(h.getBinErrorUp(1,true,""),2.),.5);
    // err_d = pow(pow(h.getBinErrorDown(1,false,"mass"),2.) - pow(h.getBinErrorDown(1,true,""),2.),.5);
    // err_symm = .5 * (err_u + err_d);


    // cout<<setw(15)<<"mass\t"<<setw(15)<<h.getBinContent(1)<<"\t"<<setw(15)<<err_u<<"\t"<<setw(15)<<err_d<<"\t"<<setw(15)<<err_symm<<endl;


    cout<<endl;
    return 0;

}

TString findPath ( const TString typeName, const TString histName ){

    TString workDir = getenv("FIT_WORKDIR"); 
    TString path = workDir+"/ztopFiles/rawHistos/emu/"+typeName+"/emu_"+typeName+"_"+histName+".ztop";

    return path;
}
