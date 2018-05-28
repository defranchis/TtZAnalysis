#!/bin/bash
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /nfs/dust/cms/user/defrancm/MassExtraction2016/Till/CMSSW_8_0_26_patch2/src/
eval `scramv1 runtime -sh`
cd TtZAnalysis/Analysis/toys_workdir

nohup fitTtBarXsec -i emu_mtop_13TeV_preapp_rebin_mlb_more.txt --emu -M --noplots --onlytotal --nToys $1 --seed $2 &> log_$2.log
