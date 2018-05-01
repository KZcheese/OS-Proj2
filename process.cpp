#include "process.h"
#include <iostream>

Process::Process() {}

Process::Process(char _name, std::vector<int> _arrTimes, std::vector<int> _runTimes) {
  name = _name;
  arrivalTimes = _arrTimes;
  runTimes = _runTimes;
}
