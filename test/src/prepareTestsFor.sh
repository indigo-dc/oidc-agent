#!/bin/bash

# e.g. utils/stringUtils
NAME=$1

[ -z $1 ] && {
    echo "The source file must be the only parameter"
    exit 1
}
SRC_FILENAME="${NAME}.h"
SRC_FILE="../../src/$SRC_FILENAME"

[ ! -f $SRC_FILE ] && {
  echo "Source file not found."
  exit 1
}

SHORTNAME="${NAME##*/}"
BIGNAME=$(echo "${NAME//\//_}" | tr [a-z] [A-Z])
# echo $SRC_FILENAME
# echo $SRC_FILE
# echo $NAME
# echo $SHORTNAME
# echo $BIGNAME
FUNCTIONNAMES=$(ctags -x --c-kinds=fp $SRC_FILE | awk '{print $1}')

touch /tmp/now
mkdir -p $NAME
cp -n ../template/suite.h ${NAME}/suite.h
cp -n ../template/suite.c ${NAME}/suite.c
for F in $FUNCTIONNAMES; do
  cp -n ../template/tc.h ${NAME}/tc_${F}.h
  cp -n ../template/tc.c ${NAME}/tc_${F}.c
done
cd $NAME
[ ! /tmp/now -nt suite.h ] && sed -i -e 's/\$BIG\$/'"$BIGNAME"'/g' suite.h
[ ! /tmp/now -nt suite.h ] && sed -i -e 's/\$NORMAL\$/'"$SHORTNAME"'/g' suite.h
for F in $FUNCTIONNAMES; do
  BIGF=$(echo $F | tr [a-z] [A-Z])
  [ ! /tmp/now -nt tc_${F}.h ] && sed -i -e 's/\$BIG\$/'"$BIGNAME"'_'"$BIGF"'/g' tc_${F}.h
  [ ! /tmp/now -nt tc_${F}.h ] && sed -i -e 's/\$NORMAL\$/'"$F"'/g' tc_${F}.h
  [ ! /tmp/now -nt tc_${F}.c ] && sed -i -e 's/\$BIG\$/'"$BIGNAME"'_'"$BIGF"'/g' tc_${F}.c
  [ ! /tmp/now -nt tc_${F}.c ] && sed -i -e 's/\$NORMAL\$/'"$F"'/g' tc_${F}.c
  [ ! /tmp/now -nt tc_${F}.c ] && sed -i -e 's!\$SRCHEADER\$!'"$SRC_FILENAME"'!g' tc_${F}.c
  INCLUDES+="#include \"tc_${F}.h\"\n"
  ADDCASES+="\tsuite_add_tcase(ts_${SHORTNAME}, test_case_${F}());\n"
done
# echo $INCLUDES
# echo $ADDCASES
[ ! /tmp/now -nt suite.c ] && sed -i -e 's/\$NORMAL\$/'"$SHORTNAME"'/g' suite.c
[ ! /tmp/now -nt suite.c ] && sed -i -e 's/\$INCLUDES\$/'"$INCLUDES"'/g' suite.c
[ ! /tmp/now -nt suite.c ] && sed -i -e 's/\$ADDCASES\$/'"$ADDCASES"'/g' suite.c

echo "OK. Prepared TestSuite."
echo "You still have to add the test suite to test/src/main.c and implement the tests."
