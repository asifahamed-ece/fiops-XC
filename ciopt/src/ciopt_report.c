/**
 * ciopt_report.c - Report generation for CiOpt
 * 
 * This implements terminal, HTML, and JSON report generation,
 * ported from the Python FiOpt project.
 */

#include "ciopt.h"
#include <ctype.h>

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static void escape_html(const char* src, char* dest, size_t dest_size) {
    if (!src || !dest || dest_size == 0) return;
    
    size_t j = 0;
    for (size_t i = 0; src[i] && j < dest_size - 1; i++) {
        switch (src[i]) {
            case '<':
                if (j + 4 < dest_size) {
                    memcpy(dest + j, "&lt;", 4);
                    j += 4;
                }
                break;
            case '>':
                if (j + 4 < dest_size) {
                    memcpy(dest + j, "&gt;", 4);
                    j += 4;
                }
                break;
            case '&':
                if (j + 5 < dest_size) {
                    memcpy(dest + j, "&amp;", 5);
                    j += 5;
                }
                break;
            case '"':
                if (j + 6 < dest_size) {
                    memcpy(dest + j, "&quot;", 6);
                    j += 6;
                }
                break;
            default:
                dest[j++] = src[i];
        }
    }
    dest[j] = '\0';
}

/* ============================================================================
 * Function Report Implementation
 * ============================================================================ */

FunctionReport* function_report_create(
    IRFunction* func,
    CFG* cfg,
    ComplexityResult* complexity,
    LoopAnalysis* loops,
    RecursionInfo* recursion,
    DataStructureAnalysis* ds_issues,
    DeadCodeAnalysis* dead_code,
    PatternAnalysis* patterns
) {
    FunctionReport* report = (FunctionReport*)calloc(1, sizeof(FunctionReport));
    if (!report) return NULL;
    
    report->function_name = ciopt_strdup(func && func->name ? func->name : "<unknown>");
    report->file_path = ciopt_strdup("");
    report->lineno = 0;
    report->end_lineno = 0;
    report->line_count = 0;
    report->complexity = complexity;  /* Takes ownership */
    report->loops = loops;            /* Takes ownership */
    report->recursion = recursion;    /* Takes ownership */
    report->data_structures = ds_issues;  /* Takes ownership */
    report->dead_code = dead_code;    /* Takes ownership */
    report->patterns = patterns;      /* Takes ownership */
    report->cfg = cfg;                /* Takes ownership */
    
    return report;
}

void function_report_free(FunctionReport* report) {
    if (!report) return;
    free(report->function_name);
    free(report->file_path);
    complexity_result_free(report->complexity);
    loop_analysis_free(report->loops);
    recursion_info_free(report->recursion);
    data_structure_analysis_free(report->data_structures);
    dead_code_analysis_free(report->dead_code);
    pattern_analysis_free(report->patterns);
    cfg_free(report->cfg);
    free(report);
}

/* ============================================================================
 * File Report Implementation
 * ============================================================================ */

FileReport* file_report_create(
    const char* file_path,
    FunctionReport** functions,
    int num_functions
) {
    FileReport* report = (FileReport*)calloc(1, sizeof(FileReport));
    if (!report) return NULL;
    
    report->file_path = ciopt_strdup(file_path ? file_path : "");
    report->file_size = 0;
    report->total_lines = 0;
    report->code_lines = 0;
    report->comment_lines = 0;
    report->blank_lines = 0;
    
    /* Copy function report pointers */
    if (functions && num_functions > 0) {
        report->functions = (FunctionReport**)calloc(num_functions, sizeof(FunctionReport*));
        if (report->functions) {
            memcpy(report->functions, functions, num_functions * sizeof(FunctionReport*));
            report->num_functions = num_functions;
        }
    } else {
        report->functions = NULL;
        report->num_functions = 0;
    }
    
    report->import_count = 0;
    report->imports = NULL;
    report->classes = NULL;
    report->num_classes = 0;
    report->maintainability_index = 0.0;
    
    return report;
}

void file_report_free(FileReport* report) {
    if (!report) return;
    free(report->file_path);
    
    for (int i = 0; i < report->num_functions; i++) {
        function_report_free(report->functions[i]);
    }
    free(report->functions);
    
    ciopt_free_string_array(report->imports, report->import_count);
    ciopt_free_string_array(report->classes, report->num_classes);
    
    free(report);
}

/* ============================================================================
 * Analysis Report Implementation
 * ============================================================================ */

AnalysisReport* analysis_report_create(const char* project_path) {
    AnalysisReport* report = (AnalysisReport*)calloc(1, sizeof(AnalysisReport));
    if (!report) return NULL;
    
    report->project_path = ciopt_strdup(project_path ? project_path : "");
    report->analysis_timestamp = time(NULL);
    report->fiopt_version = ciopt_strdup(CIOPT_VERSION);
    report->config = config_create_default();
    report->files = NULL;
    report->num_files = 0;
    report->summary.total_files = 0;
    report->summary.total_functions = 0;
    report->summary.total_lines = 0;
    report->summary.total_code_lines = 0;
    report->summary.avg_function_length = 0.0;
    report->summary.avg_complexity = 0.0;
    report->summary.max_complexity = 0;
    report->summary.critical_issues = 0;
    report->summary.warning_issues = 0;
    report->summary.info_issues = 0;
    report->summary.most_complex_function = NULL;
    report->summary.largest_file = NULL;
    report->errors = NULL;
    report->num_errors = 0;
    report->suggestions = NULL;
    report->num_suggestions = 0;
    
    return report;
}

void analysis_report_free(AnalysisReport* report) {
    if (!report) return;
    free(report->project_path);
    free(report->fiopt_version);
    config_free(report->config);
    
    for (int i = 0; i < report->num_files; i++) {
        file_report_free(report->files[i]);
    }
    free(report->files);
    
    free(report->summary.most_complex_function);
    free(report->summary.largest_file);
    
    ciopt_free_string_array(report->errors, report->num_errors);
    ciopt_free_string_array(report->suggestions, report->num_suggestions);
    
    free(report);
}

/* Add file report to analysis report */
void analysis_report_add_file(AnalysisReport* report, FileReport* file_report) {
    if (!report || !file_report) return;
    
    /* Resize files array */
    int new_size = report->num_files + 1;
    FileReport** new_files = (FileReport**)realloc(report->files, new_size * sizeof(FileReport*));
    if (!new_files) return;
    
    report->files = new_files;
    report->files[report->num_files] = file_report;
    report->num_files = new_size;
}

/* Finalize analysis report (compute summary) */
void analysis_report_finalize(AnalysisReport* report, AnalysisConfig* config) {
    if (!report) return;
    
    /* Compute summary statistics */
    report->summary.total_files = report->num_files;
    report->summary.total_functions = 0;
    report->summary.total_lines = 0;
    report->summary.total_code_lines = 0;
    report->summary.max_complexity = 0;
    report->summary.critical_issues = 0;
    report->summary.warning_issues = 0;
    report->summary.info_issues = 0;
    
    double total_complexity = 0.0;
    int complexity_count = 0;
    
    char* most_complex_func = NULL;
    int max_complexity_rank = -1;
    char* largest_file = NULL;
    size_t max_file_size = 0;
    
    for (int i = 0; i < report->num_files; i++) {
        FileReport* filerep = report->files[i];
        if (!filerep) continue;
        
        report->summary.total_lines += filerep->total_lines;
        report->summary.total_code_lines += filerep->code_lines;
        
        if (filerep->file_size > max_file_size) {
            max_file_size = filerep->file_size;
            largest_file = filerep->file_path;
        }
        
        for (int j = 0; j < filerep->num_functions; j++) {
            FunctionReport* funcrep = filerep->functions[j];
            if (!funcrep) continue;
            
            report->summary.total_functions++;
            
            if (funcrep->complexity) {
                int rank = (int)funcrep->complexity->estimated_complexity;
                total_complexity += rank;
                complexity_count++;
                
                if (rank > max_complexity_rank) {
                    max_complexity_rank = rank;
                    most_complex_func = funcrep->function_name;
                }
                
                if (rank > report->summary.max_complexity) {
                    report->summary.max_complexity = rank;
                }
            }
            
            /* Count issues by severity */
            if (funcrep->patterns) {
                for (int k = 0; k < funcrep->patterns->num_patterns; k++) {
                    AntiPattern* pattern = funcrep->patterns->anti_patterns[k];
                    if (pattern && pattern->severity == SEVERITY_CRITICAL) {
                        report->summary.critical_issues++;
                    } else if (pattern && pattern->severity == SEVERITY_WARNING) {
                        report->summary.warning_issues++;
                    } else {
                        report->summary.info_issues++;
                    }
                }
            }
            
            if (funcrep->data_structures) {
                report->summary.warning_issues += funcrep->data_structures->num_issues;
            }
        }
    }
    
    if (complexity_count > 0) {
        report->summary.avg_complexity = total_complexity / complexity_count;
    }
    
    if (report->summary.total_functions > 0) {
        report->summary.avg_function_length = 
            (double)report->summary.total_code_lines / report->summary.total_functions;
    }
    
    if (most_complex_func) {
        report->summary.most_complex_function = ciopt_strdup(most_complex_func);
    }
    
    if (largest_file) {
        report->summary.largest_file = ciopt_strdup(largest_file);
    }
    
    /* Generate suggestions from bottlenecks */
    /* This is a simplified version - full implementation would extract from all analyses */
    (void)config;  /* Unused for now */
}

/* ============================================================================
 * Terminal Report Generation
 * ============================================================================ */

char* generate_terminal_report(AnalysisReport* report) {
    if (!report) return NULL;
    
    size_t buf_size = 65536;
    char* buf = (char*)malloc(buf_size);
    if (!buf) return NULL;
    
    size_t offset = 0;
    
    /* Header */
    offset += snprintf(buf + offset, buf_size - offset,
        "\n================================================================================\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "  CiOpt - Code Complexity & Optimization Analysis Report\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "  Version: %s\n", report->fiopt_version);
    offset += snprintf(buf + offset, buf_size - offset,
        "================================================================================\n\n");
    
    offset += snprintf(buf + offset, buf_size - offset,
        "Project: %s\n", report->project_path);
    
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", 
             localtime(&report->analysis_timestamp));
    offset += snprintf(buf + offset, buf_size - offset,
        "Analyzed: %s\n\n", time_buf);
    
    /* Summary */
    offset += snprintf(buf + offset, buf_size - offset,
        "--------------------------------------------------------------------------------\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "SUMMARY\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "--------------------------------------------------------------------------------\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "  Files analyzed:      %d\n", report->summary.total_files);
    offset += snprintf(buf + offset, buf_size - offset,
        "  Total functions:     %d\n", report->summary.total_functions);
    offset += snprintf(buf + offset, buf_size - offset,
        "  Total lines:         %d\n", report->summary.total_lines);
    offset += snprintf(buf + offset, buf_size - offset,
        "  Code lines:          %d\n", report->summary.total_code_lines);
    offset += snprintf(buf + offset, buf_size - offset,
        "  Avg function length: %.1f lines\n", report->summary.avg_function_length);
    offset += snprintf(buf + offset, buf_size - offset,
        "  Avg complexity:      %.2f\n", report->summary.avg_complexity);
    offset += snprintf(buf + offset, buf_size - offset,
        "  Max complexity:      %d\n", report->summary.max_complexity);
    offset += snprintf(buf + offset, buf_size - offset,
        "\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "  Critical issues:     %d\n", report->summary.critical_issues);
    offset += snprintf(buf + offset, buf_size - offset,
        "  Warnings:            %d\n", report->summary.warning_issues);
    offset += snprintf(buf + offset, buf_size - offset,
        "  Info:                %d\n", report->summary.info_issues);
    
    if (report->summary.most_complex_function) {
        offset += snprintf(buf + offset, buf_size - offset,
            "\n  Most complex function: %s\n", report->summary.most_complex_function);
    }
    
    /* Per-file reports */
    for (int f = 0; f < report->num_files; f++) {
        FileReport* fr = report->files[f];
        
        offset += snprintf(buf + offset, buf_size - offset,
            "\n--------------------------------------------------------------------------------\n");
        offset += snprintf(buf + offset, buf_size - offset,
            "FILE: %s\n", fr->file_path);
        offset += snprintf(buf + offset, buf_size - offset,
            "--------------------------------------------------------------------------------\n");
        offset += snprintf(buf + offset, buf_size - offset,
            "  Lines: %d total, %d code, %d comments, %d blank\n",
            fr->total_lines, fr->code_lines, fr->comment_lines, fr->blank_lines);
        offset += snprintf(buf + offset, buf_size - offset,
            "  Functions: %d, Classes: %d, Imports: %d\n",
            fr->num_functions, fr->num_classes, fr->import_count);
        offset += snprintf(buf + offset, buf_size - offset,
            "  Maintainability Index: %.1f/100\n\n", fr->maintainability_index);
        
        /* Function details */
        for (int i = 0; i < fr->num_functions; i++) {
            FunctionReport* fn = fr->functions[i];
            
            offset += snprintf(buf + offset, buf_size - offset,
                "  Function: %s (lines %d-%d)\n",
                fn->function_name, fn->lineno, fn->end_lineno);
            
            if (fn->complexity) {
                offset += snprintf(buf + offset, buf_size - offset,
                    "    Complexity: %s (confidence: %.0f%%)\n",
                    complexity_to_string(fn->complexity->estimated_complexity),
                    fn->complexity->confidence * 100);
                
                if (fn->complexity->bottleneck_description) {
                    offset += snprintf(buf + offset, buf_size - offset,
                        "    Bottleneck: %s\n", fn->complexity->bottleneck_description);
                }
            }
            
            if (fn->loops && fn->loops->num_loops > 0) {
                offset += snprintf(buf + offset, buf_size - offset,
                    "    Loops: %d detected\n", fn->loops->num_loops);
            }
            
            if (fn->recursion && fn->recursion->is_recursive) {
                offset += snprintf(buf + offset, buf_size - offset,
                    "    Recursion: detected (%s)\n",
                    fn->recursion->depth_pattern ? fn->recursion->depth_pattern : "unknown");
            }
            
            if (fn->patterns && fn->patterns->num_patterns > 0) {
                int critical = 0, warning = 0;
                for (int p = 0; p < fn->patterns->num_patterns; p++) {
                    if (fn->patterns->anti_patterns[p]->severity == SEVERITY_CRITICAL) critical++;
                    else if (fn->patterns->anti_patterns[p]->severity == SEVERITY_WARNING) warning++;
                }
                if (critical > 0 || warning > 0) {
                    offset += snprintf(buf + offset, buf_size - offset,
                        "    Anti-patterns: %d critical, %d warnings\n", critical, warning);
                }
            }
            
            if (fn->dead_code && fn->dead_code->num_items > 0) {
                offset += snprintf(buf + offset, buf_size - offset,
                    "    Dead code: %d items\n", fn->dead_code->num_items);
            }
        }
    }
    
    /* Errors */
    if (report->num_errors > 0) {
        offset += snprintf(buf + offset, buf_size - offset,
            "\n--------------------------------------------------------------------------------\n");
        offset += snprintf(buf + offset, buf_size - offset,
            "ERRORS\n");
        offset += snprintf(buf + offset, buf_size - offset,
            "--------------------------------------------------------------------------------\n");
        for (int i = 0; i < report->num_errors; i++) {
            offset += snprintf(buf + offset, buf_size - offset,
                "  - %s\n", report->errors[i]);
        }
    }
    
    /* Suggestions */
    if (report->num_suggestions > 0) {
        offset += snprintf(buf + offset, buf_size - offset,
            "\n--------------------------------------------------------------------------------\n");
        offset += snprintf(buf + offset, buf_size - offset,
            "SUGGESTIONS\n");
        offset += snprintf(buf + offset, buf_size - offset,
            "--------------------------------------------------------------------------------\n");
        for (int i = 0; i < report->num_suggestions; i++) {
            offset += snprintf(buf + offset, buf_size - offset,
                "  - %s\n", report->suggestions[i]);
        }
    }
    
    offset += snprintf(buf + offset, buf_size - offset,
        "\n================================================================================\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "  End of Report\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "================================================================================\n");
    
    return buf;
}

/* ============================================================================
 * HTML Report Generation
 * ============================================================================ */

char* generate_html_report(AnalysisReport* report) {
    if (!report) return NULL;
    
    size_t buf_size = 131072;
    char* buf = (char*)malloc(buf_size);
    if (!buf) return NULL;
    
    size_t offset = 0;
    
    /* HTML Header */
    offset += snprintf(buf + offset, buf_size - offset,
        "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "  <meta charset=\"UTF-8\">\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "  <title>CiOpt Analysis Report</title>\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "  <style>\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    .container { max-width: 1200px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    h1 { color: #333; border-bottom: 2px solid #4CAF50; padding-bottom: 10px; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    h2 { color: #555; margin-top: 30px; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    .summary { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin: 20px 0; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    .stat-box { background: #f9f9f9; padding: 15px; border-radius: 4px; border-left: 4px solid #4CAF50; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    .stat-value { font-size: 24px; font-weight: bold; color: #333; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    .stat-label { color: #666; font-size: 14px; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    .critical { border-left-color: #f44336; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    .warning { border-left-color: #ff9800; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    table { width: 100%%; border-collapse: collapse; margin: 15px 0; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    th, td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    th { background: #4CAF50; color: white; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    tr:hover { background: #f5f5f5; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    .complexity-O_1 { color: #4CAF50; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    .complexity-O_N { color: #2196F3; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    .complexity-O_N_LOG_N { color: #FF9800; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    .complexity-O_N_SQUARED, .complexity-O_N_CUBED { color: #f44336; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    .complexity-O_2_N, .complexity-O_N_FACTORIAL { color: #9C27B0; font-weight: bold; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    pre { background: #f4f4f4; padding: 10px; border-radius: 4px; overflow-x: auto; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    code { font-family: 'Courier New', monospace; }\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "  </style>\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "</head>\n<body>\n<div class=\"container\">\n");
    
    /* Title */
    offset += snprintf(buf + offset, buf_size - offset,
        "  <h1>CiOpt Analysis Report</h1>\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "  <p><strong>Version:</strong> %s | <strong>Project:</strong> %s</p>\n",
        report->fiopt_version, report->project_path);
    
    /* Summary */
    offset += snprintf(buf + offset, buf_size - offset,
        "  <h2>Summary</h2>\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "  <div class=\"summary\">\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    <div class=\"stat-box\"><div class=\"stat-value\">%d</div><div class=\"stat-label\">Files</div></div>\n",
        report->summary.total_files);
    offset += snprintf(buf + offset, buf_size - offset,
        "    <div class=\"stat-box\"><div class=\"stat-value\">%d</div><div class=\"stat-label\">Functions</div></div>\n",
        report->summary.total_functions);
    offset += snprintf(buf + offset, buf_size - offset,
        "    <div class=\"stat-box\"><div class=\"stat-value\">%d</div><div class=\"stat-label\">Total Lines</div></div>\n",
        report->summary.total_lines);
    offset += snprintf(buf + offset, buf_size - offset,
        "    <div class=\"stat-box critical\"><div class=\"stat-value\">%d</div><div class=\"stat-label\">Critical Issues</div></div>\n",
        report->summary.critical_issues);
    offset += snprintf(buf + offset, buf_size - offset,
        "    <div class=\"stat-box warning\"><div class=\"stat-value\">%d</div><div class=\"stat-label\">Warnings</div></div>\n",
        report->summary.warning_issues);
    offset += snprintf(buf + offset, buf_size - offset,
        "  </div>\n");
    
    /* Files */
    for (int f = 0; f < report->num_files; f++) {
        FileReport* fr = report->files[f];
        char escaped_path[CIOPT_MAX_PATH_LEN];
        escape_html(fr->file_path, escaped_path, sizeof(escaped_path));
        
        offset += snprintf(buf + offset, buf_size - offset,
            "  <h2>File: %s</h2>\n", escaped_path);
        offset += snprintf(buf + offset, buf_size - offset,
            "  <table>\n");
        offset += snprintf(buf + offset, buf_size - offset,
            "    <tr><th>Metric</th><th>Value</th></tr>\n");
        offset += snprintf(buf + offset, buf_size - offset,
            "    <tr><td>Total Lines</td><td>%d</td></tr>\n", fr->total_lines);
        offset += snprintf(buf + offset, buf_size - offset,
            "    <tr><td>Code Lines</td><td>%d</td></tr>\n", fr->code_lines);
        offset += snprintf(buf + offset, buf_size - offset,
            "    <tr><td>Functions</td><td>%d</td></tr>\n", fr->num_functions);
        offset += snprintf(buf + offset, buf_size - offset,
            "    <tr><td>Maintainability</td><td>%.1f/100</td></tr>\n", fr->maintainability_index);
        offset += snprintf(buf + offset, buf_size - offset,
            "  </table>\n");
        
        /* Functions table */
        if (fr->num_functions > 0) {
            offset += snprintf(buf + offset, buf_size - offset,
                "  <h3>Functions</h3>\n");
            offset += snprintf(buf + offset, buf_size - offset,
                "  <table>\n");
            offset += snprintf(buf + offset, buf_size - offset,
                "    <tr><th>Name</th><th>Lines</th><th>Complexity</th><th>Issues</th></tr>\n");
            
            for (int i = 0; i < fr->num_functions; i++) {
                FunctionReport* fn = fr->functions[i];
                char escaped_name[CIOPT_MAX_NAME_LEN];
                escape_html(fn->function_name, escaped_name, sizeof(escaped_name));
                
                const char* css_class = "";
                if (fn->complexity) {
                    switch (fn->complexity->estimated_complexity) {
                        case O_1: css_class = "complexity-O_1"; break;
                        case O_N: css_class = "complexity-O_N"; break;
                        case O_N_LOG_N: css_class = "complexity-O_N_LOG_N"; break;
                        case O_N_SQUARED: case O_N_CUBED: css_class = "complexity-O_N_SQUARED"; break;
                        case O_2_N: case O_N_FACTORIAL: css_class = "complexity-O_2_N"; break;
                        default: break;
                    }
                }
                
                int issues = 0;
                if (fn->patterns) issues += fn->patterns->num_patterns;
                if (fn->dead_code) issues += fn->dead_code->num_items;
                
                offset += snprintf(buf + offset, buf_size - offset,
                    "    <tr><td>%s</td><td>%d-%d</td><td class=\"%s\">%s</td><td>%d</td></tr>\n",
                    escaped_name, fn->lineno, fn->end_lineno, css_class,
                    fn->complexity ? complexity_to_string(fn->complexity->estimated_complexity) : "N/A",
                    issues);
            }
            
            offset += snprintf(buf + offset, buf_size - offset,
                "  </table>\n");
        }
    }
    
    /* Footer */
    offset += snprintf(buf + offset, buf_size - offset,
        "</div>\n</body>\n</html>\n");
    
    return buf;
}

/* ============================================================================
 * JSON Report Generation
 * ============================================================================ */

char* generate_json_report(AnalysisReport* report) {
    if (!report) return NULL;
    
    size_t buf_size = 131072;
    char* buf = (char*)malloc(buf_size);
    if (!buf) return NULL;
    
    size_t offset = 0;
    
    offset += snprintf(buf + offset, buf_size - offset, "{\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "  \"version\": \"%s\",\n", report->fiopt_version);
    offset += snprintf(buf + offset, buf_size - offset,
        "  \"project\": \"%s\",\n", report->project_path);
    offset += snprintf(buf + offset, buf_size - offset,
        "  \"timestamp\": %ld,\n", (long)report->analysis_timestamp);
    
    /* Summary */
    offset += snprintf(buf + offset, buf_size - offset, "  \"summary\": {\n");
    offset += snprintf(buf + offset, buf_size - offset,
        "    \"total_files\": %d,\n", report->summary.total_files);
    offset += snprintf(buf + offset, buf_size - offset,
        "    \"total_functions\": %d,\n", report->summary.total_functions);
    offset += snprintf(buf + offset, buf_size - offset,
        "    \"total_lines\": %d,\n", report->summary.total_lines);
    offset += snprintf(buf + offset, buf_size - offset,
        "    \"total_code_lines\": %d,\n", report->summary.total_code_lines);
    offset += snprintf(buf + offset, buf_size - offset,
        "    \"avg_function_length\": %.2f,\n", report->summary.avg_function_length);
    offset += snprintf(buf + offset, buf_size - offset,
        "    \"avg_complexity\": %.2f,\n", report->summary.avg_complexity);
    offset += snprintf(buf + offset, buf_size - offset,
        "    \"max_complexity\": %d,\n", report->summary.max_complexity);
    offset += snprintf(buf + offset, buf_size - offset,
        "    \"critical_issues\": %d,\n", report->summary.critical_issues);
    offset += snprintf(buf + offset, buf_size - offset,
        "    \"warning_issues\": %d,\n", report->summary.warning_issues);
    offset += snprintf(buf + offset, buf_size - offset,
        "    \"info_issues\": %d\n", report->summary.info_issues);
    offset += snprintf(buf + offset, buf_size - offset, "  },\n");
    
    /* Files */
    offset += snprintf(buf + offset, buf_size - offset, "  \"files\": [\n");
    
    for (int f = 0; f < report->num_files; f++) {
        FileReport* fr = report->files[f];
        
        if (f > 0) offset += snprintf(buf + offset, buf_size - offset, ",\n");
        offset += snprintf(buf + offset, buf_size - offset, "    {\n");
        offset += snprintf(buf + offset, buf_size - offset,
            "      \"path\": \"%s\",\n", fr->file_path);
        offset += snprintf(buf + offset, buf_size - offset,
            "      \"total_lines\": %d,\n", fr->total_lines);
        offset += snprintf(buf + offset, buf_size - offset,
            "      \"code_lines\": %d,\n", fr->code_lines);
        offset += snprintf(buf + offset, buf_size - offset,
            "      \"functions\": %d,\n", fr->num_functions);
        offset += snprintf(buf + offset, buf_size - offset,
            "      \"maintainability_index\": %.2f,\n", fr->maintainability_index);
        
        /* Functions in this file */
        offset += snprintf(buf + offset, buf_size - offset, "      \"function_details\": [\n");
        
        for (int i = 0; i < fr->num_functions; i++) {
            FunctionReport* fn = fr->functions[i];
            
            if (i > 0) offset += snprintf(buf + offset, buf_size - offset, ",\n");
            offset += snprintf(buf + offset, buf_size - offset, "        {\n");
            offset += snprintf(buf + offset, buf_size - offset,
                "          \"name\": \"%s\",\n", fn->function_name);
            offset += snprintf(buf + offset, buf_size - offset,
                "          \"lineno\": %d,\n", fn->lineno);
            offset += snprintf(buf + offset, buf_size - offset,
                "          \"end_lineno\": %d,\n", fn->end_lineno);
            
            if (fn->complexity) {
                offset += snprintf(buf + offset, buf_size - offset,
                    "          \"complexity\": \"%s\",\n",
                    complexity_to_string(fn->complexity->estimated_complexity));
                offset += snprintf(buf + offset, buf_size - offset,
                    "          \"confidence\": %.2f,\n", fn->complexity->confidence);
            }
            
            offset += snprintf(buf + offset, buf_size - offset,
                "          \"has_loops\": %s,\n",
                (fn->loops && fn->loops->num_loops > 0) ? "true" : "false");
            offset += snprintf(buf + offset, buf_size - offset,
                "          \"is_recursive\": %s\n",
                (fn->recursion && fn->recursion->is_recursive) ? "true" : "false");
            
            offset += snprintf(buf + offset, buf_size - offset, "        }");
        }
        
        offset += snprintf(buf + offset, buf_size - offset, "\n      ]\n");
        offset += snprintf(buf + offset, buf_size - offset, "    }");
    }
    
    offset += snprintf(buf + offset, buf_size - offset, "\n  ]\n");
    offset += snprintf(buf + offset, buf_size - offset, "}\n");
    
    return buf;
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

int write_report_to_file(const char* content, const char* path) {
    if (!content || !path) return -1;
    
    FILE* f = fopen(path, "w");
    if (!f) return -1;
    
    size_t written = fwrite(content, 1, strlen(content), f);
    fclose(f);
    
    return (written == strlen(content)) ? 0 : -1;
}

void print_summary(AnalysisReport* report) {
    if (!report) return;
    
    printf("\n=== CiOpt Analysis Summary ===\n");
    printf("Files: %d | Functions: %d | Lines: %d\n",
           report->summary.total_files,
           report->summary.total_functions,
           report->summary.total_lines);
    printf("Issues: %d critical, %d warnings, %d info\n",
           report->summary.critical_issues,
           report->summary.warning_issues,
           report->summary.info_issues);
    printf("==============================\n");
}
