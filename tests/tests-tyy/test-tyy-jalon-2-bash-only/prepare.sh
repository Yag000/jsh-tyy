#!/bin/bash

rm -rf ./*

mkdir test_dir

echo abcdefghij > fichier1
echo jihgfedcba > fichier2
echo 42 > fichier3

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
