#!/bin/bash
if [ -z "$PINTOOL" ]; then
    echo "Error PINTOOL env var not set"
    exit 1
fi
TOOLNAME="$PINTOOL.so"

CURRENT_DIR=$(pwd)
RUNDIR=$CURRENT_DIR/run
if [ ! -d "$RUNDIR" ]; then
    echo "Error: Run directory does not exist"
    exit 1
fi

PINBALLS_DIR=$RUNDIR/pinballs 
if [ ! -d "$PINBALLS_DIR" ]; then
    echo "Error: pinballs directory does not exist"
    exit 1
fi  

SDEROOT=/opt/intel_sde/
if [ ! -d "$SDEROOT" ]; then
    echo "Error: SDE_ROOT not set"
    exit 1
fi

make clean
make PIN_ROOT=$SDEROOT/pinkit CC="gcc" CXX="g++" obj-intel64/$TOOLNAME

TOOLDIR=$CURRENT_DIR/obj-intel64
if [ ! -d "$TOOLDIR" ]; then
    echo "Error: Tool directory not created, pin tool build probably failed"
    exit 1
fi
sudo cp $TOOLDIR/$TOOLNAME $SDEROOT/intel64

cd $RUNDIR
declare -a pinball_names 

for item in "$PINBALLS_DIR"/*/; do
    if [ -d "$item" ]; then
        pinball_names+=("$(basename "$item")")
    fi
done

TRACE_DIR="${RUNDIR}/traces"
if [ ! -d "${TRACE_DIR}" ]; then 
    mkdir -p $TRACE_DIR
fi
    export TRACE_DIR

for item in "${pinball_names[@]}"; do 
    $SDEROOT/sde64 -mix -replay -replay:basename $PINBALLS_DIR/$item/bwaves_0.ref_2092028   -replay:addr_trans -t $TOOLNAME  -o "${TRACE_DIR}/${item}.trace" -- /opt/intel_sde/intel64/nullapp
done 


