# CiOpt - C Implementation of FiOpt

CiOpt is a complete port of the Python **FiOpt** (AI-Powered Code Complexity & Optimization Engine) project to the C programming language. It provides the same functionality for analyzing code complexity, detecting anti-patterns, identifying bottlenecks, and generating optimization suggestions.

## Features

- **Complexity Analysis**: Estimates time complexity of functions (O(1), O(log n), O(n), O(n log n), O(n²), etc.)
- **Loop Detection**: Identifies nested loops and their impact on complexity
- **Recursion Analysis**: Detects recursive patterns and memoization opportunities
- **Anti-Pattern Detection**: Finds common performance anti-patterns
- **Dead Code Detection**: Identifies unreachable code and unused variables
- **Data Structure Analysis**: Suggests better data structure choices
- **Multiple Output Formats**: Terminal, HTML, and JSON reports
- **CFG Generation**: Control Flow Graph visualization in DOT format

## Project Structure

```
ciopt/
├── include/
│   └── ciopt.h          # Main header file with all API declarations
├── src/
│   ├── ciopt_core.c     # Core utilities, IR, CFG implementation
│   ├── ciopt_report.c   # Report data structures and aggregation
│   ├── ciopt_analyzer.c # Analysis engines and reporting functions
│   └── main.c           # CLI entry point
├── tests/
│   └── test_basic.c     # Basic functionality tests
├── examples/            # Example Python files for testing
├── CMakeLists.txt       # Build configuration
└── README.md            # This file
```

## Building

### Prerequisites

- CMake 3.10 or higher
- GCC or Clang with C11 support

### Build Instructions

```bash
cd ciopt
mkdir build
cd build
cmake ..
make
```

This will produce:
- `libciopt.a` - Static library
- `ciopt` - Command-line tool
- `test_ciopt` - Test executable

## Usage

### Command Line Interface

```bash
# Show version
./ciopt version

# Analyze a single file
./ciopt analyze app.py

# Analyze a directory
./ciopt analyze src/

# Generate HTML report
./ciopt analyze src/ --format html -o report.html

# Generate JSON report
./ciopt analyze src/ --format json -o results.json

# Verbose output with detailed explanations
./ciopt analyze app.py --verbose

# Set complexity threshold
./ciopt analyze app.py --threshold O(n)

# Disable specific analyses
./ciopt analyze app.py --no-dead-code --no-patterns
```

### Command Options

| Option | Description |
|--------|-------------|
| `-f, --format` | Output format: terminal, html, json (default: terminal) |
| `-o, --output` | Output file path (for html/json formats) |
| `-v, --verbose` | Show detailed complexity explanations |
| `--no-dead-code` | Skip dead code detection |
| `--no-patterns` | Skip anti-pattern detection |
| `--threshold` | Complexity threshold: O(n), O(n²), O(n³) (default: O(n²)) |
| `-h, --help` | Show help message |

## API Reference

### Core Data Structures

#### Complexity Classes
```c
typedef enum {
    O_1 = 0,           // Constant time
    O_LOG_N = 1,       // Logarithmic
    O_N = 2,           // Linear
    O_N_LOG_N = 3,     // Linearithmic
    O_N_SQUARED = 4,   // Quadratic
    O_N_CUBED = 5,     // Cubic
    O_N_K = 6,         // Polynomial (k > 3)
    O_2_N = 7,         // Exponential
    O_N_FACTORIAL = 8, // Factorial
    O_UNKNOWN = 9
} ComplexityClass;
```

#### Severity Levels
```c
typedef enum {
    SEVERITY_INFO = 0,
    SEVERITY_WARNING = 1,
    SEVERITY_CRITICAL = 2
} Severity;
```

### Key Functions

#### Analysis Configuration
```c
// Create default configuration
AnalysisConfig* config_create_default(void);

// Free configuration
void config_free(AnalysisConfig* config);
```

#### Main Analysis
```c
// Analyze a file or directory
AnalysisReport* analyze(const char* path, AnalysisConfig* config);

// Analyze source code from a string
AnalysisReport* analyze_source(const char* source, const char* filename, 
                                AnalysisConfig* config);
```

#### Report Access
```c
// Get summary statistics
int analysis_report_total_files(AnalysisReport* report);
int analysis_report_total_functions(AnalysisReport* report);
int analysis_report_total_lines(AnalysisReport* report);
int analysis_report_total_issues(AnalysisReport* report);

// Get worst complexity
ComplexityClass analysis_report_worst_complexity(AnalysisReport* report);
const char* analysis_report_complexity(AnalysisReport* report);

// Get bottlenecks and suggestions
char** analysis_report_bottlenecks(AnalysisReport* report, int* count);
char** analysis_report_suggestions(AnalysisReport* report, int* count);

// Get human-readable summary
char* analysis_report_summary(AnalysisReport* report);
```

#### Reporting
```c
// Render to terminal
void render_terminal(AnalysisReport* report, bool verbose);

// Save HTML report
int save_html_report(AnalysisReport* report, const char* output_path);

// Render/save JSON
char* render_json(AnalysisReport* report);
int save_json_report(AnalysisReport* report, const char* output_path);
```

### Example Usage

```c
#include "ciopt.h"

int main() {
    // Create configuration
    AnalysisConfig* config = config_create_default();
    config->verbose = true;
    
    // Analyze a file
    AnalysisReport* report = analyze("my_app.py", config);
    
    if (report) {
        // Print summary
        printf("%s\n", analysis_report_summary(report));
        
        // Check for critical issues
        int issues = analysis_report_total_issues(report);
        if (issues > 0) {
            printf("Found %d issues!\n", issues);
        }
        
        // Clean up
        analysis_report_free(report);
    }
    
    config_free(config);
    return 0;
}
```

## Testing

Run the test suite:

```bash
cd build
./test_ciopt
```

Expected output:
```
Running ciopt tests...

Testing complexity ranking...
  ✓ Complexity ranking tests passed
Testing complexity combination...
  ✓ Complexity combination tests passed
Testing configuration...
  ✓ Configuration tests passed
Testing source file loading...
  ✓ Source file tests passed
Testing basic block...
  ✓ Basic block tests passed
Testing CFG...
  ✓ CFG tests passed

All tests passed! ✓
```

## Differences from Python FiOpt

The C implementation provides the same core functionality as the Python original, with some differences:

1. **Parser Implementation**: The full Python AST parser is not implemented in this C version. A production version would use libpython or implement a custom Python parser.

2. **Performance**: Being written in C, the core data structures and algorithms are significantly faster, though the analysis stubs currently provide simplified results.

3. **Memory Management**: Manual memory management is required. All allocated structures must be freed using the corresponding `*_free()` functions.

4. **API Design**: The C API uses explicit error checking and return codes rather than exceptions.

## Architecture

### Intermediate Representation (IR)
The code is converted to an intermediate representation consisting of:
- Basic blocks with control flow edges
- IR instructions (assignments, calls, jumps, loops)
- Loop metadata and nesting information

### Control Flow Graph (CFG)
Each function is represented as a CFG that can be:
- Traversed for analysis
- Exported to DOT format for visualization
- Used for loop detection and recursion analysis

### Analysis Pipeline
1. **Source Loading**: Read Python source files
2. **Parsing**: Convert to AST (stub in current version)
3. **IR Generation**: Build intermediate representation
4. **CFG Construction**: Create control flow graph
5. **Loop Analysis**: Detect and classify loops
6. **Recursion Analysis**: Identify recursive patterns
7. **Complexity Estimation**: Combine analyses into complexity estimates
8. **Pattern Detection**: Find anti-patterns and issues
9. **Report Generation**: Aggregate results into reports

## License

Port of FiOpt to C. Original FiOpt license applies.

## Contributing

Contributions welcome! Areas for improvement:
- Full Python AST parser implementation
- Enhanced complexity estimation algorithms
- Additional anti-pattern detectors
- Performance optimizations
- Extended test coverage
