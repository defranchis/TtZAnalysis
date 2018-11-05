#ifndef NTBTagSF_H
#define NTBTagSF_H
#include "TopAnalysis/ZTopUtils/interface/bTagSFBase.h"
#include "TtZAnalysis/DataFormats/interface/NTJet.h"
#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
//#include "mathdefs.h"
#include <iostream>

namespace ztop{

class NTBTagSF : public bTagSFBase{
public:
    NTBTagSF():bTagSFBase(true,true),isMC_(true),mode_(simplereweighting_mode){}
    ~NTBTagSF(){}

    enum modes{randomtagging_mode,shapereweighting_mode,simplereweighting_mode};
    void setMode(modes mode){mode_=mode;}
    modes getMode()const{return mode_;}



    void fillEff(ztop::NTJet * jet,const float& puweight){ //!overload: jet, puweight
        bTagSFBase::fillEff(jet->pt(), jet->eta(), jet->getMember(1), jet->btag(), puweight);
    }

    bool makesEff()const{return bTagSFBase::getMakeEff();}


    void setIsMC(bool is){isMC_=is;}


    void changeNTJetTags( std::vector<ztop::NTJet *> &)const;
    void changeNTJetTags( std::vector<ztop::NTJet *> *)const;

    float getEventWeightSimple ( std::vector<ztop::NTJet *> &);
    float getEventWeightSimple ( std::vector<ztop::NTJet *> *);

    void writeToFile(const std::string&); //! creates and writes histograms
    void readFromFile(const std::string&);  //! reads histograms
    void readReferenceFromFile(const std::string&);  //! reads reference histograms


private:
    NTBTagSF(const ztop::NTBTagSF&rhs):bTagSFBase(true,true),isMC_(true),mode_(simplereweighting_mode){}
    NTBTagSF& operator=(const ztop::NTBTagSF&rhs){return *this;}

    bool isMC_;
    modes mode_;
};
}


#endif
