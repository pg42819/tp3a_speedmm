#!/usr/bin/env bash

# interactive, 1 processor, 1 hour
qsub -lnodes=1:r641,walltime=60:00 -I
# Get 1 node from 662 from the mei queue for 1h30 interactive
#qsub -I -qmei -lnodes=1:r662,ppn=1 -lwalltime=1:30:00
