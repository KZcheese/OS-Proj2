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
        if (i % fPerLine == 0 && i != 0) {
            std::cout << std::endl;
        }
        std::cout << memory[i];
    }
    std::cout << std::endl;
    std::cout << "================================" << std::endl;
}

void printPageTable(std::vector<char> memory, std::vector<Process> process) {
    unsigned int counter = 0;
    std::cout << "PAGE TABLE [page,frame]:\n";
    for (unsigned int i = 0; i < process.size(); i++) {
        if (!process[i].positions.empty()) {
            std::cout << process[i].name << ": ";
            for (unsigned int j = 0; j < process[i].positions.size(); j++) {
                if (counter == process[i].positions.size() - 1) {
                    std::cout << "[" << counter << "," << process[i].positions[j] << "]" << std::endl;
                    break;
                } else if (counter % 10 == 9 && counter != 0) {
                    std::cout << "[" << counter << "," << process[i].positions[j] << "]" << std::endl;
                } else {
                    std::cout << "[" << counter << "," << process[i].positions[j] << "] ";
                }
                counter++;
            }
            counter = 0;
        }
    }
}

void nextFit() {}

void bestFit() {}

void writeMem(std::vector<char> &memory,
              const Process &p,
              const bool &add) {
    char writeChar;
    if (add) writeChar = p.name;
    else writeChar = '.';
    for (int i = 0; i < p.frames; i++) memory[i + p.location] = writeChar;
}

bool compByLoc(Process i, Process j) {
    return i.location < j.location;
}

std::pair<int, int> defrag(std::vector<char> &memory, std::vector<Process> &active, int &offset, int &clock) {
    std::cout << "starting defragmentation" << std::endl;
    std::sort(active.begin(), active.end(), compByLoc);
    std::vector<char> moved;
    int movedFrames = 0;
    int end = 0;
    memory = std::vector<char>(frames, '.');
    for (unsigned int i = 0; i < active.size(); i++) {
        if (active[i].location != end) {
            movedFrames += active[i].frames;
            moved.push_back(active[i].name);
            active[i].location = end;
            end = active[i].location + active[i].frames;
            offset += t_memmove;
        }
        writeMem(memory, active[i], true);
    }

    clock += offset;

    std::cout << "time " << clock << "ms: " << "Defragmentation complete (moved " << movedFrames << " frames: ";
    for (unsigned int i = 0; i < moved.size() - 1; i++) std::cout << moved[i] << ", ";
    std::cout << moved[moved.size() - 1] << ")" << std::endl;

    return std::pair<int, int>(end, frames - end);
}

void worstFit() {
    int clock = 0, offset = 0;
    std::vector<char> memory(frames, '.');
    std::vector<Process> inactive(processes);
    std::vector<Process> active;
    std::vector<std::pair<int, int> > gaps;

    int spaceUsed = 0;
    std::cout << "time " << clock << "ms: " << "Simulator started (Contiguous -- Worst-Fit)" << std::endl;

    while (!active.empty() || !inactive.empty()) {
        int jumpTime = INT_MAX;

        for (unsigned int i = 0; i < active.size(); i++) {
            Process p = active[i];

            if (p.arrTimes[p.burst] + p.runTimes[p.burst] + offset <= clock) {
                writeMem(memory, p, false);
                active.erase(active.begin() + i--);
                std::cout << "time " << clock << "ms: " << "Process " << p.name << " removed:" << std::endl;
                printMemory(memory);

                p.burst++;
                p.location = -1;
                spaceUsed -= p.frames;

                if (p.burst < p.arrTimes.size()) {
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

                for (; j < frames; j++)
                    if (memory[j] != '.') break;

                gaps.push_back(std::pair<int, int>(i, j - i));

                if (j - i > maxGapSize) {
                    maxGapSize = j - i;
                    maxGapLoc = (int) gaps.size() - 1;
                }

                i = j;
            }
        }

//        std::vector<Process> delayed;
        for (unsigned int i = 0; i < inactive.size(); i++) {
            Process p = inactive[i];
            if (p.arrTimes[p.burst] + offset <= clock) {
                std::cout << "time " << clock << "ms: Process " << p.name << " arrived (requires " << p.frames
                          << " frames)" << std::endl;
                if (maxGapSize < p.frames) {
                    std::cout << "time " << clock << "ms: Cannot place process " << p.name << " -- ";
                    if (frames - spaceUsed >= p.frames) {
                        std::pair<int, int> gap = defrag(memory, active, offset, clock);
                        gaps.push_back(gap);
                        maxGapLoc = (int) gaps.size() - 1;
                        maxGapSize = gap.second;
                        printMemory(memory);
                    } else {
                        std::cout << "skipped!" << std::endl;
                        inactive[i].burst++;
                        if(p.burst >= p.arrTimes.size()) inactive.erase(inactive.begin() + i--);
//                        delayed.push_back(p);
                        continue;
                    }
                }

                p.location = gaps[maxGapLoc].first;

                spaceUsed += p.frames;
                writeMem(memory, p, true);
                inactive.erase(inactive.begin() + i--);
                active.push_back(p);
                std::cout << "time " << clock << "ms: Placed process " << p.name << ":" << std::endl;
                printMemory(memory);
                if (maxGapSize > p.frames) {
                    gaps[maxGapLoc].first = p.location + p.frames;
                    gaps[maxGapLoc].second -= p.frames;
                } else gaps.erase(gaps.begin() + maxGapLoc);

                maxGapSize = 0;
                maxGapLoc = -1;
                for (unsigned int j = 0; j < gaps.size(); j++) {
                    if (gaps[j].second > maxGapSize) {
                        maxGapLoc = j;
                        maxGapSize = gaps[j].second;
                    }
                }
            } else if (p.arrTimes[p.burst] + offset < jumpTime) jumpTime = p.arrTimes[p.burst] + offset;
        }

        for (unsigned int i = 0; i < active.size(); i++) {
            Process p = active[i];
            if (p.runTimes[p.burst] + offset + p.arrTimes[p.burst] < jumpTime)
                jumpTime = p.runTimes[p.burst] + offset +
                           p.arrTimes[p.burst];
        }
//        for (unsigned int i = 0; i < delayed.size(); i++)
//            if (delayed[i].arrTimes[delayed[i].burst + 1] <= jumpTime)
//                delayed[i].burst++;

        if (!active.empty() || !inactive.empty()) clock = jumpTime;
    }

    std::cout << "time " << clock << "ms: " << "Simulator ended (Contiguous -- Worst-Fit)" << std::endl;
}

void nonContiguous() {
    std::vector<char> memory(frames, '.');
    std::vector<Process> newProcess(processes);
    int ms = 0;
    std::cout << "time " << ms << "ms: Simulator started (Non-contiguous)" << std::endl;
    while (!newProcess.empty()) {
        for (unsigned int i = 0; i < newProcess.size(); i++) {
            if (newProcess[i].runTimes[0] == ms - newProcess[i].arrTimes[0]) {
                std::cout << "time " << ms << "ms: Process " << newProcess[i].name << " removed:" << std::endl;
                for (int j = 0; j < 256; j++) {
                    if (memory[j] == newProcess[i].name) {
                        memory[j] = '.';
                    }
                }

                printMemory(memory);

                newProcess[i].runTimes.erase(newProcess[i].runTimes.begin());
                newProcess[i].arrTimes.erase(newProcess[i].arrTimes.begin());
                newProcess[i].positions.clear();
                printPageTable(memory, newProcess);

                if (newProcess[i].arrTimes.size() == 0) {
                    newProcess.erase(newProcess.begin() + i);
                    i--;
                }
            }
        }

        for (unsigned int i = 0; i < newProcess.size(); i++) {
            if (newProcess[i].arrTimes[0] == ms) {
                std::cout << "time " << ms << "ms: Process " << newProcess[i].name << " arrived (requires "
                          << newProcess[i].frames << " frames)" << std::endl;

                int counter = 0;
                for (unsigned int j = 0; j < memory.size(); j++) {
                    if (memory[j] == '.')
                        counter++;
                }

                if (counter >= newProcess[i].frames) {
                    std::cout << "time " << ms << "ms: Placed process " << newProcess[i].name << ":" << std::endl;
                    int remainingFrames = newProcess[i].frames;

                    for (unsigned int j = 0; j < memory.size(); j++) {
                        if (memory[j] == '.') {
                            newProcess[i].positions.push_back(j);
                            memory[j] = newProcess[i].name;
                            remainingFrames--;
                            if (remainingFrames == 0)
                                break;
                        }
                    }
                    printMemory(memory);
                    printPageTable(memory, newProcess);
                } else {
                    std::cout << "time " << ms << "ms: Cannot place process " << newProcess[i].name << " -- skipped!"
                              << std::endl;
                    newProcess[i].runTimes.erase(newProcess[i].runTimes.begin());
                    newProcess[i].arrTimes.erase(newProcess[i].arrTimes.begin());
                    if (newProcess[i].arrTimes.size() == 0) {
                        newProcess.erase(newProcess.begin() + i);
                        i--;
                    }
                }
            }
        }
        ms++;
    }
    std::cout << "time " << ms - 1 << "ms: Simulator ended (Non-contiguous)" << std::endl;
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
//    nextFit();
//    bestFit();
    worstFit();
//    nonContiguous();
    return 0;
}