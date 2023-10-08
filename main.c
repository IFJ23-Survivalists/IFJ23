/// @file main.c
/// @author Le Duy Nguyen, xnguye27, VUT FIT
/// @date 08/10/2023
/// @brief Main program of the IFJ23

#include "error.h"
#include "scanner.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        eprint("The argument count must be exactly 1\n");
        return 99;
    }

    FILE *file = fopen(argv[1], "r+");

    if (!file) {
        eprintf("Unable to read file %s\n", argv[1]);
        return 99;
    }

    // TODO do something with the file we just got

    return got_error();
}
