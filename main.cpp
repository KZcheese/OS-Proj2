#include <iostream>
#include <cstdio>
#include "process.h"
#include <fstream>
#include <sstream>
#include <vector>

const int frames = 256;
const int fPerLine = 32;

std::vector<Process> processes;

void printMemory(std::vector<char> memory) {
    std::cout << "================================" << std::endl;
    for (int i = 0; i < frames; i++) {
        if (i % fPerLine == 0) {
            std::cout << std::endl;
        }
        std::cout << memory[i];
    }
    std::cout << std::endl;
    std::cout << "================================" << std::endl;
}

void nextFit() {}

void bestFit() {}

void worstFit() {
    int clock = 0;
    std::vector<char> memory(frames, '.');
    std::vector<Process> inactive(processes);
    std::vector<Process> active;
    std::vector<std::pair<int, int>> gaps;
    gaps.push_back(std::pair<int, int>(0, frames));


    int maxGapSize = 0;
    int maxGap = -1;
    while(true) {
        std::vector<Process> loadBuff;
        for(Process p: inactive) {
            if(p.arrTimes[p.burst] <= clock) {
                loadBuff.push_back(p);
            }

        }
        for (std::pair<int, int> gap : gaps) {
            if (gap.second > maxGapSize) {
                maxGapSize = gap.second;
                maxGap = gap.first;
            }
            if (maxGapSize >= frames) break;
        }
        clock++;
        break;
    }
}

void nonContiguous(std::vector<Process> process) {
  int ms = 0;
  int freeSpaces = 256;
  std::vector<char> memory;
  for (int i = 0; i < 256; ++i) {
    memory.push_back('.');
  }
  std::cout << "time " << ms << "ms: Simulator started (Non-contiguous)"
            << std::endl;
}

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