#include <iostream>
#include <stdio.h>

void nextFit() {}
void bestFit() {}
// you can just not use this if you dont want to
void worstFit() {}
void nonContiguous() {}

int main(int argc, char * argv[]) {
    if (argc != 2){
        fprintf(stderr, "%s\n%s\n", "ERROR: Invalid arguments",
                "USAGE: ./a.out <input-file> ");
        return EXIT_FAILURE;
    }
    std::cout << "Hello, World!" << std::endl;
    return 0;
}