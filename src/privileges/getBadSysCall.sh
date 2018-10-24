ausyscall `sudo cat /var/log/audit/audit.log | grep sig=31 | grep \
comm=\"$1\" | awk '{ for(i=1; i <= NF; i++) {print $i } }' | grep \
syscall | awk -F'=' '{print $2}' | tail -1`
