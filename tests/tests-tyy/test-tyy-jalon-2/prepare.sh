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

int main(int argc, char **argv) {

    if (argc != 2) {
        printf("Usage : %s [0-9]\n", argv[0]);
        return 1;
    }

    int nb_rows = atoi(argv[1]);

    for (int row = 0; row < nb_rows; ++row) {
        int spaces = nb_rows - row - 1;
        for (int index = 0; index < spaces; ++index) {
            printf(" ");
        }
        for (int star = 0; star < 2 * row + 1; ++star) {
            printf("*");
        }
        printf("\n");
    }

    return 0;
}
EOF
gcc -o sprucetree sprucetree.c
