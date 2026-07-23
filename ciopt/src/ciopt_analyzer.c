/**
 * ciopt_analyzer.c - Analysis engine for CiOpt
 * 
 * This implements the main analysis functions, ported from Python FiOpt.
 * Note: Full AST parsing in C would require a Python embedding or external parser.
 * These are stub implementations that demonstrate the structure.
 */

#include "ciopt.h"
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>

/* ============================================================================
 * Stub Analysis Functions
 * 
 * In a full implementation, these would parse Python/C source code using
 * an AST parser (like libpython for Python or clang for C).
 * For this port, we provide stub implementations that show the structure.
 * ============================================================================ */

/* Stub: Load source file */
char* load_source_file(const char* path) {
    if (!path) return NULL;
    
    FILE* f = fopen(path, "r");
    if (!f) return NULL;
    
    /* Get file size */
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (size <= 0) {
        fclose(f);
        return NULL;
    }
    
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }
    
    size_t read_size = fread(buffer, 1, size, f);
    buffer[read_size] = '\0';
    fclose(f);
    
    return buffer;
}

/* Stub: Scan directory for source files */
FileList* scan_directory(const char* dir_path, const char** extensions, int num_extensions) {
    if (!dir_path) return NULL;
    
    FileList* list = (FileList*)calloc(1, sizeof(FileList));
    if (!list) return NULL;
    
    DIR* dir = opendir(dir_path);
    if (!dir) {
        free(list);
        return NULL;
    }
    
    list->paths = NULL;
    list->num_paths = 0;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;  /* Skip hidden files */
        
        /* Check extension */
        bool match = false;
        const char* ext = strrchr(entry->d_name, '.');
        if (ext) {
            for (int i = 0; i < num_extensions; i++) {
                if (strcmp(ext, extensions[i]) == 0) {
                    match = true;
                    break;
                }
            }
        }
        
        if (!match) continue;
        
        /* Build full path */
        char full_path[CIOPT_MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        /* Check if it's a regular file */
        struct stat st;
        if (stat(full_path, &st) != 0 || !S_ISREG(st.st_mode)) continue;
        
        /* Add to list */
        list->num_paths++;
        list->paths = (char**)realloc(list->paths, list->num_paths * sizeof(char*));
        if (list->paths) {
            list->paths[list->num_paths - 1] = ciopt_strdup(full_path);
        }
    }
    
    closedir(dir);
    return list;
}

void file_list_free(FileList* list) {
    if (!list) return;
    ciopt_free_string_array(list->paths, list->num_paths);
    free(list);
}

/* Stub: Loop detection - In real implementation would parse AST */
LoopAnalysis* detect_loops(const char* source_code, const char* func_name) {
    (void)source_code;  /* Would be parsed in real implementation */
    
    LoopAnalysis* analysis = loop_analysis_create(func_name);
    /* In real implementation: parse AST and detect loops */
    return analysis;
}

LoopAnalysis* loop_analysis_create(const char* func_name) {
    LoopAnalysis* analysis = (LoopAnalysis*)calloc(1, sizeof(LoopAnalysis));
    if (!analysis) return NULL;
    
    analysis->function_name = ciopt_strdup(func_name ? func_name : "<unknown>");
    analysis->loops = NULL;
    analysis->num_loops = 0;
    
    return analysis;
}

void loop_analysis_free(LoopAnalysis* analysis) {
    if (!analysis) return;
    free(analysis->function_name);
    
    for (int i = 0; i < analysis->num_loops; i++) {
        LoopDetail* loop = analysis->loops[i];
        if (loop) {
            for (int j = 0; j < loop->num_variables; j++) {
                free(loop->variables[j].name);
                free(loop->variables[j].iterable);
            }
            free(loop->variables);
            
            for (int j = 0; j < loop->num_invariant_lines; j++) {
                /* invariant_lines is int array */
            }
            free(loop->invariant_lines);
            
            ciopt_free_string_array(loop->expensive_operations, loop->num_expensive_operations);
            ciopt_free_string_array(loop->function_calls, loop->num_function_calls);
            
            free(loop->children);
            free(loop);
        }
    }
    free(analysis->loops);
    free(analysis);
}

/* Stub: Recursion detection */
RecursionInfo* detect_recursion(const char* source_code, const char* func_name) {
    (void)source_code;
    
    RecursionInfo* info = recursion_info_create(func_name);
    /* In real implementation: parse AST and detect recursion */
    return info;
}

RecursionInfo* recursion_info_create(const char* func_name) {
    RecursionInfo* info = (RecursionInfo*)calloc(1, sizeof(RecursionInfo));
    if (!info) return NULL;
    
    info->function_name = ciopt_strdup(func_name ? func_name : "<unknown>");
    info->lineno = 0;
    info->end_lineno = 0;
    info->is_recursive = false;
    info->direct_calls = NULL;
    info->num_direct_calls = 0;
    info->mutual_call_funcs = NULL;
    info->mutual_call_lines = NULL;
    info->num_mutual_calls = 0;
    info->has_base_case = false;
    info->base_case_lines = NULL;
    info->num_base_cases = 0;
    info->estimated_branches = 0;
    info->is_tail_recursive = false;
    info->tail_recursive_lines = NULL;
    info->num_tail_recursive = 0;
    info->can_be_memoized = false;
    info->memoization_reason = NULL;
    info->has_overlapping_subproblems = false;
    info->depth_pattern = NULL;
    
    return info;
}

void recursion_info_free(RecursionInfo* info) {
    if (!info) return;
    free(info->function_name);
    free(info->direct_calls);
    
    ciopt_free_string_array(info->mutual_call_funcs, info->num_mutual_calls);
    free(info->mutual_call_lines);
    free(info->base_case_lines);
    free(info->tail_recursive_lines);
    free(info->memoization_reason);
    free(info->depth_pattern);
    free(info);
}

/* Stub: Data structure analysis */
DataStructureAnalysis* detect_data_structure_issues(const char* source_code, const char* func_name) {
    (void)source_code;
    
    DataStructureAnalysis* analysis = data_structure_analysis_create(func_name);
    return analysis;
}

DataStructureAnalysis* data_structure_analysis_create(const char* func_name) {
    DataStructureAnalysis* analysis = (DataStructureAnalysis*)calloc(1, sizeof(DataStructureAnalysis));
    if (!analysis) return NULL;
    
    analysis->function_name = ciopt_strdup(func_name ? func_name : "<unknown>");
    analysis->issues = NULL;
    analysis->num_issues = 0;
    
    return analysis;
}

void data_structure_analysis_free(DataStructureAnalysis* analysis) {
    if (!analysis) return;
    free(analysis->function_name);
    
    for (int i = 0; i < analysis->num_issues; i++) {
        DataStructureIssue* issue = analysis->issues[i];
        if (issue) {
            free(issue->variable_name);
            free(issue->current_type);
            free(issue->suggested_type);
            free(issue->description);
            free(issue->suggestion);
            free(issue->estimated_impact);
            free(issue);
        }
    }
    free(analysis->issues);
    free(analysis);
}

/* Stub: Dead code detection */
DeadCodeAnalysis* detect_dead_code(const char* source_code) {
    (void)source_code;
    
    DeadCodeAnalysis* analysis = dead_code_analysis_create();
    return analysis;
}

DeadCodeAnalysis* dead_code_analysis_create(void) {
    DeadCodeAnalysis* analysis = (DeadCodeAnalysis*)calloc(1, sizeof(DeadCodeAnalysis));
    if (!analysis) return NULL;
    
    analysis->items = NULL;
    analysis->num_items = 0;
    
    return analysis;
}

void dead_code_analysis_free(DeadCodeAnalysis* analysis) {
    if (!analysis) return;
    
    for (int i = 0; i < analysis->num_items; i++) {
        DeadCodeItem* item = analysis->items[i];
        if (item) {
            free(item->name);
            free(item->description);
            free(item->suggestion);
            free(item);
        }
    }
    free(analysis->items);
    free(analysis);
}

/* Stub: Pattern detection */
PatternAnalysis* detect_patterns(const char* source_code, const char* func_name) {
    (void)source_code;
    
    PatternAnalysis* analysis = pattern_analysis_create(func_name);
    return analysis;
}

PatternAnalysis* pattern_analysis_create(const char* func_name) {
    PatternAnalysis* analysis = (PatternAnalysis*)calloc(1, sizeof(PatternAnalysis));
    if (!analysis) return NULL;
    
    analysis->function_name = ciopt_strdup(func_name ? func_name : "<unknown>");
    analysis->anti_patterns = NULL;
    analysis->num_patterns = 0;
    
    return analysis;
}

void pattern_analysis_free(PatternAnalysis* analysis) {
    if (!analysis) return;
    free(analysis->function_name);
    
    for (int i = 0; i < analysis->num_patterns; i++) {
        AntiPattern* pattern = analysis->anti_patterns[i];
        if (pattern) {
            free(pattern->name);
            free(pattern->description);
            free(pattern->suggestion);
            free(pattern->estimated_impact);
            free(pattern->code_snippet);
            free(pattern);
        }
    }
    free(analysis->anti_patterns);
    free(analysis);
}

/* Stub: Complexity estimation */
ComplexityResult* estimate_complexity(const char* source_code, const char* func_name) {
    (void)source_code;
    
    ComplexityResult* result = complexity_result_create(func_name);
    result->estimated_complexity = O_N;  /* Default assumption */
    result->confidence = 0.5;
    
    /* Run sub-analyses */
    result->loop_analysis = detect_loops(source_code, func_name);
    result->recursion_info = detect_recursion(source_code, func_name);
    
    return result;
}

ComplexityResult* complexity_result_create(const char* func_name) {
    ComplexityResult* result = (ComplexityResult*)calloc(1, sizeof(ComplexityResult));
    if (!result) return NULL;
    
    result->function_name = ciopt_strdup(func_name ? func_name : "<unknown>");
    result->lineno = 0;
    result->end_lineno = 0;
    result->estimated_complexity = O_UNKNOWN;
    result->confidence = 0.0;
    result->explanations = NULL;
    result->num_explanations = 0;
    result->loop_analysis = NULL;
    result->recursion_info = NULL;
    result->bottleneck_lines = NULL;
    result->num_bottleneck_lines = 0;
    result->bottleneck_description = NULL;
    result->warnings = NULL;
    result->num_warnings = 0;
    
    return result;
}

void complexity_result_free(ComplexityResult* result) {
    if (!result) return;
    free(result->function_name);
    
    for (int i = 0; i < result->num_explanations; i++) {
        ComplexityExplanation* exp = result->explanations[i];
        if (exp) {
            free(exp->source);
            free(exp->description);
            free(exp->detail);
            free(exp);
        }
    }
    free(result->explanations);
    
    loop_analysis_free(result->loop_analysis);
    recursion_info_free(result->recursion_info);
    free(result->bottleneck_lines);
    free(result->bottleneck_description);
    ciopt_free_string_array(result->warnings, result->num_warnings);
    free(result);
}

/* ============================================================================
 * Main Analysis Functions
 * ============================================================================ */

FileReport* analyze_file(const char* file_path, AnalysisConfig* config) {
    if (!file_path) return NULL;
    (void)config; /* Unused for now */
    
    /* Create empty file report */
    FileReport* report = file_report_create(file_path, NULL, 0);
    if (!report) return NULL;
    
    /* Load source */
    char* source = load_source_file(file_path);
    if (!source) {
        /* Could not load file */
        return report;
    }
    
    /* Count lines */
    report->file_size = strlen(source);
    int total = 0, code = 0, comments = 0, blank = 0;
    
    char* line = source;
    char* next;
    while ((next = strchr(line, '\n')) != NULL || *line) {
        total++;
        
        /* Find line length */
        int len = next ? (next - line) : (int)strlen(line);
        
        /* Check line type */
        bool is_blank = true;
        bool is_comment = false;
        
        for (int i = 0; i < len; i++) {
            if (!isspace(line[i])) {
                is_blank = false;
                if (line[i] == '#') {
                    is_comment = true;
                }
                break;
            }
        }
        
        if (is_blank) blank++;
        else if (is_comment) comments++;
        else code++;
        
        if (!next) break;
        line = next + 1;
    }
    
    report->total_lines = total;
    report->code_lines = code;
    report->comment_lines = comments;
    report->blank_lines = blank;
    
    /* Calculate maintainability index (simplified) */
    if (total > 0) {
        report->maintainability_index = 100.0 * (1.0 - ((double)code / total) * 0.5);
    } else {
        report->maintainability_index = 100.0;
    }
    
    /* In a full implementation, we would:
     * 1. Parse the AST
     * 2. Find all function definitions
     * 3. Analyze each function
     * 4. Populate report->functions array
     */
    
    free(source);
    return report;
}

AnalysisReport* analyze_project(const char* project_path, AnalysisConfig* config) {
    if (!project_path) return NULL;
    
    AnalysisReport* report = analysis_report_create(project_path);
    if (!report) return NULL;
    
    /* Scan for source files */
    FileList* files = scan_directory(project_path, 
                                      (const char**)config->file_extensions,
                                      config->num_extensions);
    
    if (!files) {
        report->errors = (char**)malloc(sizeof(char*));
        if (report->errors) {
            report->errors[0] = ciopt_strdup("Failed to scan directory");
            report->num_errors = 1;
        }
        return report;
    }
    
    /* Analyze each file */
    report->files = (FileReport**)calloc(files->num_paths, sizeof(FileReport*));
    report->num_files = 0;
    
    for (int i = 0; i < files->num_paths; i++) {
        FileReport* fr = analyze_file(files->paths[i], config);
        if (fr) {
            report->files[report->num_files++] = fr;
            
            /* Update summary */
            report->summary.total_files++;
            report->summary.total_functions += fr->num_functions;
            report->summary.total_lines += fr->total_lines;
            report->summary.total_code_lines += fr->code_lines;
        }
    }
    
    /* Calculate averages */
    if (report->summary.total_functions > 0) {
        report->summary.avg_function_length = 
            (double)report->summary.total_code_lines / report->summary.total_functions;
    }
    
    file_list_free(files);
    return report;
}
