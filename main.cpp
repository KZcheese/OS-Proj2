#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "process.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>


const int frames = 256;
const int fPerLine = 32;

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

void printPageTable(std::vector<char> memory, std::vector<Process> process){
  	unsigned int counter = 0;
	std::cout << "PAGE TABLE [page,frame]:\n";
        for (unsigned int i = 0; i < process.size(); i++) {
          if (!process[i].positions.empty()) {
            std::cout << process[i].name << ": ";
            for (unsigned int j = 0; j < process[i].positions.size(); j++) {
              if (counter == process[i].positions.size() - 1) {
                std::cout << "[" << counter << "," << process[i].positions[j]
                          << "]" << std::endl;
                break;
              } else if (counter % 10 == 9 && counter != 0) {
                std::cout << "[" << counter << "," << process[i].positions[j]
                          << "]" << std::endl;
              } else {
                std::cout << "[" << counter << "," << process[i].positions[j]
                          << "] ";
              }
              counter++;
            }
            counter = 0;
        }
	}
}
void writeMem(std::vector<char> &memory, const Process &p, const bool &add) {
    char writeChar;
    if (add) writeChar = p.name;
    else writeChar = '.';
    for (int i = 0; i < p.frames; i++)
        memory[i + p.location] = writeChar;

}

void nextFit() {}

void bestFit() {}

void worstFit() {
    int clock = 0;
    std::vector<char> memory(frames, '.');
    std::vector<Process> inactive(processes);
    std::vector<Process> active;
    std::vector<std::pair<int, int> > gaps;
    gaps.push_back(std::pair<int, int>(0, frames));


    int maxGapSize = 0;
    int maxGap = -1;
    while (true) {
        for (int i = 0; i < active.size(); i++) {
            Process p = active[i];
            if (p.arrTimes[p.burst] + p.runTimes[p.burst] <= clock) {
                p.burst++;
                p.location = -1;
                active.erase(active.begin() + i--);
                if (p.burst < p.arrTimes.size() - 1)
                    inactive.push_back(p);
            }
        }

        std::vector<Process> loadBuff;
        for (Process p: inactive) {
            if (p.arrTimes[p.burst] <= clock) {
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

void nonContiguous() {
	int ms = 0;
	int freeSpaces = frames;
	std::vector<char> memory(frames, '.');
	std::cout << "time " << ms << "ms: Simulator started (Non-contiguous)"
				<< std::endl;
	std::vector<Process> newProcess(processes);
	while (!newProcess.empty()) {
		for (unsigned int i = 0; i < newProcess.size(); i++) {
			if (newProcess[i].runTimes[0] == ms - newProcess[i].arrTimes[0]) {
				std::cout << "time " << ms << "ms: Process " << newProcess[i].name
						<< " removed:" << std::endl;
				for (int j = 0; j < 256; j++) {
					if (memory[j] == newProcess[i].name) {
						memory[j] = '.';
					}
				}
				newProcess[i].runTimes.erase(newProcess[i].runTimes.begin());
				newProcess[i].arrTimes.erase(newProcess[i].arrTimes.begin());
				freeSpaces += newProcess[i].frames;
				newProcess[i].positions.clear();
				printMemory(memory);
				if (newProcess[i].runTimes.empty() ||
					newProcess[i].arrTimes.empty()) {
				newProcess.erase(newProcess.begin() + i);
				}
				printPageTable(memory, newProcess);
			}
		}
		for (unsigned int i = 0; i < newProcess.size(); i++) {
			if (ms == newProcess[i].arrTimes[0]) {
			std::cout << "time " << ms << "ms: Process "
						<< newProcess[i].name << " arrived (requires "
						<< newProcess[i].frames << " frames)"
						<< std::endl;

			if (freeSpaces < newProcess[i].frames) {
				std::cout << "time " << ms << "ms: Cannot place process "
						<< newProcess[i].name << " -- skipped!"
						<< std::endl;
				newProcess[i].runTimes.erase(
					newProcess[i].runTimes.begin());
				newProcess[i].arrTimes.erase(
					newProcess[i].arrTimes.begin());
				if (newProcess[i].runTimes.empty()) {
				newProcess.erase(newProcess.begin() + i);
				i--;
				}
				continue;
			}

			if (freeSpaces >= newProcess[i].frames) {
				int counter = 0;
				for (int x = 0; x < frames; ++x) {
				if (memory[x] == '.' &&
					counter < newProcess[i].frames) {
					newProcess[i].positions.push_back(x);
					memory[x] = newProcess[i].name;
					counter++;
				}
				}
				std::cout << "time " << ms << "ms: Placed process "
						<< newProcess[i].name << ":" << std::endl;
				freeSpaces -= newProcess[i].frames;
			}
			printMemory(memory);
			printPageTable(memory, newProcess);
			continue;
			}
		}
		ms++;
	}
	std::cout << "time " << ms-1 << "ms: Simulator ended (Non-contiguous)" << std::endl;
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
    nonContiguous();
    return 1;

}