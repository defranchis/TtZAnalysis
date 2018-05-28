# ntoys=10
# njobs=20

ntoys=150
njobs=250

echo
echo submitting $njobs jobs with $ntoys toys each
echo

mkdir -p logs
cd logs
for seed in `seq 1 $njobs`; do
    echo "submitting $ntoys toys with seed $seed"
    qsub -cwd -m as -M matteo.defranchis@desy.de -l h_vmem="10G" ../job.sh $ntoys $seed
done

cd -
