rm -rdf ./*

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

cat > speak_friend.c <<"EOF"
#include <stdio.h>

int main(int argc, char *argv[]) {
  printf("Hello\n");
  fprintf(stderr, " mellon\n");
  return 0;
}
EOF

gcc -o speak_friend speak_friend.c

chmod u+x ./speak_friend

chmod u+x ./my_cat

