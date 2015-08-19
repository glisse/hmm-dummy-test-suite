#!/bin/bash
if [ -z "$1" ]; then
  out=results
else
  out=$1
fi
echo "HMM dummy test suite run results (`date`)" > $out
tests=`find . -maxdepth 1 -type f -executable -name sanity\* | grep -v run\* | grep -v kload.sh`
for i in $tests ; do
  $i
  if [ $? -ne 0 ]; then
    echo "(EE) `basename $i`" >> $out
  else
    echo "(OK) `basename $i`" >> $out
  fi
done
