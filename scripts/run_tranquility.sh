#!/bin/bash

# run_tranquility.sh from to
# Runs "tranquility n" for n from half-open interval [from, to)

USAGE="Usage: run_tranquility.sh expects two integer parameters: run_tranquility.sh from to"

# Number of arguments has to be 2
if [ $# != 2 ]; then
	echo $USAGE
	exit 1
fi

# Arguments have to be numeric
if [[ ! "$1" =~ ^[0-9]+$ ]] || [[ ! "$2" =~ ^[0-9]+$ ]]; then
	echo $USAGE
	exit 1
fi

let UPBOUND=$2-1
COUNT=0

for i in `seq $1 $UPBOUND`; do
	# Print progress info for every 10th graph
	if (( ( "$i" % 50 ) == "0" )); then
		echo "Processing graph id $i..."
	fi
	./tranquility $i
	if [ $? != 0 ]; then
		echo "Script terminated at graph id $i."
		echo "$COUNT graphs successfully processed."
		exit 2
	fi
	let COUNT=$COUNT+1
done

echo "$COUNT graphs successfully processed."
exit 0
