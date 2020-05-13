#!/bin/bash

export OMP_NUM_THREADS=1

workdir=##WORKDIR##

cd $workdir

source /cvmfs/cms.cern.ch/cmsset_default.sh
cd ../..
eval `scramv1 runtime -sh`
cd -

export ANALYSE_BATCH_BASE=$workdir

exec > "stdout/$1_$3_$4_$2" 2>&1

export LD_LIBRARY_PATH=##WORKDIR##/lib:$LD_LIBRARY_PATH

./bin/analyse -c $1 -s $2 -e $3 -mt $4 $5 --kinReco --fullPS --nCores 3
