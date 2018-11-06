#!/bin/sh
#
#(make sure the right shell will be used)
#$ -S /bin/sh
#$ -l site=hh
#$ -l distro=sld6
#
#(the cpu time for this job)
#$ -l h_rt=20:00:00
#
#(the maximum memory usage of this job)
#$ -l h_vmem=7047M
#
#(stderr and stdout are merged together to stdout)
#$ -j y
#$ -m a
#$ -cwd -V
#( -l h_stack=4536M) #try with small stack
#$ -pe make 6 -R y
#$ -P af-cms

#don't overthread the wn
export OMP_NUM_THREADS=1

workdir=##WORKDIR##

cd $workdir
export ANALYSE_BATCH_BASE=$workdir

exec > "stdout/##OUTNAME##" 2>&1

export LD_LIBRARY_PATH=##WORKDIR##/lib:$LD_LIBRARY_PATH

./bin/analyse ##PARAMETERS##


#runthe executable here
