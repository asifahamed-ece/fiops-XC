/**
 * ciopt — C Implementation of FiOpt
 * Report data structures and analysis aggregation
 */

#include "ciopt.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * Function Info
 * ============================================================================ */

FunctionInfo* function_info_create(void) {
    FunctionInfo* info = (FunctionInfo*)calloc(1, sizeof(FunctionInfo));
    if (!info) return NULL;
    
    info->name = NULL;
    info->lineno = 0;
    info->end_lineno = 0;
    info->col_offset = 0;
    info->args = NULL;
    info->num_args = 0;
    info->decorators = NULL;
    info->num_decorators = 0;
    info->docstring = NULL;
    info->is_method = false;
    info->is_async = false;
    info->is_generator = false;
    info->is_property = false;
    info->body_line_count = 0;
    info->nested_functions = NULL;
    info->num_nested_functions = 0;
    
    return info;
}

void function_info_free(FunctionInfo* info) {
    if (!info) return;
    
    free(info->name);
    str_array_free(info->args, info->num_args);
    str_array_free(info->decorators, info->num_decorators);
    free(info->docstring);
    str_array_free(info->nested_functions, info->num_nested_functions);
    free(info);
}

/* ============================================================================
 * Parsed Module
 * ============================================================================ */

ParsedModule* parsed_module_create(void) {
    ParsedModule* module = (ParsedModule*)calloc(1, sizeof(ParsedModule));
    if (!module) return NULL;
    
    module->source = NULL;
    module->source_len = 0;
    module->functions = NULL;
    module->num_functions = 0;
    module->top_level_statements = 0;
    module->num_imports = 0;
    module->global_variables = NULL;
    module->num_global_variables = 0;
    
    return module;
}

void parsed_module_free(ParsedModule* module) {
    if (!module) return;
    
    free(module->source);
    for (int i = 0; i < module->num_functions; i++) {
        function_info_free(module->functions[i]);
    }
    free(module->functions);
    str_array_free(module->global_variables, module->num_global_variables);
    free(module);
}

/* ============================================================================
 * Source File
 * ============================================================================ */

SourceFile* source_file_load(const char* path) {
    if (!path) return NULL;
    
    FILE* f = fopen(path, "r");
    if (!f) return NULL;
    
    /* Get file size */
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    /* Allocate buffer */
    char* content = (char*)malloc(size + 1);
    if (!content) {
        fclose(f);
        return NULL;
    }
    
    /* Read content */
    size_t read_size = fread(content, 1, size, f);
    content[read_size] = '\0';
    fclose(f);
    
    /* Count lines */
    int line_count = 0;
    for (const char* p = content; *p; p++) {
        if (*p == '\n') line_count++;
    }
    if (read_size > 0 && content[read_size - 1] != '\n') {
        line_count++;
    }
    
    /* Create SourceFile */
    SourceFile* file = (SourceFile*)calloc(1, sizeof(SourceFile));
    if (!file) {
        free(content);
        return NULL;
    }
    
    file->path = str_duplicate(path);
    file->content = content;
    file->line_count = line_count;
    file->encoding = str_duplicate("utf-8");
    
    return file;
}

void source_file_free(SourceFile* file) {
    if (!file) return;
    
    free(file->path);
    free(file->content);
    free(file->encoding);
    free(file);
}

char* source_file_get_line(SourceFile* file, int lineno) {
    if (!file || !file->content || lineno < 1) return NULL;
    
    const char* p = file->content;
    int current_line = 1;
    
    while (*p && current_line < lineno) {
        if (*p == '\n') current_line++;
        p++;
    }
    
    if (current_line != lineno) return NULL;
    
    /* Find end of line */
    const char* start = p;
    while (*p && *p != '\n') p++;
    
    size_t len = p - start;
    char* line = (char*)malloc(len + 1);
    if (line) {
        memcpy(line, start, len);
        line[len] = '\0';
    }
    
    return line;
}

/* ============================================================================
 * Loop Analysis
 * ============================================================================ */

int loop_analysis_max_depth(LoopAnalysis* analysis) {
    if (!analysis || analysis->num_loops == 0) return 0;
    
    int max_depth = 0;
    for (int i = 0; i < analysis->num_loops; i++) {
        if (analysis->loops[i].depth > max_depth) {
            max_depth = analysis->loops[i].depth;
        }
    }
    return max_depth;
}

int loop_analysis_total_loops(LoopAnalysis* analysis) {
    return analysis ? analysis->num_loops : 0;
}

void loop_analysis_free(LoopAnalysis* analysis) {
    if (!analysis) return;
    
    for (int i = 0; i < analysis->num_loops; i++) {
        LoopDetail* loop = &analysis->loops[i];
        for (int j = 0; j < loop->num_variables; j++) {
            free(loop->variables[j].name);
            free(loop->variables[j].iterable);
        }
        free(loop->variables);
        free(loop->invariant_lines);
        str_array_free(loop->expensive_operations, loop->num_expensive_operations);
        str_array_free(loop->function_calls, loop->num_function_calls);
    }
    free(analysis->loops);
    free(analysis->function_name);
    free(analysis);
}

/* ============================================================================
 * Recursion Info
 * ============================================================================ */

void recursion_info_free(RecursionInfo* info) {
    if (!info) return;
    
    free(info->function_name);
    free(info->direct_calls);
    free(info->base_case_lines);
    free(info->tail_recursive_lines);
    free(info->memoization_reason);
    free(info->depth_pattern);
    free(info);
}

/* ============================================================================
 * Complexity Result
 * ============================================================================ */

bool complexity_is_efficient(ComplexityResult* result) {
    if (!result) return false;
    return result->estimated_complexity <= O_N_LOG_N;
}

char* complexity_summary(ComplexityResult* result) {
    if (!result) return NULL;
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s: %s (confidence: %.0f%%)",
        result->function_name,
        complexity_to_string(result->estimated_complexity),
        result->confidence * 100.0f);
    
    return str_duplicate(buffer);
}

void complexity_result_free(ComplexityResult* result) {
    if (!result) return;
    
    free(result->function_name);
    for (int i = 0; i < result->num_explanations; i++) {
        free(result->explanations[i].source);
        free(result->explanations[i].description);
        free(result->explanations[i].detail);
    }
    free(result->explanations);
    loop_analysis_free(result->loop_analysis);
    recursion_info_free(result->recursion_info);
    free(result->bottleneck_lines);
    free(result->bottleneck_description);
    str_array_free(result->warnings, result->num_warnings);
    free(result);
}

/* ============================================================================
 * Pattern Analysis
 * ============================================================================ */

int pattern_analysis_critical_count(PatternAnalysis* analysis) {
    if (!analysis) return 0;
    
    int count = 0;
    for (int i = 0; i < analysis->num_anti_patterns; i++) {
        if (analysis->anti_patterns[i].severity == PATTERN_SEVERITY_CRITICAL) {
            count++;
        }
    }
    return count;
}

int pattern_analysis_warning_count(PatternAnalysis* analysis) {
    if (!analysis) return 0;
    
    int count = 0;
    for (int i = 0; i < analysis->num_anti_patterns; i++) {
        if (analysis->anti_patterns[i].severity == PATTERN_SEVERITY_WARNING) {
            count++;
        }
    }
    return count;
}

void pattern_analysis_free(PatternAnalysis* analysis) {
    if (!analysis) return;
    
    for (int i = 0; i < analysis->num_anti_patterns; i++) {
        AntiPattern* p = &analysis->anti_patterns[i];
        free(p->name);
        free(p->description);
        free(p->suggestion);
        free(p->estimated_impact);
        free(p->code_snippet);
    }
    free(analysis->anti_patterns);
    free(analysis->function_name);
    free(analysis);
}

/* ============================================================================
 * Data Structure Analysis
 * ============================================================================ */

void data_structure_analysis_free(DataStructureAnalysis* analysis) {
    if (!analysis) return;
    
    for (int i = 0; i < analysis->num_issues; i++) {
        DataStructureIssue* issue = &analysis->issues[i];
        free(issue->variable_name);
        free(issue->current_type);
        free(issue->suggested_type);
        free(issue->description);
        free(issue->suggestion);
        free(issue->estimated_impact);
    }
    free(analysis->issues);
    free(analysis->function_name);
    free(analysis);
}

/* ============================================================================
 * Dead Code Analysis
 * ============================================================================ */

int dead_code_total(DeadCodeAnalysis* analysis) {
    return analysis ? analysis->num_items : 0;
}

int dead_code_unreachable_count(DeadCodeAnalysis* analysis) {
    if (!analysis) return 0;
    
    int count = 0;
    for (int i = 0; i < analysis->num_items; i++) {
        if (strcmp(analysis->items[i].kind, "unreachable") == 0) {
            count++;
        }
    }
    return count;
}

void dead_code_analysis_free(DeadCodeAnalysis* analysis) {
    if (!analysis) return;
    
    for (int i = 0; i < analysis->num_items; i++) {
        DeadCodeItem* item = &analysis->items[i];
        free(item->kind);
        free(item->name);
        free(item->description);
        free(item->suggestion);
    }
    free(analysis->items);
    free(analysis);
}

/* ============================================================================
 * Function Report
 * ============================================================================ */

int function_report_total_issues(FunctionReport* report) {
    if (!report) return 0;
    
    int count = 0;
    if (report->patterns) {
        count += report->patterns->num_anti_patterns;
    }
    if (report->data_structure) {
        count += report->data_structure->num_issues;
    }
    if (report->complexity) {
        count += report->complexity->num_warnings;
    }
    return count;
}

char* function_report_summary(FunctionReport* report) {
    if (!report) return NULL;
    
    char buffer[512];
    const char* complexity_str = report->complexity
        ? complexity_to_string(report->complexity->estimated_complexity)
        : "Unknown";
    
    int issues = function_report_total_issues(report);
    
    snprintf(buffer, sizeof(buffer),
        "%s (L%d-%d): %s | %d issue(s)",
        report->name, report->lineno, report->end_lineno,
        complexity_str, issues);
    
    return str_duplicate(buffer);
}

void function_report_free(FunctionReport* report) {
    if (!report) return;
    
    free(report->name);
    complexity_result_free(report->complexity);
    pattern_analysis_free(report->patterns);
    data_structure_analysis_free(report->data_structure);
    free(report);
}

/* ============================================================================
 * File Report
 * ============================================================================ */

ComplexityClass file_report_worst_complexity(FileReport* report) {
    if (!report || report->num_function_reports == 0) return O_1;
    
    ComplexityClass worst = O_1;
    for (int i = 0; i < report->num_function_reports; i++) {
        FunctionReport* func = report->function_reports[i];
        if (func->complexity) {
            if (complexity_compare(func->complexity->estimated_complexity, worst) > 0) {
                worst = func->complexity->estimated_complexity;
            }
        }
    }
    return worst;
}

int file_report_total_functions(FileReport* report) {
    return report ? report->num_function_reports : 0;
}

int file_report_total_issues(FileReport* report) {
    if (!report) return 0;
    
    int count = 0;
    for (int i = 0; i < report->num_function_reports; i++) {
        count += function_report_total_issues(report->function_reports[i]);
    }
    if (report->dead_code) {
        count += dead_code_total(report->dead_code);
    }
    return count;
}

FunctionReport** file_report_critical_functions(FileReport* report, int* count) {
    *count = 0;
    if (!report) return NULL;
    
    /* First pass: count critical functions */
    int critical_count = 0;
    for (int i = 0; i < report->num_function_reports; i++) {
        if (report->function_reports[i]->severity == SEVERITY_CRITICAL) {
            critical_count++;
        }
    }
    
    if (critical_count == 0) return NULL;
    
    /* Second pass: collect critical functions */
    FunctionReport** critical = (FunctionReport**)malloc(
        critical_count * sizeof(FunctionReport*)
    );
    if (!critical) return NULL;
    
    int idx = 0;
    for (int i = 0; i < report->num_function_reports; i++) {
        if (report->function_reports[i]->severity == SEVERITY_CRITICAL) {
            critical[idx++] = report->function_reports[i];
        }
    }
    
    *count = critical_count;
    return critical;
}

void file_report_free(FileReport* report) {
    if (!report) return;
    
    free(report->filepath);
    for (int i = 0; i < report->num_function_reports; i++) {
        function_report_free(report->function_reports[i]);
    }
    free(report->function_reports);
    dead_code_analysis_free(report->dead_code);
    str_array_free(report->parse_errors, report->num_parse_errors);
    free(report);
}

/* ============================================================================
 * Analysis Report
 * ============================================================================ */

int analysis_report_total_files(AnalysisReport* report) {
    return report ? report->num_files : 0;
}

int analysis_report_total_functions(AnalysisReport* report) {
    if (!report) return 0;
    
    int total = 0;
    for (int i = 0; i < report->num_files; i++) {
        total += file_report_total_functions(report->files[i]);
    }
    return total;
}

int analysis_report_total_lines(AnalysisReport* report) {
    if (!report) return 0;
    
    int total = 0;
    for (int i = 0; i < report->num_files; i++) {
        total += report->files[i]->line_count;
    }
    return total;
}

int analysis_report_total_issues(AnalysisReport* report) {
    if (!report) return 0;
    
    int total = 0;
    for (int i = 0; i < report->num_files; i++) {
        total += file_report_total_issues(report->files[i]);
    }
    return total;
}

ComplexityClass analysis_report_worst_complexity(AnalysisReport* report) {
    if (!report || report->num_files == 0) return O_1;
    
    ComplexityClass worst = O_1;
    for (int i = 0; i < report->num_files; i++) {
        ComplexityClass file_worst = file_report_worst_complexity(report->files[i]);
        if (complexity_compare(file_worst, worst) > 0) {
            worst = file_worst;
        }
    }
    return worst;
}

const char* analysis_report_complexity(AnalysisReport* report) {
    static const char* unknown = "Unknown";
    if (!report) return unknown;
    return complexity_to_string(analysis_report_worst_complexity(report));
}

char** analysis_report_bottlenecks(AnalysisReport* report, int* count) {
    *count = 0;
    if (!report) return NULL;
    
    /* Estimate maximum bottlenecks */
    int max_bottlenecks = report->num_files * 10;
    char** bottlenecks = (char**)malloc(max_bottlenecks * sizeof(char*));
    if (!bottlenecks) return NULL;
    
    int idx = 0;
    for (int i = 0; i < report->num_files; i++) {
        FileReport* file = report->files[i];
        for (int j = 0; j < file->num_function_reports; j++) {
            FunctionReport* func = file->function_reports[j];
            if (func->complexity && 
                func->complexity->estimated_complexity >= O_N_SQUARED) {
                
                char buffer[512];
                const char* filepath = file->filepath ? file->filepath : "unknown";
                const char* name = func->name ? func->name : "unknown";
                const char* complexity = complexity_to_string(
                    func->complexity->estimated_complexity);
                
                if (func->complexity->bottleneck_description) {
                    snprintf(buffer, sizeof(buffer),
                        "%s:%s (L%d) — %s: %s",
                        filepath, name, func->lineno, complexity,
                        func->complexity->bottleneck_description);
                } else {
                    snprintf(buffer, sizeof(buffer),
                        "%s:%s (L%d) — %s",
                        filepath, name, func->lineno, complexity);
                }
                
                bottlenecks[idx++] = str_duplicate(buffer);
                if (idx >= max_bottlenecks - 1) break;
            }
        }
        if (idx >= max_bottlenecks - 1) break;
    }
    
    *count = idx;
    return bottlenecks;
}

char** analysis_report_suggestions(AnalysisReport* report, int* count) {
    *count = 0;
    if (!report) return NULL;
    
    /* Estimate maximum suggestions */
    int max_suggestions = report->num_files * 20;
    char** suggestions = (char**)malloc(max_suggestions * sizeof(char*));
    if (!suggestions) return NULL;
    
    int idx = 0;
    for (int i = 0; i < report->num_files; i++) {
        FileReport* file = report->files[i];
        for (int j = 0; j < file->num_function_reports; j++) {
            FunctionReport* func = file->function_reports[j];
            const char* filepath = file->filepath ? file->filepath : "unknown";
            const char* name = func->name ? func->name : "unknown";
            
            /* From complexity warnings */
            if (func->complexity) {
                for (int k = 0; k < func->complexity->num_warnings; k++) {
                    suggestions[idx++] = str_duplicate(func->complexity->warnings[k]);
                    if (idx >= max_suggestions - 1) break;
                }
            }
            
            /* From anti-patterns */
            if (func->patterns) {
                for (int k = 0; k < func->patterns->num_anti_patterns; k++) {
                    AntiPattern* p = &func->patterns->anti_patterns[k];
                    char buffer[512];
                    snprintf(buffer, sizeof(buffer),
                        "%s:%s (L%d): %s",
                        filepath, name, p->lineno, p->suggestion);
                    suggestions[idx++] = str_duplicate(buffer);
                    if (idx >= max_suggestions - 1) break;
                }
            }
            
            /* From data structure issues */
            if (func->data_structure) {
                for (int k = 0; k < func->data_structure->num_issues; k++) {
                    DataStructureIssue* issue = &func->data_structure->issues[k];
                    char buffer[512];
                    snprintf(buffer, sizeof(buffer),
                        "%s:%s (L%d): %s",
                        filepath, name, issue->lineno, issue->suggestion);
                    suggestions[idx++] = str_duplicate(buffer);
                    if (idx >= max_suggestions - 1) break;
                }
            }
            
            if (idx >= max_suggestions - 1) break;
        }
        if (idx >= max_suggestions - 1) break;
    }
    
    *count = idx;
    return suggestions;
}

char* analysis_report_summary(AnalysisReport* report) {
    if (!report) return NULL;
    
    size_t buf_size = 4096;
    char* buffer = (char*)malloc(buf_size);
    if (!buffer) return NULL;
    
    size_t offset = 0;
    offset += snprintf(buffer + offset, buf_size - offset,
        "FiOpt Analysis Report\n");
    offset += snprintf(buffer + offset, buf_size - offset,
        "==================================================\n");
    offset += snprintf(buffer + offset, buf_size - offset,
        "Files analyzed: %d\n", analysis_report_total_files(report));
    offset += snprintf(buffer + offset, buf_size - offset,
        "Functions analyzed: %d\n", analysis_report_total_functions(report));
    offset += snprintf(buffer + offset, buf_size - offset,
        "Total lines: %d\n", analysis_report_total_lines(report));
    offset += snprintf(buffer + offset, buf_size - offset,
        "Worst complexity: %s\n", analysis_report_complexity(report));
    offset += snprintf(buffer + offset, buf_size - offset,
        "Total issues: %d\n", analysis_report_total_issues(report));
    
    /* Add bottlenecks (up to 5) */
    int bottleneck_count = 0;
    char** bottlenecks = analysis_report_bottlenecks(report, &bottleneck_count);
    if (bottlenecks && bottleneck_count > 0) {
        offset += snprintf(buffer + offset, buf_size - offset,
            "\nBottlenecks:\n");
        int limit = bottleneck_count < 5 ? bottleneck_count : 5;
        for (int i = 0; i < limit; i++) {
            offset += snprintf(buffer + offset, buf_size - offset,
                "  • %s\n", bottlenecks[i]);
        }
        str_array_free(bottlenecks, bottleneck_count);
    }
    
    /* Add suggestions (up to 5) */
    int suggestion_count = 0;
    char** suggestions = analysis_report_suggestions(report, &suggestion_count);
    if (suggestions && suggestion_count > 0) {
        offset += snprintf(buffer + offset, buf_size - offset,
            "\nSuggestions:\n");
        int limit = suggestion_count < 5 ? suggestion_count : 5;
        for (int i = 0; i < limit; i++) {
            offset += snprintf(buffer + offset, buf_size - offset,
                "  • %s\n", suggestions[i]);
        }
        str_array_free(suggestions, suggestion_count);
    }
    
    return buffer;
}

void analysis_report_free(AnalysisReport* report) {
    if (!report) return;
    
    for (int i = 0; i < report->num_files; i++) {
        file_report_free(report->files[i]);
    }
    free(report->files);
    free(report->timestamp);
    free(report->fiopt_version);
    free(report);
}
