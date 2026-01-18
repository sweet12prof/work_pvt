#include "fstream"
#include "iostream"
#include "pin.H"

// #define 
/* Globals */
UINT32 threadCount{0};
std::ofstream file;
KNOB<std::string> output_file(KNOB_MODE_WRITEONCE, "pintool", "o", "ThreadCountTool.log",
                              "File to log outputs");
std::ostream *out = &std::cerr;
/* Analysis Routines */
/* Instrumentation Callbacks */
VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v) {
  threadCount++;
  file << "Thread " << threadid << ": started " << std::endl;
}

VOID ThreadFinish(THREADID threadid, const CONTEXT *, INT32 flags, VOID *v) {
  file << "Thread " << threadid << ": finished " << std::endl;
}

VOID ToolFinish(INT32 code, VOID *v) {
  file << " Total number of threads: " << threadCount << std::endl;
}

/* Helpers */
UINT32 Usage() {
  (*out) << "This toool is checking how many threads are in the pinpall"
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

  file.open(file_name, std::ios::out);

  PIN_AddThreadStartFunction(ThreadStart, 0);
  PIN_AddThreadFiniFunction(ThreadFinish, 0);
  PIN_StartProgram();
  PIN_AddFiniFunction(ToolFinish, 0);
}