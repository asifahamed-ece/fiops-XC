/**
 * ciopt — C Implementation of FiOpt
 * AI-Powered Code Complexity & Optimization Engine for C
 * 
 * This is a port of the Python fiopt project to C language.
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

/* ============================================================================
 * Configuration and Constants
 * ============================================================================ */

/* Severity levels for analysis findings */
typedef enum {
    SEVERITY_INFO = 0,
    SEVERITY_WARNING = 1,
    SEVERITY_CRITICAL = 2
} Severity;

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
 * Intermediate Representation (IR)
 * ============================================================================ */

/* Opcodes for IR instructions */
typedef enum {
    IR_ASSIGN = 0,         /* variable = expression */
    IR_LOAD = 1,           /* load variable */
    IR_STORE = 2,          /* store to variable */
    IR_CALL = 3,           /* function call */
    IR_RETURN = 4,         /* return from function */
    IR_JUMP = 5,           /* unconditional jump */
    IR_BRANCH = 6,         /* conditional branch (if/elif) */
    IR_LOOP_HEADER = 7,    /* loop entry point (for/while) */
    IR_LOOP_END = 8,       /* loop exit point */
    IR_LOOP_CONTINUE = 9,  /* continue statement */
    IR_LOOP_BREAK = 10,    /* break statement */
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
    IR_NOP = 22            /* no-operation (placeholder) */
} IROpcode;

/* Type of loop construct */
typedef enum {
    LOOP_FOR = 0,
    LOOP_WHILE = 1,
    LOOP_LISTCOMP = 2,
    LOOP_SETCOMP = 3,
    LOOP_DICTCOMP = 4,
    LOOP_GENEXPR = 5
} LoopType;

/* A single instruction in the IR */
typedef struct IRNode {
    IROpcode opcode;
    int lineno;
    
    /* Variables read by this instruction */
    char** reads;
    int num_reads;
    
    /* Variables written by this instruction */
    char** writes;
    int num_writes;
    
    /* Additional metadata */
    char* label;
    char* target;
    
    /* For calls */
    char* call_target;
    int call_args;
    
    /* For loops */
    LoopType loop_type;
    char* loop_var;
    char* loop_iterable;
    
    struct IRNode* next;
} IRNode;

/* Create a new IR node */
IRNode* ir_node_create(IROpcode opcode, int lineno);

/* Free an IR node */
void ir_node_free(IRNode* node);

/* ============================================================================
 * Basic Block
 * ============================================================================ */

/* Classification of a basic block */
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

/* A basic block — a sequence of instructions with no internal branches */
typedef struct BasicBlock {
    int id;
    char* label;
    BlockType block_type;
    
    /* Instructions in this block */
    IRNode** instructions;
    int num_instructions;
    
    /* Control flow edges */
    int* successors;
    int num_successors;
    int* predecessors;
    int num_predecessors;
    
    /* Loop information */
    int loop_depth;
    int loop_header_id;
    
    /* Source line range */
    int start_line;
    int end_line;
    
    struct BasicBlock* next;
} BasicBlock;

/* Create a new basic block */
BasicBlock* basic_block_create(int id, const char* label, BlockType block_type);

/* Free a basic block */
void basic_block_free(BasicBlock* block);

/* Add an instruction to a basic block */
void basic_block_add_instruction(BasicBlock* block, IRNode* node);

/* Add a successor edge */
void basic_block_add_successor(BasicBlock* block, int successor_id);

/* Add a predecessor edge */
void basic_block_add_predecessor(BasicBlock* block, int predecessor_id);

/* Check if block contains any function calls */
bool basic_block_has_call(BasicBlock* block);

/* ============================================================================
 * Control Flow Graph
 * ============================================================================ */

/* Tracks information about a loop during CFG construction */
typedef struct {
    int header_block_id;
    int exit_block_id;
    LoopType loop_type;
    int lineno;
    char* loop_var;
    char* iterable_name;
    int depth;
} LoopInfo;

/* Control Flow Graph for a function or module */
typedef struct {
    char* name;
    BasicBlock** blocks;
    int num_blocks;
    int entry_block_id;
    int* exit_block_ids;
    int num_exit_blocks;
    LoopInfo* loops;
    int num_loops;
} CFG;

/* Create a new CFG */
CFG* cfg_create(const char* name);

/* Free a CFG */
void cfg_free(CFG* cfg);

/* Get entry block */
BasicBlock* cfg_entry_block(CFG* cfg);

/* Get maximum loop depth */
int cfg_max_loop_depth(CFG* cfg);

/* Export CFG to DOT format for visualization */
char* cfg_to_dot(CFG* cfg);

/* ============================================================================
 * Function Information
 * ============================================================================ */

typedef struct {
    char* name;
    int lineno;
    int end_lineno;
    int col_offset;
    char** args;
    int num_args;
    char** decorators;
    int num_decorators;
    char* docstring;
    bool is_method;
    bool is_async;
    bool is_generator;
    bool is_property;
    int body_line_count;
    char** nested_functions;
    int num_nested_functions;
} FunctionInfo;

/* Create function info */
FunctionInfo* function_info_create(void);

/* Free function info */
void function_info_free(FunctionInfo* info);

/* ============================================================================
 * Parsed Module
 * ============================================================================ */

typedef struct {
    char* source;
    size_t source_len;
    FunctionInfo** functions;
    int num_functions;
    int top_level_statements;
    int num_imports;
    char** global_variables;
    int num_global_variables;
} ParsedModule;

/* Create parsed module */
ParsedModule* parsed_module_create(void);

/* Free parsed module */
void parsed_module_free(ParsedModule* module);

/* ============================================================================
 * Source File
 * ============================================================================ */

typedef struct {
    char* path;
    char* content;
    int line_count;
    char* encoding;
} SourceFile;

/* Load a source file */
SourceFile* source_file_load(const char* path);

/* Free source file */
void source_file_free(SourceFile* file);

/* Get a specific line (1-indexed) */
char* source_file_get_line(SourceFile* file, int lineno);

/* ============================================================================
 * Loop Analysis
 * ============================================================================ */

/* Kind of loop */
typedef enum {
    LOOP_KIND_FOR = 0,
    LOOP_KIND_WHILE = 1,
    LOOP_KIND_LIST_COMP = 2,
    LOOP_KIND_SET_COMP = 3,
    LOOP_KIND_DICT_COMP = 4,
    LOOP_KIND_GEN_EXPR = 5
} LoopKind;

/* Information about a loop variable */
typedef struct {
    char* name;
    int lineno;
    char* iterable;
} LoopVariable;

/* Detailed information about a single loop */
typedef struct {
    LoopKind kind;
    int lineno;
    int end_lineno;
    int col_offset;
    int depth;              /* 1 = top-level, 2 = nested, etc. */
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
    LoopDetail* loops;
    int num_loops;
} LoopAnalysis;

/* Get maximum loop nesting depth */
int loop_analysis_max_depth(LoopAnalysis* analysis);

/* Get total number of loops */
int loop_analysis_total_loops(LoopAnalysis* analysis);

/* Free loop analysis */
void loop_analysis_free(LoopAnalysis* analysis);

/* ============================================================================
 * Recursion Analysis
 * ============================================================================ */

typedef struct {
    char* function_name;
    int lineno;
    int end_lineno;
    bool is_recursive;
    int* direct_calls;      /* line numbers of recursive calls */
    int num_direct_calls;
    bool has_base_case;
    int* base_case_lines;
    int num_base_case_lines;
    int estimated_branches;
    bool is_tail_recursive;
    int* tail_recursive_lines;
    int num_tail_recursive_lines;
    bool can_be_memoized;
    char* memoization_reason;
    bool has_overlapping_subproblems;
    char* depth_pattern;
} RecursionInfo;

/* Free recursion info */
void recursion_info_free(RecursionInfo* info);

/* ============================================================================
 * Complexity Analysis
 * ============================================================================ */

typedef struct {
    char* source;
    ComplexityClass complexity;
    int lineno;
    char* description;
    char* detail;
} ComplexityExplanation;

typedef struct {
    char* function_name;
    int lineno;
    int end_lineno;
    ComplexityClass estimated_complexity;
    float confidence;       /* 0.0 to 1.0 */
    ComplexityExplanation* explanations;
    int num_explanations;
    LoopAnalysis* loop_analysis;
    RecursionInfo* recursion_info;
    int* bottleneck_lines;
    int num_bottleneck_lines;
    char* bottleneck_description;
    char** warnings;
    int num_warnings;
} ComplexityResult;

/* Check if complexity is efficient (<= O(n log n)) */
bool complexity_is_efficient(ComplexityResult* result);

/* Get summary string */
char* complexity_summary(ComplexityResult* result);

/* Free complexity result */
void complexity_result_free(ComplexityResult* result);

/* ============================================================================
 * Anti-Pattern Detection
 * ============================================================================ */

typedef enum {
    PATTERN_SEVERITY_INFO = 0,
    PATTERN_SEVERITY_WARNING = 1,
    PATTERN_SEVERITY_CRITICAL = 2
} PatternSeverity;

typedef enum {
    CATEGORY_DATA_STRUCTURE = 0,
    CATEGORY_STRING_OPERATION = 1,
    CATEGORY_ALGORITHM = 2,
    CATEGORY_IO_OPERATION = 3,
    CATEGORY_MEMORY = 4,
    CATEGORY_COMPUTATION = 5
} PatternCategory;

typedef struct {
    char* name;
    PatternCategory category;
    PatternSeverity severity;
    int lineno;
    int end_lineno;
    char* description;
    char* suggestion;
    char* estimated_impact;
    char* code_snippet;
} AntiPattern;

typedef struct {
    char* function_name;
    AntiPattern* anti_patterns;
    int num_anti_patterns;
} PatternAnalysis;

/* Get count of critical patterns */
int pattern_analysis_critical_count(PatternAnalysis* analysis);

/* Get count of warning patterns */
int pattern_analysis_warning_count(PatternAnalysis* analysis);

/* Free pattern analysis */
void pattern_analysis_free(PatternAnalysis* analysis);

/* ============================================================================
 * Data Structure Analysis
 * ============================================================================ */

typedef struct {
    char* variable_name;
    char* current_type;
    char* suggested_type;
    int lineno;
    char* description;
    char* suggestion;
    char* estimated_impact;
} DataStructureIssue;

typedef struct {
    char* function_name;
    DataStructureIssue* issues;
    int num_issues;
} DataStructureAnalysis;

/* Free data structure analysis */
void data_structure_analysis_free(DataStructureAnalysis* analysis);

/* ============================================================================
 * Dead Code Analysis
 * ============================================================================ */

typedef struct {
    char* kind;             /* "unreachable", "unused_variable", etc. */
    int lineno;
    int end_lineno;
    char* name;
    char* description;
    char* suggestion;
} DeadCodeItem;

typedef struct {
    DeadCodeItem* items;
    int num_items;
} DeadCodeAnalysis;

/* Get total count */
int dead_code_total(DeadCodeAnalysis* analysis);

/* Get unreachable code count */
int dead_code_unreachable_count(DeadCodeAnalysis* analysis);

/* Free dead code analysis */
void dead_code_analysis_free(DeadCodeAnalysis* analysis);

/* ============================================================================
 * Function Report
 * ============================================================================ */

typedef struct {
    char* name;
    int lineno;
    int end_lineno;
    int line_count;
    ComplexityResult* complexity;
    PatternAnalysis* patterns;
    DataStructureAnalysis* data_structure;
    Severity severity;
} FunctionReport;

/* Get total issues count */
int function_report_total_issues(FunctionReport* report);

/* Get summary line */
char* function_report_summary(FunctionReport* report);

/* Free function report */
void function_report_free(FunctionReport* report);

/* ============================================================================
 * File Report
 * ============================================================================ */

typedef struct {
    char* filepath;
    int line_count;
    FunctionReport** function_reports;
    int num_function_reports;
    DeadCodeAnalysis* dead_code;
    char** parse_errors;
    int num_parse_errors;
} FileReport;

/* Get worst complexity in file */
ComplexityClass file_report_worst_complexity(FileReport* report);

/* Get total functions */
int file_report_total_functions(FileReport* report);

/* Get total issues */
int file_report_total_issues(FileReport* report);

/* Get critical functions */
FunctionReport** file_report_critical_functions(FileReport* report, int* count);

/* Free file report */
void file_report_free(FileReport* report);

/* ============================================================================
 * Analysis Report
 * ============================================================================ */

typedef struct {
    FileReport** files;
    int num_files;
    char* timestamp;
    double analysis_duration_ms;
    char* fiopt_version;
} AnalysisReport;

/* Get total files */
int analysis_report_total_files(AnalysisReport* report);

/* Get total functions */
int analysis_report_total_functions(AnalysisReport* report);

/* Get total lines */
int analysis_report_total_lines(AnalysisReport* report);

/* Get total issues */
int analysis_report_total_issues(AnalysisReport* report);

/* Get worst complexity */
ComplexityClass analysis_report_worst_complexity(AnalysisReport* report);

/* Get complexity string */
const char* analysis_report_complexity(AnalysisReport* report);

/* Get bottlenecks */
char** analysis_report_bottlenecks(AnalysisReport* report, int* count);

/* Get suggestions */
char** analysis_report_suggestions(AnalysisReport* report, int* count);

/* Get human-readable summary */
char* analysis_report_summary(AnalysisReport* report);

/* Free analysis report */
void analysis_report_free(AnalysisReport* report);

/* ============================================================================
 * Parser Functions
 * ============================================================================ */

/* Parse Python source code into a ParsedModule */
ParsedModule* parse_source(const char* source, const char* filename);

/* Load a Python file */
SourceFile* load_file(const char* path);

/* Scan a directory for Python files */
SourceFile** scan_project(const char* directory, int* count);

/* ============================================================================
 * Analyzer Functions
 * ============================================================================ */

/* Detect and analyze loops in a function */
LoopAnalysis* detect_loops(const char* source, const char* func_name);

/* Detect recursion patterns */
RecursionInfo* detect_recursion(const char* source, const char* func_name);

/* Estimate complexity of a function */
ComplexityResult* estimate_complexity(const char* source, const char* func_name);

/* Detect anti-patterns */
PatternAnalysis* detect_patterns(const char* source, const char* func_name);

/* Detect data structure issues */
DataStructureAnalysis* detect_data_structure_issues(const char* source, const char* func_name);

/* Detect dead code */
DeadCodeAnalysis* detect_dead_code(const char* source);

/* ============================================================================
 * Main API
 * ============================================================================ */

/* Analyze a Python file or directory */
AnalysisReport* analyze(const char* path, AnalysisConfig* config);

/* Analyze source code from a string */
AnalysisReport* analyze_source(const char* source, const char* filename, AnalysisConfig* config);

/* ============================================================================
 * Reporting Functions
 * ============================================================================ */

/* Render report to terminal */
void render_terminal(AnalysisReport* report, bool verbose);

/* Save HTML report */
int save_html_report(AnalysisReport* report, const char* output_path);

/* Render JSON report */
char* render_json(AnalysisReport* report);

/* Save JSON report */
int save_json_report(AnalysisReport* report, const char* output_path);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/* Get current timestamp as ISO string */
char* get_timestamp_iso(void);

/* String utilities */
char* str_duplicate(const char* s);
char** str_split(const char* s, char delimiter, int* count);
void str_array_free(char** arr, int count);

#endif /* CIOPT_H */
