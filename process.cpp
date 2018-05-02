#include "process.h"
#include <iostream>

Process::Process() {}

Process::Process(char _name, int _frames, std::vector<int> _arrTimes, std::vector<int> _runTimes) {
    name = _name;
    frames = _frames;
    arrTimes = _arrTimes;
    runTimes = _runTimes;
    location = -1;
    burst = 0;
}
