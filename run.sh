#!/bin/sh
if [ -z "$1" ]; then
  out=results
else
  out=$1
fi
echo "HMM dummy test suite run results (`date`)" > $out
tests=`find . -maxdepth 1 -type f -executable | grep -v run.sh | grep -v kload.sh`
for i in $tests ; do
  $i
  if [ $? -ne 0 ]; then
    echo "(EE) `basename $i`" >> $out
  else
    echo "(OK) `basename $i`" >> $out
  fi
done
