#!/bin/bash

_suboption() {
  for key in "${!suboptions[@]}";
  do
    if [[ "$key" == "$1" ]]; then
      options="${suboptions[$key]}"
      case $option in
        LIFETIME)
          return
          ;;
        FILE)
          return
          ;;
        [A-Z]*)
          return
          ;;
        *)
          return
          ;;
      esac
      return
    fi
  done
}
