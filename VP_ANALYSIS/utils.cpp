#include "utils.hpp"

std::pair<bool, std::vector<TraceData>> readFile(std::istream *file){
  std::vector<TraceData> buffer;
  buffer.resize(CHUNK_SIZE);
    file->read(reinterpret_cast<char *>(buffer.data()),
               CHUNK_SIZE * sizeof(TraceData));
    auto itemsRead = file->gcount() / sizeof(TraceData);
    if (itemsRead == 0)
      return {true, {}};
    buffer.resize(itemsRead);
    return {false, buffer};
}