#!/bin/bash

_matchFiles() {
    local IFS=$'\n'
    local LASTCHAR=' '

    COMPREPLY=($(compgen -o plusdirs -f  -- "$1"))

    if [ ${#COMPREPLY[@]} = 1 ]; then
        [ -d "$COMPREPLY" ] && LASTCHAR=/
        COMPREPLY=$(printf %q%s "$COMPREPLY" "$LASTCHAR")
    else
        for ((i=0; i < ${#COMPREPLY[@]}; i++)); do
            [ -d "${COMPREPLY[$i]}" ] && COMPREPLY[$i]=${COMPREPLY[$i]}/
        done
    fi

    return 0
}

_suboption() {
  ret=0
  for key in "${!suboptions[@]}";
  do
    if [[ "$key" == "$1" ]]; then
      option="${suboptions[$key]}"
      case $option in
        LIFETIME[[:space:]])
          local value=999999999999
          COMPREPLY=( $( compgen -W "{${value:1:((${#value}-2))}}" \
                      -- "$cur" ) )
          ;;
        FILE[[:space:]])
          _matchFiles ${cur}
          ;;
        [[:upper:]_]*)
          ;;
        [[:lower:]]*)
          local IFS=$'|\n'
          COMPREPLY=( $(compgen -W "${option}" -- ${cur}) )
          ;;
      esac
      ret=1
      break
    fi
  done
}
