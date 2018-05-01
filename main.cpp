#include <iostream>
#include <cstdio>
#include "process.h"
#include <fstream>
#include <sstream>


const int frames = 256;
const int fPerLine = 32;

std::vector<Process> processes;

void nextFit() {}

void bestFit() {}

// you can just not use this if you dont want to
void worstFit() {}

void nonContiguous() {}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "%s\n%s\n", "ERROR: Invalid arguments",
                "USAGE: ./a.out <input-file> ");
        return EXIT_FAILURE;
    }

    std::ifstream in(argv[1]);
    std::string line;
    std::string dash = "/";

    while (getline(in, line)) {
        if (line[0] == '#' || line[0] == '\0') continue;
        char id;
        int size, arrTime, runTime;
        std::vector<int> arrTimes;
        std::vector<int> runTimes;

        std::string frac;
        std::istringstream split(line);
        split >> id >> size;
        while (split >> frac) {
            arrTime = stoi(frac.substr(0, frac.find(dash)), nullptr, 10);
            runTime = stoi(frac.substr(frac.find(dash) + 1, frac.length()), nullptr, 10);
            arrTimes.push_back(arrTime);
            runTimes.push_back(runTime);
        }
        processes.emplace_back(Process(id, size, arrTimes, runTimes));
    }
    in.close();
    return 0;
}