#!/bin/bash

_getOptions() {
# IFS=$'\r\n' GLOBIGNORE='*' command eval 'optLines=($(cat src/oidc-gen/options.struct | grep {*} | grep -v {0,))'
IFS=$'\r\n' GLOBIGNORE='*' command eval 'optLines=($($1 -h | grep "^[[:space:]]*-"))'

# suggestions=()
opts=""
for var in "${optLines[@]}"
do
  # var=${var:1:-2} # removing curly braces (and trailing comma)
  # echo "${var}"
  # readarray -td, elements <<<"$var,"; unset 'elements[-1]'; declare -p elements;
  IFS=$'\r\n' GLOBIGNORE='*' command eval 'elements=($(echo "${var}"  | sed -e "s/^[[:space:]]*//" -e "s/[[:space:]]*$//"| sed "s/[[:space:]]/\n/g"))'
  for el in "${elements[@]}"
  do
    if [[ $el == --* ]] ; then
      longname=$el
    fi
  done
  if [[ $longname == *=* ]]; then
    if [[ $longname == *[=* ]]; then
      # suggestions+=("${longname%[=*}=")
      # suggestions+=("${longname%[=*} ")
      opts+="${longname%[=*}=#"
      opts+="${longname%[=*} #"
    else
      # suggestions+=("${longname%=*}=")
      opts+="${longname%=*}=#"
    fi
  else
    # suggestions+=("$longname ")
    opts+="$longname #"
  fi
done

# for var in "${suggestions[@]}"
# do
#   echo "${var}"
# done
#
# echo $opts
}
