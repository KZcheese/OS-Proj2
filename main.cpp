#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "process.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <climits>
#include <algorithm>


const int frames = 256;
const int fPerLine = 32;
const int t_memmove = 1;

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

void writeMem(std::vector<char> &memory, const Process &p, const bool &add) {
    char writeChar;
    if (add) writeChar = p.name;
    else writeChar = '.';
    for (int i = 0; i < p.frames; i++)
        memory[i + p.location] = writeChar;
}

bool compByLoc(Process i, Process j){
    return i.location < j.location;
}

std::pair<int, int> defrag(std::vector<char> &memory, std::vector<Process> &active, int &offset) {
    std::sort(memory.begin(), memory.end(), compByLoc);
    int end = 0;
    for (int i = 0; i < active.size(); i++) {
        active[i].location = end;
        end = active[i].location + active[i].frames;
    }
    return std::pair<int, int>{end, frames - end};
}

void worstFit() {
    int clock = 0, offset = 0;
    std::vector<char> memory(frames, '.');
    std::vector<Process> inactive(processes);
    std::vector<Process> active;
    std::vector<std::pair<int, int> > gaps;

    int spaceUsed = 0;
    int jumpTime = INT_MAX;
    while (!active.empty() && !inactive.empty()) {

        for (int i = 0; i < active.size(); i++) {
            Process p = active[i];

            if (p.arrTimes[p.burst] + p.runTimes[p.burst] + offset <= clock) {
                writeMem(memory, p, false);
                active.erase(active.begin() + i--);

                p.burst++;
                p.location = -1;
                spaceUsed += p.frames;

                if (p.burst < p.arrTimes.size() - 1) {
                    if (jumpTime > p.arrTimes[p.burst] + offset) jumpTime = p.arrTimes[p.burst] + offset;
                    inactive.push_back(p);
                }
            }
        }
        int maxGapSize = 0;
        int maxGapLoc = -1;
        for (int i = 0; i < frames; i++) {
            if (memory[i] == '.') {
                int j = i + 1;

                for (; j < frames; j++) if (memory[j] != '.') break;

                gaps.push_back(std::pair<int, int>(i, j - i));

                if (j - i > maxGapSize) {
                    maxGapSize = j - i;
                    maxGapLoc = gaps.size() - 1;
                }

                i = j;
            }
        }

        for (int i = 0; i < inactive.size(); i++) {
            Process p = inactive[i];
            if (p.arrTimes[p.burst] + offset <= clock) {

                if (maxGapSize < p.frames) {
                    if (frames - spaceUsed >= p.frames) {
                        std::pair<int, int> gap = defrag(memory, active, offset);
                        maxGapLoc = gap.first;
                        maxGapSize = gap.second;
                    } else {
                        inactive[i].burst++;
                        continue;
                    }
                }

                p.location = gaps[maxGapLoc].first;

                spaceUsed -= p.frames;
                writeMem(memory, p, true);
                inactive.erase(inactive.begin() + i--);
                active.push_back(p);

                if (maxGapSize < p.frames) {
                    gaps[maxGapLoc].first = p.location + p.frames;
                    gaps[maxGapLoc].second -= p.frames;
                } else gaps.erase(gaps.begin() + maxGapLoc);

                if (p.runTimes[p.burst] + offset < jumpTime) jumpTime = p.runTimes[p.burst] + offset;
            } else if (p.arrTimes[p.burst] + offset < jumpTime) jumpTime = p.arrTimes[p.burst] + offset;
        }

        clock = jumpTime;
    }
}

void nonContiguous() {
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
            std::istringstream(frac.substr(0, frac.find(dash))) >> arrTime;
            std::istringstream(frac.substr(frac.find(dash) + 1, frac.length())) >> runTime;
            arrTimes.push_back(arrTime);
            runTimes.push_back(runTime);
        }
        processes.push_back(Process(id, size, arrTimes, runTimes));
    }
    in.close();
    return 0;

}