#!/bin/bash

rm -rf ./*

mkdir -p .data
mkdir -p files

echo -e "#!/bin/bash\nrm -f files/*" > clean_files
chmod u+x clean_files

cat > .data/util.c <<EOF
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
long int usleep_factor = -1;
int my_usleep(useconds_t microseconds) {
  if (usleep_factor < 0) {
    char * env_usleep_factor = getenv("USLEEP_FACTOR");
    unsigned long int f;
    if (env_usleep_factor != NULL && sscanf(env_usleep_factor, "%lu", &f) == 1) {
      usleep_factor = f;
    } else {
      usleep_factor = 100;
    }
  }
  useconds_t true_microseconds = (microseconds * usleep_factor + 99) / 100;
  return usleep(true_microseconds);
}
EOF

cat > .data/usleep.c <<EOF
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int my_usleep(useconds_t microseconds);
int main(int argc, char ** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: usleep DELAY\n");
  }
  my_usleep(strtol(argv[1], NULL, 10));
  return 0;
}
EOF
gcc -o usleep .data/usleep.c .data/util.c

