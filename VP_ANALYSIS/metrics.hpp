#include <types.h>
#include <array>
#include <map>
/*
    * We want to check distribution of 
        * strided  Load patterns at a particular pc
        * irregular Load patterns at a particualr pc, that do not repeat.....difficult to predict overall -> truly irregular patterns
        * irregular Load patterns at a particular pc that repeat -> irregular but repetitive patterns
        * intra-correlated irregular load aka Global Correlation -> At the individual pc, it is irregular but it relates well with nearby loads; most likely RDS loads
        * Control Correlated loads ie; 
            * A memory access pattern within a function, may be stride, truly irregular or irregular but repetitive
            * The memory access pattern within the function has a particular memory access pattern depending on where the function was called 
        * Array based RDS structure;  
            * These are array based recursive data structures 
     * For each address based predictor
        * Check how well they detect each pattern - Ideal , No storage limits
        * Check how well they detect each pattern - Realistic, with Limits
        * Analyse Results 
            * Does one have a good average for these types of patterns why ? 
            * Is there one that performs well across all patterns
        * Using a tournament approach Create inter combinations
            * Analyse 
        * Invesitigate 
            * a possible TAGE type that combines all.
            * Conflictng store disambiguation 
            * Limitting access pattern type  
*/
struct AccessTypeCounts{
    UINT64 stridedAccessesFrequency;
    UINT64 trulyIrregularFrequency;
    UINT64 IrregularButRepetitiveFrequency;
};

struct Metrics{
    UINT64 global_load_count;
    UINT64 global_store_count;
    std::vector<UINT> stride_distribution;
    /* window of load accesses, before an intermediate conflicting store  */
    std::array<UINT64, 1000> loadIntervalFrequency;   
    UINT64 ControlCorrelatedAccessCount;
    UINT64 GlobalCorrelatedAccessCount;
};