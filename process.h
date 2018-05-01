#ifndef Process_h
#define Process_h
#include <iostream>
#include <vector>

class Process {
 public:
  Process();
  Process(char _name, int _frames, std::vector<int> _arrTimes, std::vector<int> _runTimes);
  char name;
  int frames;
  std::vector<int> arrivalTimes;
  std::vector<int> runTimes;
};
#endif
