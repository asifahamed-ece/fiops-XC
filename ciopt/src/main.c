/**
 * ciopt — C Implementation of FiOpt
 * Main CLI entry point
 */

#include "ciopt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static void print_version(void) {
    printf("ciopt v%s\n", CIOPT_VERSION);
    printf("C Implementation of FiOpt — AI-Powered Code Complexity & Optimization Engine\n");
}

static void print_usage(const char* program_name) {
    printf("Usage: %s [command] [options]\n\n", program_name);
    printf("Commands:\n");
    printf("  analyze <path>    Analyze Python code for complexity and bottlenecks\n");
    printf("  version           Show version information\n");
    printf("\nOptions for 'analyze' command:\n");
    printf("  -f, --format      Output format: terminal, html, json (default: terminal)\n");
    printf("  -o, --output      Output file path (for html/json formats)\n");
    printf("  -v, --verbose     Show detailed complexity explanations\n");
    printf("  --no-dead-code    Skip dead code detection\n");
    printf("  --no-patterns     Skip anti-pattern detection\n");
    printf("  --threshold       Complexity threshold: O(n), O(n²), O(n³) (default: O(n²))\n");
    printf("  -h, --help        Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s analyze app.py\n", program_name);
    printf("  %s analyze src/ --format html -o report.html\n", program_name);
    printf("  %s analyze app.py -v --format json -o results.json\n", program_name);
}

static ComplexityClass parse_threshold(const char* str) {
    if (strcmp(str, "O(n)") == 0) return O_N;
    if (strcmp(str, "O(n²)") == 0 || strcmp(str, "O(n^2)") == 0) return O_N_SQUARED;
    if (strcmp(str, "O(n³)") == 0 || strcmp(str, "O(n^3)") == 0) return O_N_CUBED;
    return O_N_SQUARED;  /* default */
}

static ReportFormat parse_format(const char* str) {
    if (strcmp(str, "terminal") == 0) return FORMAT_TERMINAL;
    if (strcmp(str, "html") == 0) return FORMAT_HTML;
    if (strcmp(str, "json") == 0) return FORMAT_JSON;
    return FORMAT_TERMINAL;  /* default */
}

static int cmd_analyze(int argc, char** argv) {
    if (argc < 1) {
        fprintf(stderr, "Error: No path specified\n");
        return 1;
    }
    
    const char* path = argv[0];
    ReportFormat output_format = FORMAT_TERMINAL;
    const char* output_path = NULL;
    bool verbose = false;
    bool no_dead_code = false;
    bool no_patterns = false;
    ComplexityClass threshold = O_N_SQUARED;
    
    /* Parse options */
    static struct option long_options[] = {
        {"format", required_argument, 0, 'f'},
        {"output", required_argument, 0, 'o'},
        {"verbose", no_argument, 0, 'v'},
        {"no-dead-code", no_argument, 0, 'd'},
        {"no-patterns", no_argument, 0, 'p'},
        {"threshold", required_argument, 0, 't'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    optind = 1;  /* Reset getopt */
    int opt;
    while ((opt = getopt_long(argc, argv, "f:o:vdpht:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'f':
                output_format = parse_format(optarg);
                break;
            case 'o':
                output_path = optarg;
                break;
            case 'v':
                verbose = true;
                break;
            case 'd':
                no_dead_code = true;
                break;
            case 'p':
                no_patterns = true;
                break;
            case 't':
                threshold = parse_threshold(optarg);
                break;
            case 'h':
                print_usage("ciopt");
                return 0;
            default:
                return 1;
        }
    }
    
    /* Create configuration */
    AnalysisConfig* config = config_create_default();
    if (!config) {
        fprintf(stderr, "Error: Failed to create configuration\n");
        return 1;
    }
    
    config->detect_dead_code = !no_dead_code;
    config->detect_anti_patterns = !no_patterns;
    config->verbose = verbose;
    config->complexity_warning_threshold = threshold;
    
    /* Perform analysis */
    clock_t start = clock();
    AnalysisReport* report = analyze(path, config);
    clock_t end = clock();
    
    if (!report) {
        fprintf(stderr, "Error: Analysis failed for '%s'\n", path);
        config_free(config);
        return 1;
    }
    
    report->analysis_duration_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
    
    /* Output results */
    if (output_format == FORMAT_TERMINAL) {
        render_terminal(report, verbose);
    } else if (output_format == FORMAT_HTML) {
        const char* out = output_path ? output_path : "ciopt_report.html";
        if (save_html_report(report, out) == 0) {
            printf("HTML report saved to: %s\n", out);
        } else {
            fprintf(stderr, "Error: Failed to save HTML report\n");
        }
    } else if (output_format == FORMAT_JSON) {
        if (output_path) {
            if (save_json_report(report, output_path) == 0) {
                printf("JSON report saved to: %s\n", output_path);
            } else {
                fprintf(stderr, "Error: Failed to save JSON report\n");
            }
        } else {
            char* json = render_json(report);
            if (json) {
                printf("%s\n", json);
                free(json);
            }
        }
    }
    
    /* Check for critical issues */
    int critical_count = 0;
    for (int i = 0; i < report->num_files; i++) {
        FileReport* file = report->files[i];
        for (int j = 0; j < file->num_function_reports; j++) {
            FunctionReport* func = file->function_reports[j];
            if (func->severity == SEVERITY_CRITICAL) {
                critical_count++;
            }
        }
    }
    
    /* Cleanup */
    analysis_report_free(report);
    config_free(config);
    
    return critical_count > 0 ? 1 : 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 0;
    }
    
    const char* command = argv[1];
    
    if (strcmp(command, "version") == 0) {
        print_version();
        return 0;
    }
    
    if (strcmp(command, "analyze") == 0) {
        return cmd_analyze(argc - 2, argv + 2);
    }
    
    if (strcmp(command, "-h") == 0 || strcmp(command, "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    
    if (strcmp(command, "-v") == 0 || strcmp(command, "--version") == 0) {
        print_version();
        return 0;
    }
    
    fprintf(stderr, "Unknown command: %s\n", command);
    print_usage(argv[0]);
    return 1;
}
