#include "pin.H"
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

/* Golbal Variables */
std::ostream *out{&std::cerr};
std::ofstream fileptr;

struct trace_bbl {
  UINT64 traceID;
  ADDRINT inst_address;
  std::string assembly_string;
};

KNOB<std::string> output_file_name(KNOB_MODE_WRITEONCE, "pintool", "o",
                                   "run.log", "The name of the output file");
UINT64 traceCount{0};
std::vector<trace_bbl> trace_buffer;

/* Analysis Functions */
// VOID DumpTraceData() {
//   trace_buffer.push_back({traceCount, });
// }

void Push_Trace_Inst(ADDRINT inst_addr, std::string *disaasem) {
  trace_buffer.push_back({traceCount, inst_addr, *disaasem});
  if (trace_buffer.size() >= 1000) {
    for (const auto &item : trace_buffer) {
      fileptr << "trace_id: " << item.traceID
              << " inst_address: " << (void *)item.inst_address
              << " assembly: " << item.assembly_string << std::endl;
    }
    trace_buffer.clear();
  }
}

/* Instrumentation Callbacks */
VOID Trace_Instrument(TRACE trace, void *v) {
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {
      std::string *disassem_string = new std::string(INS_Disassemble(ins));
      INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)Push_Trace_Inst,
                     IARG_INST_PTR, IARG_PTR, disassem_string, IARG_END);
    }
  }
  traceCount++;
}

/* Helpers */
int Usage() {
  *out << "Trying to figure out what a trace is " << std::endl;
  return 1;
}

/* Main */
int main(INT32 argc, char *argv[]) {
  if (PIN_Init(argc, argv)) {
    return Usage();
  }
  std::string filename{output_file_name.Value()};
  fileptr.open(filename, std::ios::out);
  if (filename.empty()) {
    (*out) << "impropoer file " << std::endl;
    return 1;
  }

  if (!fileptr) {
    (*out) << " File error " << std::endl;
    return 1;
  }

  TRACE_AddInstrumentFunction(Trace_Instrument, 0);

  PIN_StartProgram();

  return 0;
}