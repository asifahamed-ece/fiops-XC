/**
 * ciopt — C Implementation of FiOpt
 * Analyzer implementations and main analysis functions
 */

#include "ciopt.h"
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>

/* ============================================================================
 * Stub Analysis Functions
 * Note: Full implementation would require a Python AST parser in C,
 * which is beyond the scope of this port. These are placeholder implementations
 * that demonstrate the API structure.
 * ============================================================================ */

ParsedModule* parse_source(const char* source, const char* filename) {
    /* TODO: Implement Python AST parsing in C using libpython or custom parser */
    (void)source;
    (void)filename;
    
    ParsedModule* module = parsed_module_create();
    if (!module) return NULL;
    
    /* For now, just create a stub module */
    module->source = str_duplicate(source);
    module->source_len = source ? strlen(source) : 0;
    
    return module;
}

SourceFile** scan_project(const char* directory, int* count) {
    *count = 0;
    if (!directory) return NULL;
    
    DIR* dir = opendir(directory);
    if (!dir) return NULL;
    
    SourceFile** files = NULL;
    int file_count = 0;
    int capacity = 16;
    files = (SourceFile**)malloc(capacity * sizeof(SourceFile*));
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        /* Check for .py extension */
        const char* name = entry->d_name;
        size_t len = strlen(name);
        if (len < 3 || strcmp(name + len - 3, ".py") != 0) continue;
        
        /* Build full path */
        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", directory, name);
        
        /* Load file */
        SourceFile* file = source_file_load(path);
        if (file) {
            if (file_count >= capacity) {
                capacity *= 2;
                files = (SourceFile**)realloc(files, capacity * sizeof(SourceFile*));
            }
            files[file_count++] = file;
        }
    }
    
    closedir(dir);
    *count = file_count;
    return files;
}

LoopAnalysis* detect_loops(const char* source, const char* func_name) {
    (void)source;
    (void)func_name;
    
    LoopAnalysis* analysis = (LoopAnalysis*)calloc(1, sizeof(LoopAnalysis));
    if (!analysis) return NULL;
    
    analysis->function_name = str_duplicate(func_name);
    analysis->loops = NULL;
    analysis->num_loops = 0;
    
    /* TODO: Parse source and detect loops */
    
    return analysis;
}

RecursionInfo* detect_recursion(const char* source, const char* func_name) {
    (void)source;
    (void)func_name;
    
    RecursionInfo* info = (RecursionInfo*)calloc(1, sizeof(RecursionInfo));
    if (!info) return NULL;
    
    info->function_name = str_duplicate(func_name);
    info->is_recursive = false;
    info->has_base_case = false;
    info->can_be_memoized = false;
    
    /* TODO: Parse source and detect recursion */
    
    return info;
}

ComplexityResult* estimate_complexity(const char* source, const char* func_name) {
    (void)source;
    (void)func_name;
    
    ComplexityResult* result = (ComplexityResult*)calloc(1, sizeof(ComplexityResult));
    if (!result) return NULL;
    
    result->function_name = str_duplicate(func_name);
    result->estimated_complexity = O_UNKNOWN;
    result->confidence = 0.5f;
    
    /* TODO: Analyze source for complexity estimation */
    
    return result;
}

PatternAnalysis* detect_patterns(const char* source, const char* func_name) {
    (void)source;
    (void)func_name;
    
    PatternAnalysis* analysis = (PatternAnalysis*)calloc(1, sizeof(PatternAnalysis));
    if (!analysis) return NULL;
    
    analysis->function_name = str_duplicate(func_name);
    analysis->anti_patterns = NULL;
    analysis->num_anti_patterns = 0;
    
    /* TODO: Detect anti-patterns in source */
    
    return analysis;
}

DataStructureAnalysis* detect_data_structure_issues(const char* source, const char* func_name) {
    (void)source;
    (void)func_name;
    
    DataStructureAnalysis* analysis = (DataStructureAnalysis*)calloc(1, sizeof(DataStructureAnalysis));
    if (!analysis) return NULL;
    
    analysis->function_name = str_duplicate(func_name);
    analysis->issues = NULL;
    analysis->num_issues = 0;
    
    /* TODO: Detect data structure issues */
    
    return analysis;
}

DeadCodeAnalysis* detect_dead_code(const char* source) {
    (void)source;
    
    DeadCodeAnalysis* analysis = (DeadCodeAnalysis*)calloc(1, sizeof(DeadCodeAnalysis));
    if (!analysis) return NULL;
    
    analysis->items = NULL;
    analysis->num_items = 0;
    
    /* TODO: Detect dead code */
    
    return analysis;
}

/* ============================================================================
 * Main Analysis Functions
 * ============================================================================ */

static FunctionReport* analyze_function_stub(const char* name, int lineno, int end_lineno) {
    FunctionReport* report = (FunctionReport*)calloc(1, sizeof(FunctionReport));
    if (!report) return NULL;
    
    report->name = str_duplicate(name);
    report->lineno = lineno;
    report->end_lineno = end_lineno;
    report->line_count = end_lineno - lineno + 1;
    
    /* Create stub complexity result */
    report->complexity = (ComplexityResult*)calloc(1, sizeof(ComplexityResult));
    if (report->complexity) {
        report->complexity->function_name = str_duplicate(name);
        report->complexity->lineno = lineno;
        report->complexity->end_lineno = end_lineno;
        report->complexity->estimated_complexity = O_N;
        report->complexity->confidence = 0.5f;
    }
    
    report->severity = SEVERITY_INFO;
    
    return report;
}

AnalysisReport* analyze(const char* path, AnalysisConfig* config) {
    if (!path || !config) return NULL;
    
    AnalysisReport* report = (AnalysisReport*)calloc(1, sizeof(AnalysisReport));
    if (!report) return NULL;
    
    report->timestamp = get_timestamp_iso();
    report->fiopt_version = str_duplicate(CIOPT_VERSION);
    report->files = NULL;
    report->num_files = 0;
    
    /* Check if path is a file or directory */
    struct stat st;
    if (stat(path, &st) != 0) {
        analysis_report_free(report);
        return NULL;
    }
    
    SourceFile** files = NULL;
    int file_count = 0;
    
    if (S_ISDIR(st.st_mode)) {
        /* Scan directory */
        files = scan_project(path, &file_count);
    } else {
        /* Single file */
        SourceFile* file = source_file_load(path);
        if (file) {
            file_count = 1;
            files = (SourceFile**)malloc(sizeof(SourceFile*));
            files[0] = file;
        }
    }
    
    if (!files || file_count == 0) {
        analysis_report_free(report);
        return NULL;
    }
    
    /* Allocate file reports */
    report->num_files = file_count;
    report->files = (FileReport**)malloc(file_count * sizeof(FileReport*));
    
    for (int i = 0; i < file_count; i++) {
        SourceFile* src = files[i];
        
        FileReport* file_report = (FileReport*)calloc(1, sizeof(FileReport));
        if (!file_report) continue;
        
        file_report->filepath = str_duplicate(src->path);
        file_report->line_count = src->line_count;
        file_report->function_reports = NULL;
        file_report->num_function_reports = 0;
        
        /* Create stub function reports based on line count */
        /* In a real implementation, we would parse the source to find actual functions */
        int num_stub_functions = src->line_count / 10 + 1;
        if (num_stub_functions > 20) num_stub_functions = 20;
        
        file_report->num_function_reports = num_stub_functions;
        file_report->function_reports = (FunctionReport**)malloc(
            num_stub_functions * sizeof(FunctionReport*)
        );
        
        for (int j = 0; j < num_stub_functions; j++) {
            char func_name[64];
            snprintf(func_name, sizeof(func_name), "function_%d", j + 1);
            
            int start_line = j * 10 + 1;
            int end_line = start_line + 9;
            if (end_line > src->line_count) end_line = src->line_count;
            
            file_report->function_reports[j] = analyze_function_stub(
                func_name, start_line, end_line
            );
        }
        
        /* Run dead code detection if enabled */
        if (config->detect_dead_code) {
            file_report->dead_code = detect_dead_code(src->content);
        }
        
        report->files[i] = file_report;
    }
    
    /* Clean up source files */
    for (int i = 0; i < file_count; i++) {
        source_file_free(files[i]);
    }
    free(files);
    
    return report;
}

AnalysisReport* analyze_source(const char* source, const char* filename, AnalysisConfig* config) {
    (void)filename;
    (void)config;
    
    /* Write source to temp file and analyze */
    FILE* f = fopen("/tmp/ciopt_temp.py", "w");
    if (!f) return NULL;
    
    fputs(source, f);
    fclose(f);
    
    AnalysisReport* report = analyze("/tmp/ciopt_temp.py", config);
    
    remove("/tmp/ciopt_temp.py");
    
    return report;
}

/* ============================================================================
 * Reporting Functions
 * ============================================================================ */

void render_terminal(AnalysisReport* report, bool verbose) {
    if (!report) return;
    
    char* summary = analysis_report_summary(report);
    if (summary) {
        printf("%s\n", summary);
        free(summary);
    }
    
    if (verbose) {
        printf("\nDetailed Report:\n");
        printf("==================================================\n");
        
        for (int i = 0; i < report->num_files; i++) {
            FileReport* file = report->files[i];
            printf("\nFile: %s (%d lines)\n", 
                   file->filepath ? file->filepath : "unknown",
                   file->line_count);
            
            for (int j = 0; j < file->num_function_reports; j++) {
                FunctionReport* func = file->function_reports[j];
                char* func_summary = function_report_summary(func);
                if (func_summary) {
                    printf("  %s\n", func_summary);
                    free(func_summary);
                }
                
                if (func->complexity && func->complexity->num_explanations > 0) {
                    printf("    Complexity: %s (confidence: %.0f%%)\n",
                           complexity_to_string(func->complexity->estimated_complexity),
                           func->complexity->confidence * 100.0f);
                }
            }
        }
    }
}

int save_html_report(AnalysisReport* report, const char* output_path) {
    if (!report || !output_path) return -1;
    
    FILE* f = fopen(output_path, "w");
    if (!f) return -1;
    
    fprintf(f, "<!DOCTYPE html>\n<html>\n<head>\n");
    fprintf(f, "<title>CiOpt Analysis Report</title>\n");
    fprintf(f, "<style>\n");
    fprintf(f, "body { font-family: Arial, sans-serif; margin: 20px; }\n");
    fprintf(f, "h1 { color: #333; }\n");
    fprintf(f, ".summary { background: #f5f5f5; padding: 15px; border-radius: 5px; }\n");
    fprintf(f, ".critical { color: red; }\n");
    fprintf(f, ".warning { color: orange; }\n");
    fprintf(f, "table { border-collapse: collapse; width: 100%%; }\n");
    fprintf(f, "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n");
    fprintf(f, "th { background-color: #4CAF50; color: white; }\n");
    fprintf(f, "</style>\n");
    fprintf(f, "</head>\n<body>\n");
    
    fprintf(f, "<h1>FiOpt/CiOpt Analysis Report</h1>\n");
    fprintf(f, "<div class=\"summary\">\n");
    fprintf(f, "<p><strong>Files analyzed:</strong> %d</p>\n", analysis_report_total_files(report));
    fprintf(f, "<p><strong>Functions analyzed:</strong> %d</p>\n", analysis_report_total_functions(report));
    fprintf(f, "<p><strong>Total lines:</strong> %d</p>\n", analysis_report_total_lines(report));
    fprintf(f, "<p><strong>Worst complexity:</strong> %s</p>\n", analysis_report_complexity(report));
    fprintf(f, "<p><strong>Total issues:</strong> %d</p>\n", analysis_report_total_issues(report));
    fprintf(f, "</div>\n");
    
    fprintf(f, "<h2>Files</h2>\n<table>\n");
    fprintf(f, "<tr><th>File</th><th>Lines</th><th>Functions</th><th>Issues</th><th>Worst Complexity</th></tr>\n");
    
    for (int i = 0; i < report->num_files; i++) {
        FileReport* file = report->files[i];
        fprintf(f, "<tr>");
        fprintf(f, "<td>%s</td>", file->filepath ? file->filepath : "unknown");
        fprintf(f, "<td>%d</td>", file->line_count);
        fprintf(f, "<td>%d</td>", file_report_total_functions(file));
        fprintf(f, "<td>%d</td>", file_report_total_issues(file));
        fprintf(f, "<td>%s</td>", complexity_to_string(file_report_worst_complexity(file)));
        fprintf(f, "</tr>\n");
    }
    
    fprintf(f, "</table>\n");
    fprintf(f, "</body>\n</html>\n");
    
    fclose(f);
    return 0;
}

char* render_json(AnalysisReport* report) {
    if (!report) return NULL;
    
    size_t buf_size = 8192;
    char* buffer = (char*)malloc(buf_size);
    if (!buffer) return NULL;
    
    size_t offset = 0;
    offset += snprintf(buffer + offset, buf_size - offset, "{\n");
    offset += snprintf(buffer + offset, buf_size - offset,
        "  \"version\": \"%s\",\n", report->fiopt_version ? report->fiopt_version : "0.1.0");
    offset += snprintf(buffer + offset, buf_size - offset,
        "  \"timestamp\": \"%s\",\n", report->timestamp ? report->timestamp : "");
    offset += snprintf(buffer + offset, buf_size - offset,
        "  \"duration_ms\": %.2f,\n", report->analysis_duration_ms);
    offset += snprintf(buffer + offset, buf_size - offset,
        "  \"total_files\": %d,\n", analysis_report_total_files(report));
    offset += snprintf(buffer + offset, buf_size - offset,
        "  \"total_functions\": %d,\n", analysis_report_total_functions(report));
    offset += snprintf(buffer + offset, buf_size - offset,
        "  \"total_lines\": %d,\n", analysis_report_total_lines(report));
    offset += snprintf(buffer + offset, buf_size - offset,
        "  \"worst_complexity\": \"%s\",\n", analysis_report_complexity(report));
    offset += snprintf(buffer + offset, buf_size - offset,
        "  \"total_issues\": %d,\n", analysis_report_total_issues(report));
    offset += snprintf(buffer + offset, buf_size - offset, "  \"files\": [\n");
    
    for (int i = 0; i < report->num_files; i++) {
        FileReport* file = report->files[i];
        offset += snprintf(buffer + offset, buf_size - offset, "    {\n");
        offset += snprintf(buffer + offset, buf_size - offset,
            "      \"path\": \"%s\",\n", file->filepath ? file->filepath : "");
        offset += snprintf(buffer + offset, buf_size - offset,
            "      \"lines\": %d,\n", file->line_count);
        offset += snprintf(buffer + offset, buf_size - offset,
            "      \"functions\": %d,\n", file_report_total_functions(file));
        offset += snprintf(buffer + offset, buf_size - offset,
            "      \"issues\": %d,\n", file_report_total_issues(file));
        offset += snprintf(buffer + offset, buf_size - offset,
            "      \"worst_complexity\": \"%s\"\n", 
            complexity_to_string(file_report_worst_complexity(file)));
        offset += snprintf(buffer + offset, buf_size - offset, 
            "    }%s\n", (i < report->num_files - 1) ? "," : "");
    }
    
    offset += snprintf(buffer + offset, buf_size - offset, "  ]\n");
    offset += snprintf(buffer + offset, buf_size - offset, "}\n");
    
    return buffer;
}

int save_json_report(AnalysisReport* report, const char* output_path) {
    if (!report || !output_path) return -1;
    
    char* json = render_json(report);
    if (!json) return -1;
    
    FILE* f = fopen(output_path, "w");
    if (!f) {
        free(json);
        return -1;
    }
    
    fputs(json, f);
    fclose(f);
    free(json);
    
    return 0;
}
