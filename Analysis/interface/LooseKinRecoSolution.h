#ifndef LooseKinRecoSolution_h
#define LooseKinRecoSolution_h

#include "../../../TopAnalysis/Configuration/analysis/common/include/classes.h"
#include <TLorentzVector.h>

class LooseKinRecoSolution{

public:

    /// Default constructor
    LooseKinRecoSolution();

    /// Constructors for setting values of the solution
    LooseKinRecoSolution(const float minmlbKR_, const float mwwKR_, const LV ttbarLLMETTotal_, const bool hasSolution_, const std::pair<TLorentzVector, TLorentzVector> lvBBbar_, const std::pair<TLorentzVector, TLorentzVector> lvLLbar_);

    /// Copy constructor
    LooseKinRecoSolution(const LooseKinRecoSolution& looseKinRecoSolution);

    /// Destructor
    ~LooseKinRecoSolution(){}

    const LooseKinRecoSolution& operator=(const LooseKinRecoSolution& rhs){return rhs;}

    const float& getMinMlb()const{return minmlbKR_;}
    const float& getWWmass()const{return mwwKR_;}
    const std::pair<TLorentzVector, TLorentzVector>& getLVBBbar()const{return lvBBbar_;}
    const std::pair<TLorentzVector, TLorentzVector>& getLVLLbar()const{return lvLLbar_;}
    const LV& TTbar()const{return ttbarLLMETTotal_;}
    const bool& hasSolution()const{return hasSolution_;}




private:

    const float minmlbKR_;
    const float mwwKR_;
    const LV ttbarLLMETTotal_;
    const bool hasSolution_;
    const std::pair<TLorentzVector, TLorentzVector> lvBBbar_;
    const std::pair<TLorentzVector, TLorentzVector> lvLLbar_;

};


#endif
