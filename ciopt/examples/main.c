/**
 * main.c - Example usage of CiOpt library
 * 
 * This demonstrates how to use the CiOpt library to analyze code.
 */

#include "ciopt.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_analyze>\n", argv[0]);
        fprintf(stderr, "\nCiOpt - Code Complexity & Optimization Analysis Tool\n");
        fprintf(stderr, "Version: %s\n\n", CIOPT_VERSION);
        fprintf(stderr, "Analyzes Python and C source code for:\n");
        fprintf(stderr, "  - Time complexity estimation\n");
        fprintf(stderr, "  - Loop detection and nesting analysis\n");
        fprintf(stderr, "  - Recursion pattern detection\n");
        fprintf(stderr, "  - Dead code identification\n");
        fprintf(stderr, "  - Anti-pattern detection\n");
        fprintf(stderr, "  - Data structure misuse\n");
        return 1;
    }
    
    const char* path = argv[1];
    
    /* Create configuration */
    AnalysisConfig* config = config_create_default();
    if (!config) {
        fprintf(stderr, "Error: Failed to create configuration\n");
        return 1;
    }
    
    /* Set output format based on extension if provided */
    if (argc >= 3) {
        if (strcmp(argv[2], "--html") == 0 || strcmp(argv[2], "-h") == 0) {
            config->report_format = FORMAT_HTML;
        } else if (strcmp(argv[2], "--json") == 0 || strcmp(argv[2], "-j") == 0) {
            config->report_format = FORMAT_JSON;
        } else if (strcmp(argv[2], "--verbose") == 0 || strcmp(argv[2], "-v") == 0) {
            config->verbose = true;
        }
    }
    
    printf("CiOpt v%s - Analyzing: %s\n\n", CIOPT_VERSION, path);
    
    /* Analyze the project/path */
    AnalysisReport* report = analyze_project(path, config);
    if (!report) {
        fprintf(stderr, "Error: Failed to analyze project\n");
        config_free(config);
        return 1;
    }
    
    /* Generate and print report */
    char* output = NULL;
    switch (config->report_format) {
        case FORMAT_HTML:
            output = generate_html_report(report);
            break;
        case FORMAT_JSON:
            output = generate_json_report(report);
            break;
        case FORMAT_TERMINAL:
        default:
            output = generate_terminal_report(report);
            break;
    }
    
    if (output) {
        printf("%s\n", output);
        
        /* Write to file if output path specified */
        if (config->output_path) {
            if (write_report_to_file(output, config->output_path) == 0) {
                printf("\nReport written to: %s\n", config->output_path);
            }
        }
        
        free(output);
    }
    
    /* Print summary */
    print_summary(report);
    
    /* Cleanup */
    analysis_report_free(report);
    config_free(config);
    
    return 0;
}
