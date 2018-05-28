

############### SETUP ##################

nToys=150
nJobs=250

massfit=true

config=emu_mtop_13TeV_preapp_rebin_mlb_more.txt

input_dir_pre=/nfs/dust/cms/user/defrancm/MassExtraction2016/Till/CMSSW_8_0_26_patch2/src/TtZAnalysis/Analysis/workdir
input_dir=20180525_btag_breakdown_merge

inputfiles=emu_13TeV_172.5_nominal_syst.ztop #use wildcards if more files needed
cmssw=$CMSSW_BASE

#########################################


cp templates/submit_HTCondor.submit .
cp templates/job_HTCondor.sh .

sed -i "s;##CMSSW_DIR##;$CMSSW_BASE;g" job_HTCondor.sh
chmod u+x job_HTCondor.sh

sed -i "s/#NTOYS#/$nToys/"      submit_HTCondor.submit
sed -i "s/#NJOBS#/$nJobs/"      submit_HTCondor.submit
sed -i "s/#CONFIG#/$config/"    submit_HTCondor.submit
sed -i "s/#MASSFIT#/$massfit/"  submit_HTCondor.submit

if [ -f *~ ]; then
    rm *~
fi

mkdir -p outputHTCondor
mkdir -p ../toys_workdir

if [ -f ../toys_workdir/*tex ]; then
    echo
    echo "first move .tex files in toys_workdir to some other directory!"
    echo "STOPPING"
    echo
else

    cd ../toys_workdir
    if [ -f *.ztop ]; then
        mkdir -p zz_temp_softlinks
        mv *.ztop zz_temp_softlinks
        cd zz_temp_softlinks
        find -type l -delete
        cd ..
    fi
    ln -s $input_dir_pre/$input_dir/$inputfiles .
    cd ../submit_toys

    echo
    echo "toys are ready to be submitted"
    echo
    echo "to submit: condor_submit submit_HTCondor.submit"
    echo

fi