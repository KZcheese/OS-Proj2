//Danny Li - liy37
//Kevin Zhan - zhank2

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
    for (int i = 0; i < frames; ++i) {
        if (i % fPerLine == 0 && i != 0) std::cout << std::endl;
        std::cout << memory[i];
    }
    std::cout << std::endl;
    std::cout << "================================" << std::endl;
}

void printPageTable(std::vector<char> memory, std::vector<Process> process) {
    unsigned int counter = 0;
    std::cout << "PAGE TABLE [page,frame]:\n";
    for (unsigned int i = 0; i < process.size(); ++i) {
        if (!process[i].positions.empty()) {
            std::cout << process[i].name << ": ";
            for (unsigned int j = 0; j < process[i].positions.size(); ++j) {
                if (counter == process[i].positions.size() - 1) {
                    std::cout << "[" << counter << "," << process[i].positions[j] << "]" << std::endl;
                    break;
                } else if (counter % 10 == 9 && counter != 0)
                    std::cout << "[" << counter << "," << process[i].positions[j] << "]" << std::endl;
                else std::cout << "[" << counter << "," << process[i].positions[j] << "] ";
                counter++;
            }
            counter = 0;
        }
    }
}

bool compByLoc(Process i, Process j) {
    return i.location < j.location;
}

void findLocation(const std::vector<char> &memory, Process &p, int alg, int nextAvailSpot) {
    //next = 0 
    if (alg == 0) {
        for (unsigned int i = nextAvailSpot; i < memory.size(); ++i) {
            if (memory[i] == '.') {
                bool canPlace = true;
                for (int j = 0; j < p.frames; ++j) {
                    if (memory[i + j] != '.') {
                        canPlace = false;
                        break;
                    }
                }
                if (canPlace) {
                    p.location = i;
                    return;
                }
            }
        }

        for (int i = 0; i < nextAvailSpot; ++i) {
            if (memory[i] == '.') {
                bool canPlace = true;
                for (int j = 0; j < p.frames; ++j) {
                    if (memory[i + j] != '.') {
                        canPlace = false;
                        break;
                    }
                }
                if (canPlace) {
                    p.location = i;
                    return;
                }
            }
        }
    }
}

void writeMem(std::vector<char> &memory, const Process &p, const bool &add) {
    char writeChar;
    if (add) writeChar = p.name;
    else writeChar = '.';
    for (int i = 0; i < p.frames; i++) memory[i + p.location] = writeChar;
}

bool compByID(Process i, Process j) {
    return i.name < j.name;
}

bool compGap(std::pair<int, int> i, std::pair<int, int> j) {
    return i.second < j.second;
}

void defrag(std::vector<char> &memory, std::vector<Process> &active, int &offset, int &clock) {
    std::cout << "starting defragmentation" << std::endl;
    std::sort(active.begin(), active.end(), compByLoc);
    std::vector<char> moved;
    int movedFrames = 0, end = 0;
    memory = std::vector<char>(frames, '.');

    for (unsigned int i = 0; i < active.size(); i++) {
        if (active[i].location != end) {
            movedFrames += active[i].frames;
            moved.push_back(active[i].name);
            active[i].location = end;
        }
        end = active[i].location + active[i].frames;
        writeMem(memory, active[i], true);
    }
    offset += movedFrames;
    clock += movedFrames;

    std::cout << "time " << clock << "ms: " << "Defragmentation complete (moved " << movedFrames << " frames: ";
    for (unsigned int i = 0; i < moved.size() - 1; i++) std::cout << moved[i] << ", ";
    std::cout << moved[moved.size() - 1] << ")" << std::endl;

}

std::pair<int, int> generateGaps(std::vector<char> &memory, std::vector<std::pair<int, int> > &gaps) {
    int maxGapSize = 0;
    int maxGapLoc = -1;
    for (int i = 0; i < frames; i++) {
        if (memory[i] == '.') {
            int j = i + 1;
            for (; j < frames; j++) if (memory[j] != '.') break;
            gaps.push_back(std::pair<int, int>(i, j - i));
            if (j - i > maxGapSize) {
                maxGapSize = j - i;
                maxGapLoc = (int) gaps.size() - 1;
            }
            i = j;
        }
    }
    return std::pair<int, int>(maxGapLoc, maxGapSize);
}

void defragment(std::vector<char> &memory, std::vector<Process> &newProcess, int &ms, int &nextAvailSpot) {
    nextAvailSpot = 0;
    int movedFrames = 0;

    // std::set<char> movedProcesses;
    std::vector<char> movedProcesses;

    for (unsigned int i = 0; i < memory.size() - 1; ++i) {
        if (memory[i] == '.') {
            for (unsigned int j = i; j < memory.size(); ++j) {
                if (memory[j] != '.') {
                    bool notThere = true;
                    for (unsigned int k = 0; k < movedProcesses.size(); ++k) {
                        if (movedProcesses[k] == memory[j]) {
                            notThere = false;
                            break;
                        }
                    }
                    if (notThere) {
                        movedProcesses.push_back(memory[j]);
                    }
                    movedFrames++;

                    memory[i++] = memory[j];
                    memory[j] = '.';
                }
            }
        }
    }
    ms += (t_memmove * movedFrames);

    for (unsigned int i = 0; i < newProcess.size(); i++)
        for (unsigned int j = 0; j < newProcess[i].arrTimes.size(); ++j)
            newProcess[i].arrTimes[j] += movedFrames * t_memmove;


    std::cout << "time " << ms << "ms: " << "Defragmentation complete (moved " << movedFrames << " frames: ";
    for (std::vector<char>::iterator i = movedProcesses.begin(); i != movedProcesses.end(); i++) {
        if (i == movedProcesses.begin()) std::cout << *i;
        else std::cout << ", " << *i;
    }
    std::cout << ")" << std::endl;

}

void nextFit() {
    std::vector<char> memory(frames, '.');
    std::vector<Process> newProcess(processes);
    int ms = 0;
    int nextAvailSpot = 0;
    std::cout << "time " << ms << "ms: Simulator started (Contiguous -- Next-Fit)" << std::endl;
    while (!newProcess.empty()) {
        for (unsigned int i = 0; i < newProcess.size(); ++i) {
            if (newProcess[i].runTimes[0] + newProcess[i].arrTimes[0] == ms) {
                std::cout << "time " << ms << "ms: Process " << newProcess[i].name << " removed:" << std::endl;
                for (int j = 0; j < frames; ++j) if (memory[j] == newProcess[i].name) memory[j] = '.';


                printMemory(memory);
                newProcess[i].runTimes.erase(newProcess[i].runTimes.begin());
                newProcess[i].arrTimes.erase(newProcess[i].arrTimes.begin());
                std::sort(newProcess.begin(), newProcess.end(), compByID);
                if (newProcess[i].arrTimes.size() == 0) newProcess.erase(newProcess.begin() + i--);
            }
        }

        for (unsigned int i = 0; i < newProcess.size(); ++i) {
            if (newProcess[i].arrTimes[0] == ms) {
                std::cout << "time " << ms << "ms: Process " << newProcess[i].name << " arrived (requires "
                          << newProcess[i].frames << " frames)" << std::endl;
                findLocation(memory, newProcess[i], 0, nextAvailSpot);
                if (newProcess[i].location != -1) {
                    std::cout << "time " << ms << "ms: Placed process " << newProcess[i].name << ":" << std::endl;
                    for (int j = newProcess[i].location; j < newProcess[i].location + newProcess[i].frames; ++j)
                        memory[j] = newProcess[i].name;

                    nextAvailSpot = newProcess[i].location + newProcess[i].frames;
                    printMemory(memory);
                } else {
                    int counter = 0;
                    for (unsigned int j = 0; j < memory.size(); ++j) if (memory[j] == '.') counter++;

                    if (counter >= newProcess[i].frames) {
                        std::cout << "time " << ms << "ms: Cannot place process " << newProcess[i].name
                                  << " -- starting defragmentation" << std::endl;
                        defragment(memory, newProcess, ms, nextAvailSpot);
                        printMemory(memory);
                        findLocation(memory, newProcess[i], 0, nextAvailSpot);
                        std::cout << "time " << ms << "ms: Placed process " << newProcess[i].name << ":" << std::endl;
                        for (int j = newProcess[i].location; j < newProcess[i].location + newProcess[i].frames; ++j) {
                            memory[j] = newProcess[i].name;
                        }
                        nextAvailSpot = newProcess[i].location + newProcess[i].frames;
                        printMemory(memory);
                    } else {
                        std::cout << "time " << ms << "ms: Cannot place process " << newProcess[i].name
                                  << " -- skipped!" << std::endl;
                        newProcess[i].runTimes.erase(newProcess[i].runTimes.begin());
                        newProcess[i].arrTimes.erase(newProcess[i].arrTimes.begin());
                        std::sort(newProcess.begin(), newProcess.end(), compByID);
                        if (newProcess[i].arrTimes.size() == 0) newProcess.erase(newProcess.begin() + i--);
                    }
                }
            }
        }
        ms++;
    }
    std::cout << "time " << ms - 1 << "ms: Simulator ended (Contiguous -- Next-Fit)" << std::endl;
}


void bestFit() {
    int clock = 0, offset = 0;
    std::vector<char> memory(frames, '.');
    std::vector<Process> inactive(processes);
    std::vector<Process> active;

    int spaceUsed = 0;
    std::cout << "time " << clock << "ms: " << "Simulator started (Contiguous -- Best-Fit)" << std::endl;

    while (!active.empty() || !inactive.empty()) {
        int jumpTime = INT_MAX;
        std::vector<std::pair<int, int> > gaps;

        std::vector<Process> removeBuff;
        for (unsigned int i = 0; i < active.size(); i++) {
            Process p = active[i];
            if (p.arrTimes[p.burst] + p.runTimes[p.burst] + offset <= clock) {
                removeBuff.push_back(p);
                active.erase(active.begin() + i--);
            }
        }

        std::sort(removeBuff.begin(), removeBuff.end(), compByID);
        for (unsigned int i = 0; i < removeBuff.size(); i++) {
            Process p = removeBuff[i];
            writeMem(memory, p, false);
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

        generateGaps(memory, gaps);

        std::vector<Process> addBuff;
        for (unsigned int i = 0; i < inactive.size(); i++) {
            Process p = inactive[i];
            if (p.arrTimes[p.burst] + offset <= clock) {
                addBuff.push_back(p);
                inactive.erase(inactive.begin() + i--);
            } else if (p.arrTimes[p.burst] + offset < jumpTime) jumpTime = p.arrTimes[p.burst] + offset;
        }

        std::sort(addBuff.begin(), addBuff.end(), compByID);
        for (unsigned int i = 0; i < addBuff.size(); i++) {
            Process p = addBuff[i];
            std::cout << "time " << clock << "ms: Process " << p.name << " arrived (requires " << p.frames
                      << " frames)" << std::endl;

            int gapLoc = -1;
            int gapSize = 0;
            sort(gaps.begin(), gaps.end(), compGap);
            for (unsigned int j = 0; j < gaps.size(); j++)
                if (gaps[j].second >= p.frames) {
                    gapLoc = j;
                    gapSize = gaps[gapLoc].second;
                    break;
                }

            if (gapLoc == -1) {
                std::cout << "time " << clock << "ms: Cannot place process " << p.name << " -- ";
                if (frames - spaceUsed >= p.frames) {
                    defrag(memory, active, offset, clock);
                    gaps.clear();
                    std::pair<int, int> gap = generateGaps(memory, gaps);
                    gapLoc = gap.first;
                    gapSize = gap.second;
                    printMemory(memory);
                } else {
                    std::cout << "skipped!" << std::endl;
                    p.burst++;
                    if (p.burst < p.arrTimes.size()) inactive.push_back(p);
                    continue;
                }
            }

            p.location = gaps[gapLoc].first;
            spaceUsed += p.frames;
            writeMem(memory, p, true);
            active.push_back(p);
            std::cout << "time " << clock << "ms: Placed process " << p.name << ":" << std::endl;
            printMemory(memory);

            if (gapSize > p.frames) {
                gaps[gapLoc].first = p.location + p.frames;
                gaps[gapLoc].second -= p.frames;
            } else gaps.erase(gaps.begin() + gapLoc);
        }

        for (unsigned int i = 0; i < active.size(); i++) {
            Process p = active[i];
            if (p.runTimes[p.burst] + offset + p.arrTimes[p.burst] < jumpTime)
                jumpTime = p.runTimes[p.burst] + offset + p.arrTimes[p.burst];
        }

        if (!active.empty() || !inactive.empty()) clock = jumpTime;
    }

    std::cout << "time " << clock << "ms: " << "Simulator ended (Contiguous -- Best-Fit)" << std::endl;
}


void worstFit() {
    int clock = 0, offset = 0;
    std::vector<char> memory(frames, '.');
    std::vector<Process> inactive(processes);
    std::vector<Process> active;

    int spaceUsed = 0;
    std::cout << "time " << clock << "ms: " << "Simulator started (Contiguous -- Worst-Fit)" << std::endl;

    while (!active.empty() || !inactive.empty()) {
        int jumpTime = INT_MAX;
        std::vector<std::pair<int, int> > gaps;

        std::vector<Process> removeBuff;
        for (unsigned int i = 0; i < active.size(); i++) {
            Process p = active[i];
            if (p.arrTimes[p.burst] + p.runTimes[p.burst] + offset <= clock) {
                removeBuff.push_back(p);
                active.erase(active.begin() + i--);
            }
        }

        std::sort(removeBuff.begin(), removeBuff.end(), compByID);
        for (unsigned int i = 0; i < removeBuff.size(); i++) {
            Process p = removeBuff[i];
            writeMem(memory, p, false);
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

        std::pair<int, int> maxGap = generateGaps(memory, gaps);
        int maxGapLoc = maxGap.first;
        int maxGapSize = maxGap.second;

        std::vector<Process> addBuff;
        for (unsigned int i = 0; i < inactive.size(); i++) {
            Process p = inactive[i];
            if (p.arrTimes[p.burst] + offset <= clock) {
                addBuff.push_back(p);
                inactive.erase(inactive.begin() + i--);
            } else if (p.arrTimes[p.burst] + offset < jumpTime) jumpTime = p.arrTimes[p.burst] + offset;
        }

        std::sort(addBuff.begin(), addBuff.end(), compByID);
        for (unsigned int i = 0; i < addBuff.size(); i++) {
            Process p = addBuff[i];
            std::cout << "time " << clock << "ms: Process " << p.name << " arrived (requires " << p.frames
                      << " frames)" << std::endl;
            if (maxGapSize < p.frames) {
                std::cout << "time " << clock << "ms: Cannot place process " << p.name << " -- ";
                if (frames - spaceUsed >= p.frames) {
                    defrag(memory, active, offset, clock);
                    gaps.clear();
                    std::pair<int, int> maxGap = generateGaps(memory, gaps);
                    maxGapLoc = maxGap.first;
                    maxGapSize = maxGap.second;
                    printMemory(memory);
                } else {
                    std::cout << "skipped!" << std::endl;
                    p.burst++;
                    if (p.burst < p.arrTimes.size()) inactive.push_back(p);
                    continue;
                }
            }

            p.location = gaps[maxGapLoc].first;
            spaceUsed += p.frames;
            writeMem(memory, p, true);
            active.push_back(p);
            std::cout << "time " << clock << "ms: Placed process " << p.name << ":" << std::endl;
            printMemory(memory);

            if (maxGapSize > p.frames) {
                gaps[maxGapLoc].first = p.location + p.frames;
                gaps[maxGapLoc].second -= p.frames;
            } else gaps.erase(gaps.begin() + maxGapLoc);

            maxGapSize = 0;
            maxGapLoc = -1;
            for (unsigned int j = 0; j < gaps.size(); j++)
                if (gaps[j].second > maxGapSize) {
                    maxGapLoc = j;
                    maxGapSize = gaps[j].second;
                }
        }

        for (unsigned int i = 0; i < active.size(); i++) {
            Process p = active[i];
            if (p.runTimes[p.burst] + offset + p.arrTimes[p.burst] < jumpTime)
                jumpTime = p.runTimes[p.burst] + offset + p.arrTimes[p.burst];
        }

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
                for (int j = 0; j < frames; ++j) if (memory[j] == newProcess[i].name) memory[j] = '.';

                printMemory(memory);

                newProcess[i].runTimes.erase(newProcess[i].runTimes.begin());
                newProcess[i].arrTimes.erase(newProcess[i].arrTimes.begin());
                newProcess[i].positions.clear();
                printPageTable(memory, newProcess);

                if (newProcess[i].arrTimes.size() == 0) newProcess.erase(newProcess.begin() + i--);
            }
        }

        for (unsigned int i = 0; i < newProcess.size(); ++i) {
            if (newProcess[i].arrTimes[0] == ms) {
                std::cout << "time " << ms << "ms: Process " << newProcess[i].name << " arrived (requires "
                          << newProcess[i].frames << " frames)" << std::endl;

                int counter = 0;
                for (unsigned int j = 0; j < memory.size(); ++j)
                    if (memory[j] == '.') counter++;

                if (counter >= newProcess[i].frames) {
                    std::cout << "time " << ms << "ms: Placed process " << newProcess[i].name << ":" << std::endl;
                    int remainingFrames = newProcess[i].frames;

                    for (unsigned int j = 0; j < memory.size(); ++j) {
                        if (memory[j] == '.') {
                            newProcess[i].positions.push_back(j);
                            memory[j] = newProcess[i].name;
                            remainingFrames--;
                            if (remainingFrames == 0) break;
                        }
                    }
                    printMemory(memory);
                    printPageTable(memory, newProcess);
                } else {
                    std::cout << "time " << ms << "ms: Cannot place process " << newProcess[i].name
                              << " -- skipped!"
                              << std::endl;
                    newProcess[i].runTimes.erase(newProcess[i].runTimes.begin());
                    newProcess[i].arrTimes.erase(newProcess[i].arrTimes.begin());
                    if (newProcess[i].arrTimes.size() == 0) newProcess.erase(newProcess.begin() + i--);
                }
            }
        }
        ms++;
    }
    std::cout << "time " << ms - 1 << "ms: Simulator ended (Non-contiguous)";
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

    nextFit();
    std::cout << std::endl;

    bestFit();
    std::cout << std::endl;

    worstFit();
    std::cout << std::endl;

    nonContiguous();
    std::cout << std::endl;

    return 0;
}