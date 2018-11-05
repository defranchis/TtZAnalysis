#!/bin/bash
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd ##CMSSW_DIR##/src/
eval `scramv1 runtime -sh`
cd TtZAnalysis/Analysis/toys_workdir

seed=$1
nToys=$2
config=$3
mass_fit=$4

echo "seed -1 =" $1
echo "ntoys =" $2
echo "config =" $3
echo "mass fit =" $4
echo

if [ "$mass_fit" == true ]; then
    fitTtBarXsec -i $config --emu -M --noplots --onlytotal --nToys $nToys --seed $((seed+1))
else
    fitTtBarXsec -i $config --noplots --onlytotal --nToys $nToys --seed $((seed+1))
fi

