

############### SETUP ##################

massfit=true

input_dir=20180802_1156_tW_norm/output


if [ "$massfit" == true ]; then
    config=emu_mtop_13TeV_preapp_rebin_mlb_more.txt
    input_dir_pre=/nfs/dust/cms/user/defrancm/MassExtraction2016/Till/CMSSW_8_0_26_patch2/src/TtZAnalysis/Analysis/workdir
    inputfiles=emu_13TeV_172.5_nominal_syst.ztop
    nToys=150
    nJobs=250
elif [ "$massfit" == false ]; then
    config=datasets_13_TeV.txt
    input_dir_pre=/nfs/dust/cms/user/tarndt/AnalysisWorkdir
    inputfiles=*_13TeV_172.5_nominal_syst.ztop #use wildcards if more files needed
    nToys=50
    nJobs=750
else
    echo
    echo "ERROR: invalid massfit option. Allowed values are \"true\" and \"false\""
    echo
    return
fi

echo
echo
echo "running $nJobs jobs with $nToys toys each"
echo
echo "mass fit = $massfit"
echo "fit config file: $config"
echo
echo "input directory: $input_dir_pre/$input_dir"
echo "input files: $inputfiles"


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

if ls ../toys_workdir/*.tex >/dev/null 2>&1; then
    echo
    echo "first move .tex files in toys_workdir to some other directory!"
    echo "STOPPING"
    echo
else

    cd ../toys_workdir
    if ls *.ztop >/dev/null 2>&1; then
        echo
        for tempfile in *.ztop ; do
            unlink $tempfile
            echo "INFO: unlinked file $tempfile"
        done
        echo
    fi

    ln -s $input_dir_pre/$input_dir/$inputfiles .
    cd ../submit_toys

    echo
    echo "toys are ready to be submitted: please check the parameters above first"
    echo
    echo "to submit: condor_submit submit_HTCondor.submit"
    echo
    echo
fi