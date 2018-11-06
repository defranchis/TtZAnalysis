
#include "TtZAnalysis/Tools/interface/histo1D.h"
#include "TtZAnalysis/Tools/interface/applicationMainMacro.h"
#include "TtZAnalysis/Tools/interface/variateHisto1D.h"
#include "TtZAnalysis/Tools/interface/plotterMultiplePlots.h"
#include "TtZAnalysis/Tools/interface/plotterCompare.h"
#include "TMath.h"
#include <iomanip>

TString findPath ( const TString typeName, const TString histName );

invokeApplication(){


    using namespace ztop;
    using namespace std;

    double nominal_xsec = 831.76;
    double nominal_mass = 172.50;
    double lumi_unc = 0.025;

    TString workDir = getenv("FIT_WORKDIR"); 

    TString histName = "HypLeptonBjetMass";

    // TString histName = "HypjetMulti";
    // TString histName = "HypTopMass";
    // TString histName = "HypBjetMulti_noBTag";
    
    histo1D data ;
    histo1D ttsignal ;
    histo1D ttother ;
    histo1D singlet ;
    histo1D diboson ;
    histo1D wjets ;
    histo1D zjets ;

    data.readFromFile( (string) findPath("data",histName) );
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

    mcsum.addErrorContainer("lumi_up",mcsum*(1.+lumi_unc));
    mcsum.addErrorContainer("lumi_down",mcsum*(1.-lumi_unc));

    // if (histName == "HypLeptonBjetMass"){
    //     data.rebin(5);
    //     mcsum.rebin(5);
    // }

    variateHisto1D hv;
    hv.import(mcsum);


    hv.setPrior("mass",  Prior::Box);
    // hv.setPrior("had",   Prior::BoxAuto  );
    // hv.setPrior("mod",   Prior::BoxAuto  );
    hv.setPrior("*scale", Prior::Box, 1. , true  );
    hv.setPrior("pdf_*", Prior::Gauss, TMath::ErfInverse(.9), true );

    hv.setPrior("signalxsec", Prior::Float );

    hv.printPriors();



    // size_t nsys = hv.getNDependencies();
    vector<TString> list = mcsum.getSystNameList();
    // cout<<nsys<<endl;
    cout<<endl;
    // cout<<list.size()<<endl;

    // cout<<endl;
    // for (auto var : list) cout<<var<<endl;

    // simpleFitter result = hv.my_fitToConstHisto(data);
    // simpleFitter result = hv.my_fitToConstHisto(mcsum);

    simpleFitter result = hv.fitToConstHisto(data,true);
    // simpleFitter result = hv.fitToConstHisto(mcsum,true);

    // std::vector<TString>* names = result.getParameterNames();
    // cout<<names->size()<<endl;

    // cout<<endl<<"size"<<endl;
    // cout<<result.getParameters()->size()<<endl;
    // cout<<result.getParameterNames()->size()<<endl;

    double pull_xsec = 0;
    double err_xsec = 0;

    double pull_mass = 0;
    double err_mass = 0;

    bool read_xsec = false;
    bool read_mass = false;



    for(size_t i=0;i<result.getParameters()->size();i++){
        // std::cout << names.at(i) << " " << result.getParameter(i) << "  " << result.getParameterErr(i) << std::endl;
        // std::cout << result.getParameter(i) << "  " << result.getParameterErr(i) << std::endl;
        std::cout<< i<< " " << list.at(i) << " " << result.getParameter(i) << "  " << result.getParameterErr(i) << std::endl;

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


    if (!read_xsec || !read_mass){
        cout<<"ERROR"<<endl;
        return 0;
    }



    cout<<fixed<<setprecision(1);


    cout<<endl;
    cout<<"**********************************"<<endl;
    cout<<endl;
    cout<<"fit: tot xsec = "<<nominal_xsec*(1+pull_xsec/2.)<<" +/- "<<err_xsec*nominal_xsec/2.<<" pb"<<endl;
    cout<<"fit:  MC mass = "<<nominal_mass + pull_mass*3.<<" +/- "<<err_mass*3.<<" GeV"<<endl;
    cout<<endl;
    cout<<"**********************************"<<endl;
    cout<<endl;




    histo1D postfit = hv.exportContainer( *result.getParameters() );

    postfit = postfit.cutRight(200.);
    data = data.cutRight(200.);


    plotterCompare pl;
    pl.setComparePlot(& postfit, 0);
    pl.setNominalPlot(& data);
    
    pl.printToPdf("test");


    return 0;

}




TString findPath ( const TString typeName, const TString histName ){

    TString workDir = getenv("FIT_WORKDIR"); 
    TString path = workDir+"/ztopFiles/rawHistos/emu/"+typeName+"/emu_"+typeName+"_"+histName+".ztop";

    return path;
}
