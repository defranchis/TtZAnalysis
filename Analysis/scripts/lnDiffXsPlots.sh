
read DIFFXSPATH < $FIT_WORKDIR/configs/diffXsFwk/diffXsDirPath.txt

if [ -e $FIT_WORKDIR/diffXsPlots ]; then
    unlink diffXsPlots
fi

ln -s  $DIFFXSPATH/Plots diffXsPlots

