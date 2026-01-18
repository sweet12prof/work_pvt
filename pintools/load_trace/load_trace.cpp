#include "pin.H"
#include "gzstream.h"
#include <fstream>
#include <iostream>
#include <vector>
/* Globals */
enum class InstType : UINT8 { MEM_LOAD, MEM_STORE };
struct CallFrame {
  ADDRINT func_id;
  UINT8 has_meta;
  ADDRINT call_site;
};

struct TraceData {
  ADDRINT pc;
  InstType inst_type;
  CallFrame frame;
  ADDRINT memAddress;
};

KNOB<std::string> output_file(KNOB_MODE_WRITEONCE, "pintool", "o",
                              "mem_cfg_trace.log", "File to log outputs");
std::ostream *out = &std::cerr;
UINT32 threadCount{0};
ogzstream file;

std::vector<TraceData> trace;
std::vector<CallFrame> callstack;

/* Analysis Routines */
void RecordInstInfo(UINT32 inst_type, ADDRINT pc, ADDRINT memaddress) {
  // ADDRINT func_id = callstack.empty() ? 0 : callstack.back().;
  trace.push_back({pc, (InstType)inst_type, callstack.back(), memaddress});
  if (trace.size() >= 1000) {
    // for (auto item : trace)
    //   file << (void *)item.pc << " " << (void *)item.memAddress << " "
    //        << (void *)item.frame.func_id << " " << (void *)item.frame.call_site << " "
    //        << (UINT16)item.inst_type << " " << (UINT16)item.frame.has_meta << std::endl;
    file.write(reinterpret_cast<const char *>(trace.data()), trace.size()*sizeof(TraceData));
    trace.clear();
  }
}

void OnCall(ADDRINT target, ADDRINT callsite) {
  PIN_LockClient();
  RTN rtn = RTN_FindByAddress(target);
  if (RTN_Valid(rtn)) {
    ADDRINT rtn_id = RTN_Address(rtn);
    callstack.push_back({rtn_id, 1, callsite});
  } else {
    (*out) << " Pin lacks metadata for " << (void *)target << std::endl;
    callstack.push_back({target, 0, callsite});
  }
  PIN_UnlockClient();
}

void OnRet() {
  if (!callstack.empty())
    callstack.pop_back();
}

/* Instrumentation Callbacks */
VOID Trace(TRACE trace, VOID *v) {
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {
      if (INS_IsCall(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(OnCall),
                       IARG_BRANCH_TARGET_ADDR, IARG_INST_PTR, IARG_END);
      }

      if (INS_IsRet(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(OnRet), IARG_END);
      }

      UINT32 memoryOperands = INS_MemoryOperandCount(ins);
      for (UINT32 memOp = 0; memOp < memoryOperands; memOp++) {
        InstType inst_type;
        if (INS_MemoryOperandIsRead(ins, memOp)) {
          inst_type = InstType::MEM_LOAD;
          INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordInstInfo,
                                   IARG_UINT32, (UINT32)inst_type,
                                   IARG_INST_PTR, IARG_MEMORYOP_EA, memOp,
                                   IARG_END);
        }

        if (INS_MemoryOperandIsWritten(ins, memOp)) {
          inst_type = InstType::MEM_STORE;
          INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordInstInfo,
                                   IARG_UINT32, (UINT32)inst_type,
                                   IARG_INST_PTR, IARG_MEMORYOP_EA, memOp,
                                   IARG_END);
        }
      }
    }
  }
}

VOID ToolFinish(INT32 code, VOID *v) {
  if (!trace.empty()) {
    // for (auto item : trace)
      // file << (void *)item.pc << " " << (void *)item.memAddress << " "
      //      << (void *)item.frame.func_id << " " << item.frame.call_site << " "
      //      << (UINT16)item.inst_type << " " << (UINT16)item.frame.has_meta
      //      << std::endl;
      file.write(reinterpret_cast<const char *>(trace.data()), trace.size()*sizeof(TraceData));
  }
}
/* Helpers */
UINT32 Usage() {
  (*out) << "This toool is tracing memory requests and control flow in program "
            "order"
         << std::endl;
  return 1;
}
/* Main Function */
int main(int argc, char **argv) {
  PIN_Init(argc, argv);
  std::string file_name(output_file.Value());
  if (file_name.empty()) {
    (*out) << "Input a filename with -o flag " << std::endl;
    return 1;
  }

  file.open(file_name.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
  // INS_AddInstrumentFunction(Instruction, 0);
  TRACE_AddInstrumentFunction(Trace, 0);
  PIN_AddFiniFunction(ToolFinish, 0);
  PIN_StartProgram();
}