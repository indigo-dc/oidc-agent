#!/bin/bash


if [ "$#" -ne 3 ] && [ "$#" -ne 2 ]; then
    echo "Usage: $0 PATTERN IN_FILE [OUT_FILE]"
    exit 1
fi

PATTERN=$1
IN_FILE=$2
OUT_FILE=$3
REPLACE_IN_FILE=0

if [ -z "$OUT_FILE" ]; then
  OUT_FILE="/tmp/file-includer-$RANDOM"
  REPLACE_IN_FILE=1
fi

if ! grep "$PATTERN" $IN_FILE >/dev/null; then
  if [ $REPLACE_IN_FILE -eq 0 ]; then
    cp $IN_FILE $OUT_FILE
  fi
  exit
fi
sed -n "/$PATTERN/!p;//q" $IN_FILE  >$OUT_FILE
echo "$(</dev/stdin)" >>$OUT_FILE
sed "1,/$PATTERN/d" $IN_FILE >>$OUT_FILE

if [ $REPLACE_IN_FILE -ne 0 ]; then
  mv $OUT_FILE $IN_FILE
fi
