#!/bin/bash

rm -rf ./*

mkdir test_dir

echo abcdefghij > fichier1
echo jihgfedcba > fichier2
echo 42 > fichier3

cat > loremipsum <<"EOF"
Lorem ipsum dolor sit amet, officia excepteur ex fugiat reprehenderit enim labore culpa sint ad nisi Lorem pariatur mollit ex esse exercitation amet. 
Nisi anim cupidatat excepteur officia.
Reprehenderit nostrud nostrud ipsum Lorem est aliquip amet voluptate voluptate dolor minim nulla est proident. 
Nostrud officia pariatur ut officia.
Sit irure elit esse ea nulla sunt ex occaecat reprehenderit commodo officia dolor Lorem duis laboris cupidatat officia voluptate. 
Culpa proident adipisicing id nulla nisi laboris ex in Lorem sunt duis officia eiusmod.
Aliqua reprehenderit commodo ex non excepteur duis sunt velit enim. 
Voluptate laboris sint cupidatat ullamco ut ea consectetur et est culpa et culpa duis.
EOF

cat > sprucetree.c <<"EOF"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Usage : %s [0-9]\n", argv[0]);
        return 1;
    }

    srand(time(NULL));
    int nb_rows = atoi(argv[1]);

    for (int row = 0; row < nb_rows; ++row) {
        int spaces = nb_rows - row - 1;
        for (int index = 0; index < spaces; ++index) {
            printf(" ");
        }
        for (int star = 0; star < 2 * row + 1; ++star) {

            if (row == 0) {
                printf("\033[0;33m*");
            } else if (rand() % 6 == 0) {
                switch (rand() % 2) {
                    case 0:
                        printf("\033[0;31m*");
                        break;
                    case 1:
                        printf("\033[0;34m*");
                        break;
                }
            } else {
                printf("\033[0;32m*");
            }
        }
        printf("\n");
    }

    int trunk_width = nb_rows / 3;

    trunk_width = trunk_width < 1 ? 1 : trunk_width;
    trunk_width = trunk_width % 2 == 0 ? trunk_width + 1 : trunk_width;

    int spaces = (nb_rows * 2 - trunk_width) / 2;

    printf("\033[0;31m");
    for (int trunk = 0; trunk < trunk_width; ++trunk) {

        for (int space = 0; space < spaces; ++space) {
            printf(" ");
        }
        for (int star = 0; star < trunk_width; ++star) {
            printf("*");
        }
        printf("\n");
    }
    printf("\033[0m");

    return 0;
}
EOF
gcc -o sprucetree sprucetree.c
