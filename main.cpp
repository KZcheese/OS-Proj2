#include <iostream>
#include <stdio.h>
#include <vector>
#include "process.h"
void printMemory(std::vector<char> memory){
  std::cout << "================================" << std::endl;
  for (int i = 0; i < 256; i++) {
    if (i % 32 == 0) {
      std::cout << std::endl;
    }
    std::cout << memory[i];
  }
  std::cout << std::endl;
  std::cout << "================================" << std::endl;
}

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