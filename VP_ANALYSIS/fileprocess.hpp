
#include "globals.hpp"
#include "utils.hpp"
#include <fstream>

#ifndef FILE_PROCESS_H
#define FILE_PROCESS_H

class FileProcess {
public:
  FileProcess(std::istream *file) : file(file) {}

  std::pair<bool, std::vector<TraceData>> progress_read() const {
    return readFile(file);
  }

private:
  std::istream *file;
};

#endif