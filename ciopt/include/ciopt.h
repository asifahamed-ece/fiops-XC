/**
 * ciopt — C Implementation of FiOpt
 * AI-Powered Code Complexity & Optimization Engine for C
 * 
 * This is a complete port of the Python fiopt project to C language.
 */

#ifndef CIOPT_H
#define CIOPT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/* Version information */
#define CIOPT_VERSION_MAJOR 0
#define CIOPT_VERSION_MINOR 1
#define CIOPT_VERSION_PATCH 0
#define CIOPT_VERSION "0.1.0"

/* Maximum limits */
#define CIOPT_MAX_STRING_LEN 4096
#define CIOPT_MAX_NAME_LEN 256
#define CIOPT_MAX_PATH_LEN 1024
#define CIOPT_MAX_FUNCTIONS 10000
#define CIOPT_MAX_LOOPS 1000
#define CIOPT_MAX_ISSUES 5000

/* ============================================================================
 * Configuration and Constants
 * ============================================================================ */

/* Severity levels for analysis findings */
typedef enum {
    SEVERITY_INFO = 0,
    SEVERITY_WARNING = 1,
    SEVERITY_CRITICAL = 2
} Severity;

/* Pattern categories for anti-patterns */
typedef enum {
    PATTERN_DATA_STRUCTURE = 0,
    PATTERN_STRING_OPERATION = 1,
    PATTERN_ALGORITHM = 2,
    PATTERN_IO_OPERATION = 3,
    PATTERN_MEMORY = 4,
    PATTERN_COMPUTATION = 5
} PatternCategory;

/* Supported output formats */
typedef enum {
    FORMAT_TERMINAL = 0,
    FORMAT_HTML = 1,
    FORMAT_JSON = 2
} ReportFormat;

/* Known algorithmic complexity classes, ordered from best to worst */
typedef enum {
    O_1 = 0,
    O_LOG_N = 1,
    O_N = 2,
    O_N_LOG_N = 3,
    O_N_SQUARED = 4,
    O_N_CUBED = 5,
    O_N_K = 6,       /* polynomial, k > 3 */
    O_2_N = 7,       /* exponential */
    O_N_FACTORIAL = 8,
    O_UNKNOWN = 9
} ComplexityClass;

/* Loop types */
typedef enum {
    LOOP_FOR = 0,
    LOOP_WHILE = 1,
    LOOP_LIST_COMP = 2,
    LOOP_SET_COMP = 3,
    LOOP_DICT_COMP = 4,
    LOOP_GEN_EXPR = 5
} LoopType;

/* Block types for CFG */
typedef enum {
    BLOCK_ENTRY = 0,
    BLOCK_EXIT = 1,
    BLOCK_NORMAL = 2,
    BLOCK_LOOP_HEADER = 3,
    BLOCK_LOOP_BODY = 4,
    BLOCK_LOOP_EXIT = 5,
    BLOCK_BRANCH_TRUE = 6,
    BLOCK_BRANCH_FALSE = 7,
    BLOCK_EXCEPT_HANDLER = 8,
    BLOCK_FINALLY_BLOCK = 9
} BlockType;

/* IR Opcodes */
typedef enum {
    IR_ASSIGN = 0,
    IR_LOAD = 1,
    IR_STORE = 2,
    IR_CALL = 3,
    IR_RETURN = 4,
    IR_JUMP = 5,
    IR_BRANCH = 6,
    IR_LOOP_HEADER = 7,
    IR_LOOP_END = 8,
    IR_LOOP_CONTINUE = 9,
    IR_LOOP_BREAK = 10,
    IR_TRY_ENTER = 11,
    IR_TRY_EXIT = 12,
    IR_EXCEPT = 13,
    IR_FINALLY = 14,
    IR_WITH_ENTER = 15,
    IR_WITH_EXIT = 16,
    IR_YIELD = 17,
    IR_AWAIT = 18,
    IR_RAISE = 19,
    IR_ASSERT = 20,
    IR_PASS = 21,
    IR_NOP = 22
} IROpcode;

/* Dead code kinds */
typedef enum {
    DEAD_UNREACHABLE = 0,
    DEAD_UNUSED_VARIABLE = 1,
    DEAD_UNUSED_IMPORT = 2,
    DEAD_UNUSED_FUNCTION = 3
} DeadCodeKind;

/* Get numeric rank for complexity comparison (higher = worse) */
int complexity_rank(ComplexityClass c);

/* Compare two complexity classes */
int complexity_compare(ComplexityClass c1, ComplexityClass c2);

/* Get string representation of complexity class */
const char* complexity_to_string(ComplexityClass c);

/* Combine two complexity classes (for nested loops) */
ComplexityClass combine_complexities(ComplexityClass c1, ComplexityClass c2);

/* ============================================================================
 * Analysis Configuration
 * ============================================================================ */

typedef struct {
    ComplexityClass complexity_warning_threshold;
    ComplexityClass complexity_critical_threshold;
    int max_nesting_depth;
    bool detect_dead_code;
    bool detect_anti_patterns;
    bool detect_data_structure_issues;
    bool detect_recursion_issues;
    ReportFormat report_format;
    char* output_path;
    bool include_source;
    bool verbose;
    char** file_extensions;
    int num_extensions;
    char** exclude_dirs;
    int num_exclude_dirs;
} AnalysisConfig;

/* Create default analysis configuration */
AnalysisConfig* config_create_default(void);

/* Free analysis configuration */
void config_free(AnalysisConfig* config);

/* ============================================================================
 * Intermediate Representation (IR) Nodes
 * ============================================================================ */

/* A single instruction in the IR */
typedef struct IRNode {
    IROpcode opcode;
    int lineno;
    char** reads;           /* Variables read by this instruction */
    int num_reads;
    char** writes;          /* Variables written by this instruction */
    int num_writes;
    char* label;            /* Human-readable label */
    char* target;           /* Jump/branch target label */
    char* call_target;      /* Function being called */
    int call_args;          /* Number of arguments */
    LoopType loop_type;     /* For loops */
    char* loop_var;         /* Iteration variable */
    char* loop_iterable;    /* What is being iterated over */
    struct IRNode* next;    /* Next instruction in block */
} IRNode;

/* Create a new IR node */
IRNode* ir_node_create(IROpcode opcode, int lineno);

/* Free an IR node */
void ir_node_free(IRNode* node);

/* ============================================================================
 * Basic Blocks for CFG
 * ============================================================================ */

/* Forward declaration */
struct BasicBlock;

/* A basic block — sequence of instructions with no internal branches */
typedef struct BasicBlock {
    int id;
    char* label;
    BlockType block_type;
    IRNode* instructions;       /* Linked list of instructions */
    int num_instructions;
    int* successors;            /* Block IDs */
    int num_successors;
    int* predecessors;          /* Block IDs */
    int num_predecessors;
    int loop_depth;             /* Nesting depth */
    int loop_header_id;         /* ID of loop header this block belongs to */
    int start_line;             /* Source line range */
    int end_line;
    struct BasicBlock* next;    /* Next block in linked list */
} BasicBlock;

/* Create a new basic block */
BasicBlock* basic_block_create(int id, const char* label, BlockType block_type);

/* Free a basic block */
void basic_block_free(BasicBlock* block);

/* Add instruction to block */
void basic_block_add_instruction(BasicBlock* block, IRNode* node);

/* Add successor edge */
void basic_block_add_successor(BasicBlock* block, int succ_id);

/* Add predecessor edge */
void basic_block_add_predecessor(BasicBlock* block, int pred_id);

/* ============================================================================
 * Control Flow Graph (CFG)
 * ============================================================================ */

/* Loop information for CFG */
typedef struct {
    int header_block_id;
    int exit_block_id;
    LoopType loop_type;
    int lineno;
    char* loop_var;
    char* iterable_name;
    int depth;
} CFGLoopInfo;

/* Control Flow Graph for a function or module */
typedef struct {
    char* name;                 /* Function name or "<module>" */
    BasicBlock* blocks;         /* Linked list of blocks */
    int block_count;
    int entry_block_id;
    int* exit_block_ids;
    int num_exit_blocks;
    CFGLoopInfo* loops;
    int num_loops;
} CFG;

/* Create a new CFG */
CFG* cfg_create(const char* name);

/* Free a CFG */
void cfg_free(CFG* cfg);

/* Export CFG to DOT format */
char* cfg_to_dot(CFG* cfg);

/* Get back edges (indicates loops) */
typedef struct {
    int from_block;
    int to_block;
} BackEdge;

BackEdge* cfg_get_back_edges(CFG* cfg, int* num_edges);

/* ============================================================================
 * Loop Detection and Analysis
 * ============================================================================ */

/* Loop variable information */
typedef struct {
    char* name;
    int lineno;
    char* iterable;
} LoopVariable;

/* Detailed information about a single loop */
typedef struct LoopDetail {
    LoopType kind;
    int lineno;
    int end_lineno;
    int col_offset;
    int depth;                  /* 1 = top-level, 2 = nested once, etc. */
    struct LoopDetail* parent;
    struct LoopDetail** children;
    int num_children;
    LoopVariable* variables;
    int num_variables;
    int body_line_count;
    bool contains_break;
    bool contains_continue;
    bool contains_return;
    bool has_invariant_code;
    int* invariant_lines;
    int num_invariant_lines;
    bool has_expensive_operation;
    char** expensive_operations;
    int num_expensive_operations;
    char** function_calls;
    int num_function_calls;
} LoopDetail;

/* Complete loop analysis results for a function */
typedef struct {
    char* function_name;
    LoopDetail** loops;
    int num_loops;
} LoopAnalysis;

/* Create loop analysis */
LoopAnalysis* loop_analysis_create(const char* func_name);

/* Free loop analysis */
void loop_analysis_free(LoopAnalysis* analysis);

/* Detect loops in source code */
LoopAnalysis* detect_loops(const char* source_code, const char* func_name);

/* ============================================================================
 * Recursion Detection and Analysis
 * ============================================================================ */

/* Information about recursion in a function */
typedef struct {
    char* function_name;
    int lineno;
    int end_lineno;
    bool is_recursive;
    int* direct_calls;          /* Line numbers of recursive calls */
    int num_direct_calls;
    char** mutual_call_funcs;   /* Functions involved in mutual recursion */
    int* mutual_call_lines;
    int num_mutual_calls;
    bool has_base_case;
    int* base_case_lines;
    int num_base_cases;
    int estimated_branches;     /* How many recursive calls per invocation */
    bool is_tail_recursive;
    int* tail_recursive_lines;
    int num_tail_recursive;
    bool can_be_memoized;
    char* memoization_reason;
    bool has_overlapping_subproblems;
    char* depth_pattern;        /* "linear", "logarithmic", "exponential" */
} RecursionInfo;

/* Create recursion info */
RecursionInfo* recursion_info_create(const char* func_name);

/* Free recursion info */
void recursion_info_free(RecursionInfo* info);

/* Detect recursion in source code */
RecursionInfo* detect_recursion(const char* source_code, const char* func_name);

/* ============================================================================
 * Data Structure Analysis
 * ============================================================================ */

/* A detected data structure misuse */
typedef struct {
    char* variable_name;
    char* current_type;
    char* suggested_type;
    int lineno;
    char* description;
    char* suggestion;
    char* estimated_impact;
} DataStructureIssue;

/* Data structure analysis results for a function */
typedef struct {
    char* function_name;
    DataStructureIssue** issues;
    int num_issues;
} DataStructureAnalysis;

/* Create data structure analysis */
DataStructureAnalysis* data_structure_analysis_create(const char* func_name);

/* Free data structure analysis */
void data_structure_analysis_free(DataStructureAnalysis* analysis);

/* Detect data structure issues */
DataStructureAnalysis* detect_data_structure_issues(const char* source_code, const char* func_name);

/* ============================================================================
 * Dead Code Detection
 * ============================================================================ */

/* A piece of detected dead code */
typedef struct {
    DeadCodeKind kind;
    int lineno;
    int end_lineno;
    char* name;
    char* description;
    char* suggestion;
} DeadCodeItem;

/* Dead code analysis results */
typedef struct {
    DeadCodeItem** items;
    int num_items;
} DeadCodeAnalysis;

/* Create dead code analysis */
DeadCodeAnalysis* dead_code_analysis_create(void);

/* Free dead code analysis */
void dead_code_analysis_free(DeadCodeAnalysis* analysis);

/* Detect dead code */
DeadCodeAnalysis* detect_dead_code(const char* source_code);

/* ============================================================================
 * Anti-Pattern Detection
 * ============================================================================ */

/* A detected anti-pattern */
typedef struct {
    char* name;
    PatternCategory category;
    Severity severity;
    int lineno;
    int end_lineno;
    char* description;
    char* suggestion;
    char* estimated_impact;
    char* code_snippet;
} AntiPattern;

/* Results of anti-pattern detection for a function */
typedef struct {
    char* function_name;
    AntiPattern** anti_patterns;
    int num_patterns;
} PatternAnalysis;

/* Create pattern analysis */
PatternAnalysis* pattern_analysis_create(const char* func_name);

/* Free pattern analysis */
void pattern_analysis_free(PatternAnalysis* analysis);

/* Detect anti-patterns */
PatternAnalysis* detect_patterns(const char* source_code, const char* func_name);

/* ============================================================================
 * Complexity Estimation
 * ============================================================================ */

/* Explanation for a complexity component */
typedef struct {
    char* source;               /* "loop", "recursion", "call", etc. */
    ComplexityClass complexity;
    int lineno;
    char* description;
    char* detail;
} ComplexityExplanation;

/* Result of complexity estimation for a single function */
typedef struct {
    char* function_name;
    int lineno;
    int end_lineno;
    ComplexityClass estimated_complexity;
    float confidence;           /* 0.0 (guess) to 1.0 (certain) */
    ComplexityExplanation** explanations;
    int num_explanations;
    LoopAnalysis* loop_analysis;
    RecursionInfo* recursion_info;
    int* bottleneck_lines;
    int num_bottleneck_lines;
    char* bottleneck_description;
    char** warnings;
    int num_warnings;
} ComplexityResult;

/* Create complexity result */
ComplexityResult* complexity_result_create(const char* func_name);

/* Free complexity result */
void complexity_result_free(ComplexityResult* result);

/* Estimate complexity */
ComplexityResult* estimate_complexity(const char* source_code, const char* func_name);

/* ============================================================================
 * Function Analysis Report
 * ============================================================================ */

/* Comprehensive analysis report for a single function */
typedef struct {
    char* function_name;
    char* file_path;
    int lineno;
    int end_lineno;
    int line_count;
    ComplexityResult* complexity;
    LoopAnalysis* loops;
    RecursionInfo* recursion;
    DataStructureAnalysis* data_structures;
    DeadCodeAnalysis* dead_code;
    PatternAnalysis* patterns;
    CFG* cfg;
} FunctionReport;

/* Create function report with all analysis data */
FunctionReport* function_report_create(
    IRFunction* func,
    CFG* cfg,
    ComplexityResult* complexity,
    LoopAnalysis* loops,
    RecursionInfo* recursion,
    DataStructureAnalysis* ds_issues,
    DeadCodeAnalysis* dead_code,
    PatternAnalysis* patterns
);

/* Free function report */
void function_report_free(FunctionReport* report);

/* ============================================================================
 * File Analysis Report
 * ============================================================================ */

/* Analysis report for a single file */
typedef struct {
    char* file_path;
    size_t file_size;
    int total_lines;
    int code_lines;
    int comment_lines;
    int blank_lines;
    FunctionReport** functions;
    int num_functions;
    int import_count;
    char** imports;
    char** classes;
    int num_classes;
    double maintainability_index;
} FileReport;

/* Create file report with functions */
FileReport* file_report_create(
    const char* file_path,
    FunctionReport** functions,
    int num_functions
);

/* Free file report */
void file_report_free(FileReport* report);

/* ============================================================================
 * Project Analysis Report
 * ============================================================================ */

/* Summary statistics for entire project */
typedef struct {
    int total_files;
    int total_functions;
    int total_lines;
    int total_code_lines;
    double avg_function_length;
    double avg_complexity;
    int max_complexity;
    int critical_issues;
    int warning_issues;
    int info_issues;
    char* most_complex_function;
    char* largest_file;
} AnalysisSummary;

/* Complete analysis report for a project */
typedef struct {
    char* project_path;
    time_t analysis_timestamp;
    char* fiopt_version;
    AnalysisConfig* config;
    FileReport** files;
    int num_files;
    AnalysisSummary summary;
    char** errors;
    int num_errors;
    char** suggestions;
    int num_suggestions;
} AnalysisReport;

/* Create analysis report */
AnalysisReport* analysis_report_create(const char* project_path);

/* Add file report to analysis report */
void analysis_report_add_file(AnalysisReport* report, FileReport* file_report);

/* Finalize analysis report (compute summary) */
void analysis_report_finalize(AnalysisReport* report, AnalysisConfig* config);

/* Free analysis report */
void analysis_report_free(AnalysisReport* report);

/* ============================================================================
 * Source Loading and Parsing
 * ============================================================================ */

/* Load source code from file */
char* load_source_file(const char* path);

/* Scan directory for source files */
typedef struct {
    char** paths;
    int num_paths;
} FileList;

FileList* scan_directory(const char* dir_path, const char** extensions, int num_extensions);

/* Free file list */
void file_list_free(FileList* list);

/* Parse C source file into IR function */
typedef struct IRFunction {
    char* name;
    CFG* cfg;
    struct IRNodeList* body;
} IRFunction;

IRFunction* ciopt_parse_file(const char* filename);

/* Analyze a C file with full pipeline (parse + analyze + report) */
AnalysisReport* ciopt_analyze_c_file(const char* filename, const AnalysisConfig* config);

/* ============================================================================
 * Main Analysis Engine
 * ============================================================================ */

/* Analyze a single source file */
FileReport* analyze_file(const char* file_path, AnalysisConfig* config);

/* Analyze entire project */
AnalysisReport* analyze_project(const char* project_path, AnalysisConfig* config);

/* ============================================================================
 * Report Generation
 * ============================================================================ */

/* Generate terminal report */
char* generate_terminal_report(AnalysisReport* report);

/* Generate HTML report */
char* generate_html_report(AnalysisReport* report);

/* Generate JSON report */
char* generate_json_report(AnalysisReport* report);

/* Write report to file */
int write_report_to_file(const char* content, const char* path);

/* Print summary to terminal */
void print_summary(AnalysisReport* report);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/* String utilities */
char* ciopt_strdup(const char* s);
char** ciopt_split_string(const char* s, char delimiter, int* count);
void ciopt_free_string_array(char** arr, int count);

/* Get complexity class from string */
ComplexityClass complexity_from_string(const char* s);

/* Get severity string */
const char* severity_to_string(Severity s);

/* Get pattern category string */
const char* pattern_category_to_string(PatternCategory c);

#endif /* CIOPT_H */
