#include <string.h>
#include <unistd.h>

int main() {
    char alph[10] = "ABCDEFGHI\n";

    int pid = fork();
    fork();

    sleep(10);

    if (pid) {
        write(STDOUT_FILENO, alph, strlen(alph));
    }
}
