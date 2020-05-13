#include <iostream>
#include <iomanip>

#include "../../../TopAnalysis/Configuration/analysis/common/include/classes.h"
#include "../interface/LooseKinRecoSolution.h"


// -------------------------------------- Methods for LooseKinRecoSolution --------------------------------------


LooseKinRecoSolution::LooseKinRecoSolution():
minmlbKR_(0),
mwwKR_(0),
ttbarLLMETTotal_(LV(0.,0.,0.,0.)),
hasSolution_(false),
lvBBbar_(std::make_pair(TLorentzVector(0.,0.,0.,0.),TLorentzVector(0.,0.,0.,0.))),
lvLLbar_(std::make_pair(TLorentzVector(0.,0.,0.,0.),TLorentzVector(0.,0.,0.,0.)))
{}



LooseKinRecoSolution::LooseKinRecoSolution(const LooseKinRecoSolution& looseKinRecoSolution):
minmlbKR_(looseKinRecoSolution.minmlbKR_),
mwwKR_(looseKinRecoSolution.mwwKR_),
ttbarLLMETTotal_(looseKinRecoSolution.ttbarLLMETTotal_),
hasSolution_(looseKinRecoSolution.hasSolution_),
lvBBbar_(looseKinRecoSolution.lvBBbar_),
lvLLbar_(looseKinRecoSolution.lvLLbar_)
{}


LooseKinRecoSolution::LooseKinRecoSolution(const float minmlbKR_, const float mwwKR_, const LV ttbarLLMETTotal_, const bool hasSolution_, const std::pair<TLorentzVector, TLorentzVector> lvBBbar_, const std::pair<TLorentzVector, TLorentzVector> lvLLbar_):
minmlbKR_(minmlbKR_),
mwwKR_(mwwKR_),
ttbarLLMETTotal_(ttbarLLMETTotal_),
hasSolution_(hasSolution_),
lvBBbar_(lvBBbar_),
lvLLbar_(lvLLbar_)
{}
