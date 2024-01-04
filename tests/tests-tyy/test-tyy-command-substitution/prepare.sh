cat > my_cat <<"EOF"
#!/bin/bash
F="$1"
if [ -z "$F" ]; then
  printf "Usage: ./my_cat FILE\n" >&2
  exit 1
fi
if ! [ -e "$F" ]; then
  printf "Error: file not found: \"$F\"\n" >&2
  exit 1
fi
if ! [ -p "$F" ]; then
  printf "Error: not a pipe or a fifo: \"$F\"\n" >&2
  exit 1
fi
cat "$F"
EOF

printf "BBB\n" > file1
printf "CCC\n" > file2

chmod u+x ./my_cat

