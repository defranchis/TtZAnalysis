
#include "TtZAnalysis/Tools/interface/histo1D.h"
#include "TtZAnalysis/Tools/interface/applicationMainMacro.h"
#include "TtZAnalysis/Tools/interface/variateHisto1D.h"
#include "TtZAnalysis/Tools/interface/plotterMultiplePlots.h"
#include "TtZAnalysis/Tools/interface/plotterCompare.h"
#include "TtZAnalysis/Tools/interface/texTabler.h"
#include "TMath.h"
#include "TSystem.h"
#include <iostream>
#include <iomanip>
#include <cmath>

TString findPath ( const TString typeName, const TString histName, const TString channel );

invokeApplication(){


    using namespace ztop;
    using namespace std;

    double nominal_xsec = 831.76;
    double nominal_mass = 172.50;
    double lumi_unc = 0.025;

    TString workDir = getenv("FIT_WORKDIR"); 

    TString histName = "HypLeptonBjetMass";


    vector<TString> channel_vec = {"emu","ee","mumu"};

    histo1D data ;
    histo1D ttsignal ;
    histo1D ttother ;
    histo1D singlet ;
    histo1D diboson ;
    histo1D wjets ;
    histo1D zjets ;
    histo1D ttV ;

    for ( TString & channel : channel_vec ) {

        if (channel == "ee") histName = "HypjetMulti";
        if (channel == "mumu") histName = "HypBjetMulti_noBTag";

        if (channel=="emu"){

            data.readFromFile( (string) findPath("data",histName, channel) );
            ttsignal.readFromFile( (string) findPath("ttsignal",histName, channel) );
            ttother.readFromFile( (string) findPath("ttother",histName, channel) );
            singlet.readFromFile( (string) findPath("singlet",histName, channel) );
            diboson.readFromFile( (string) findPath("diboson",histName, channel) );
            wjets.readFromFile( (string) findPath("wjets",histName, channel) );
            zjets.readFromFile( (string) findPath("zjets",histName, channel) );
            ttV.readFromFile( (string) findPath("ttV",histName, channel) );

            data=data.cutRight(200);
            ttsignal=ttsignal.cutRight(200);
            ttother=ttother.cutRight(200);
            singlet=singlet.cutRight(200);
            diboson=diboson.cutRight(200);
            wjets=wjets.cutRight(200);
            zjets=zjets.cutRight(200);
            ttV=ttV.cutRight(200);

        }

        else{

            histo1D data_temp ;
            histo1D ttsignal_temp ;
            histo1D ttother_temp ;
            histo1D singlet_temp ;
            histo1D diboson_temp ;
            histo1D wjets_temp ;
            histo1D zjets_temp ;
            histo1D ttV_temp ;

            data_temp.readFromFile( (string) findPath("data",histName, channel) );
            ttsignal_temp.readFromFile( (string) findPath("ttsignal",histName, channel) );
            ttother_temp.readFromFile( (string) findPath("ttother",histName, channel) );
            singlet_temp.readFromFile( (string) findPath("singlet",histName, channel) );
            diboson_temp.readFromFile( (string) findPath("diboson",histName, channel) );
            wjets_temp.readFromFile( (string) findPath("wjets",histName, channel) );
            zjets_temp.readFromFile( (string) findPath("zjets",histName, channel) );
            ttV_temp.readFromFile( (string) findPath("ttV",histName, channel) );

            data=data.append(data_temp);
            ttsignal=ttsignal.append(ttsignal_temp);
            ttother=ttother.append(ttother_temp);
            singlet=singlet.append(singlet_temp);
            diboson=diboson.append(diboson_temp);
            wjets=wjets.append(wjets_temp);
            zjets=zjets.append(zjets_temp);
            ttV=ttV.append(ttV_temp);
        }

    }


    histo1D signal = ttsignal + ttother ;
    histo1D bgsum = singlet + diboson + zjets + wjets + ttV ;

    signal.addErrorContainer("signalxsec_up",signal*1.5);
    signal.addErrorContainer("signalxsec_down",signal*0.5);

    bgsum.addErrorContainer("bgnorm_up",bgsum*1.3);
    bgsum.addErrorContainer("bgnorm_down",bgsum*0.7);


    histo1D mcsum = signal + bgsum;

    mcsum.addErrorContainer("lumi_up",mcsum*(1.+lumi_unc));
    mcsum.addErrorContainer("lumi_down",mcsum*(1.-lumi_unc));

    variateHisto1D hv;
    hv.import(mcsum);

    // hv.setPrior("mass",  Prior::Gauss, 3.);
    // hv.setPrior("mass",  Prior::Float);
    // hv.setPrior("had",   Prior::BoxAuto  );
    // hv.setPrior("mod",   Prior::BoxAuto  );
    // hv.setPrior("scale", Prior::Box  );
    // hv.setPrior("pdf_*", Prior::Gauss, TMath::ErfInverse(.9), true );

    hv.setPrior("*scale", Prior::Box, 1. , true  );
    hv.setPrior("pdf_*", Prior::Gauss, TMath::ErfInverse(.9), true );
    hv.setPrior("psfsrscale", Prior::Box, sqrt(2.));

    hv.setPrior("mescale", Prior::Fixed);
    hv.setPrior("pdf_29", Prior::Fixed);

    hv.setPrior("mass",  Prior::Float);
    hv.setPrior("signalxsec", Prior::Float );



    hv.printPriors();


    vector<TString> list = mcsum.getSystNameList();
    cout<<endl;

    // simpleFitter result = hv.fitToConstHisto(data,true);
    simpleFitter result = hv.fitToConstHisto(mcsum,true, true);

    double pull_xsec = 0;
    double err_xsec = 0;

    double pull_mass = 0;
    double err_mass = 0;

    bool read_xsec = false;
    bool read_mass = false;

    ofstream outFit ("outdir/fitResults_combined.tex");

    for(size_t i=0;i<result.getParameters()->size();i++){
        std::cout<< i<< " " << list.at(i) << " " << result.getParameter(i) << "  " << result.getParameterErr(i) << std::endl;
        outFit<<list.at(i)<< " & " << result.getParameter(i) << " & " << result.getParameterErr(i)<< "\\\\" << endl;

        if (list.at(i) == "signalxsec"){
            pull_xsec = result.getParameter(i);
            err_xsec = result.getParameterErr(i);
            read_xsec = true;
        }

        if (list.at(i) == "mass"){
            pull_mass = result.getParameter(i);
            err_mass = result.getParameterErr(i);
            read_mass = true;
        }
        

    }

    outFit.close();

    if (!read_xsec || !read_mass){
        cout<<"ERROR"<<endl;
        return 0;
    }



    cout<<fixed<<setprecision(1);



    cout<<endl;
    cout<<"******************************************"<<endl;
    cout<<endl;
    cout<<"fit: tot xsec = "<<nominal_xsec*(1+pull_xsec/2.)<<" +/- "<<err_xsec*nominal_xsec/2.<<" pb  ("<<(err_xsec*nominal_xsec/2.)/(nominal_xsec*(1+pull_xsec/2.))*100<<" %)"<<endl;
    cout<<"fit:  MC mass = "<<nominal_mass + pull_mass*3.<<" +/- "<<err_mass*3.<<" GeV  ("<<(err_mass*3.)/(nominal_mass + pull_mass*3.)*100<<" %)"<<endl;
    cout<<endl;
    cout<<"******************************************"<<endl;
    cout<<endl;

    // cout<<endl;
    // cout<<"**********************************"<<endl;
    // cout<<endl;
    // cout<<"fit: tot xsec = "<<nominal_xsec*(1+pull_xsec/2.)<<" +/- "<<err_xsec*nominal_xsec/2.<<" pb"<<endl;
    // cout<<"fit:  MC mass = "<<nominal_mass + pull_mass*3.<<" +/- "<<err_mass*3.<<" GeV"<<endl;
    // cout<<endl;
    // cout<<"**********************************"<<endl;
    // cout<<endl;


    ofstream out_xSec ("outdir/xSecMassResults_combined.tex");
    out_xSec<<fixed<<setprecision(1);
    out_xSec<<"$\\sigma_{tot} = "<<nominal_xsec*(1+pull_xsec/2.)<<" \\pm "<<err_xsec*nominal_xsec/2.<<" ~\\mathrm{pb} $"<<endl;
    out_xSec<<"$m_{t}^{\\mathrm{MC}} = "<<nominal_mass + pull_mass*3.<<" \\pm "<<err_mass*3.<<" ~\\mathrm{GeV}$"<<endl;



    histo1D postfit = hv.exportContainer( *result.getParameters() );

    histo1D postfit_mlb = postfit.cutRight(180.);
    histo1D data_mlb = data.cutRight(180.);

    postfit_mlb = postfit_mlb.cutLeft(10.);
    data_mlb = data_mlb.cutLeft(10.);

    data_mlb.setName("data");
    postfit_mlb.setName("MC");
    
    plotterCompare pl_mlb;
    pl_mlb.setComparePlot(& postfit_mlb, 0);
    pl_mlb.setNominalPlot(& data_mlb);

    gSystem->Exec("mkdir -p " + workDir + "/output");
    pl_mlb.printToPdf("output/test_combined_mlb");

    texTabler corr = result.makeCorrTable();
    corr.writeToFile("output/tabCorr_combined.tex");
    corr.writeToPdfFile("output/tabCorr_combined.pdf");




    return 0;

}





TString findPath ( const TString typeName, const TString histName, const TString channel ){


    // if (channel == "ee") histName = "HypBjetMulti_noBTag";
    // else if (channel == "mumu") histName = "HypTTBarMass";

    TString workDir = getenv("FIT_WORKDIR"); 
    TString path = workDir+"/ztopFiles/rawHistos/"+channel+"/"+typeName+"/"+channel+"_"+typeName+"_"+histName+".ztop";

    // std::cout<<path<<std::endl;

    return path;
}
