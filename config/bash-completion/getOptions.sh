#!/bin/bash

_elementIn () {
  local e match="$1"
  shift
  for e; do [[ "$e" == "$match" ]] && return 0; done
  return 1
}

declare -A suboptions
_getOptions() {
IFS=$'\r\n' GLOBIGNORE='*' command eval 'optLines=($($1 -h | grep "^[[:space:]]*-"))'
usage="$($1 -h | grep Usage:)"
usage=${usage#*\[OPTION...\]}
#might add something for ACCOUNT_SHORTNAME here
usage=$(echo $usage | sed -r -e 's/^.*\[?ACCOUNT_SHORTNAME\]?[[:space:]]*//')
IFS=$'\r\n' GLOBIGNORE='*' command eval 'singleOpts=($(echo "${usage}"  | sed "s/|/\n/g"| sed -e "s/^[[:space:]]*//" -e "s/[[:space:]]*$//"))'
opts=""
singleOptsLong="--help#--usage#--version"
for var in "${optLines[@]}"
do
  IFS=$'\r\n' GLOBIGNORE='*' command eval 'elements=($(echo "${var}"  | sed -e "s/^[[:space:]]*//" -e "s/[[:space:]]*$//"| sed "s/[[:space:]]/\n/g"))'
  shortname=""
  for el in "${elements[@]}"
  do
    if [[ $el == --* ]] ; then
      longname=$el
    fi
    if [[ $el == -[a-zA-Z], ]]; then
      shortname="${el:0:-1}"
    fi
  done
  if  _elementIn $shortname "${singleOpts[@]}"; then
    singleOptsLong+="#${longname}"
  fi
  if [[ $longname == *=* ]]; then
    if [[ $longname == *\[=*\] ]]; then
      longparam="${longname%\[=*}="
      opts+="${longname%\[=*} #"
      argument=${longname#*\[=}
      argument=${argument:0:-1}
    else
      longparam="${longname%=*}="
      argument=${longname#*=}
    fi
    suboptions[$longparam]="$argument"
    # echo "$longparam - $argument"
    # echo "${suboptions[$longparam]}"
  else
    longparam="$longname "
  fi
  opts+="$longparam#"
done

}

# _getOptions $1
