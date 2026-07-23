/**
 * ciopt_core.c - Core implementation of CiOpt
 * 
 * This implements the core data structures and utilities for CiOpt,
 * ported from the Python FiOpt project.
 */

#include "ciopt.h"

/* ============================================================================
 * Complexity Class Utilities
 * ============================================================================ */

int complexity_rank(ComplexityClass c) {
    return (int)c;
}

int complexity_compare(ComplexityClass c1, ComplexityClass c2) {
    return complexity_rank(c1) - complexity_rank(c2);
}

const char* complexity_to_string(ComplexityClass c) {
    switch (c) {
        case O_1: return "O(1)";
        case O_LOG_N: return "O(log n)";
        case O_N: return "O(n)";
        case O_N_LOG_N: return "O(n log n)";
        case O_N_SQUARED: return "O(n²)";
        case O_N_CUBED: return "O(n³)";
        case O_N_K: return "O(n^k)";
        case O_2_N: return "O(2^n)";
        case O_N_FACTORIAL: return "O(n!)";
        default: return "O(?)";
    }
}

ComplexityClass combine_complexities(ComplexityClass c1, ComplexityClass c2) {
    /* Polynomial degree mapping */
    double degree[] = {
        0,      /* O_1 */
        0.5,    /* O_LOG_N */
        1,      /* O_N */
        1.5,    /* O_N_LOG_N */
        2,      /* O_N_SQUARED */
        3,      /* O_N_CUBED */
        4,      /* O_N_K */
        100,    /* O_2_N */
        200,    /* O_N_FACTORIAL */
        0       /* O_UNKNOWN */
    };
    
    double sum = degree[c1] + degree[c2];
    
    if (sum <= 0) return O_1;
    if (sum <= 0.5) return O_LOG_N;
    if (sum <= 1) return O_N;
    if (sum <= 1.5) return O_N_LOG_N;
    if (sum <= 2) return O_N_SQUARED;
    if (sum <= 3) return O_N_CUBED;
    if (sum <= 4) return O_N_K;
    if (sum <= 100) return O_2_N;
    return O_N_FACTORIAL;
}

ComplexityClass complexity_from_string(const char* s) {
    if (strcmp(s, "O(1)") == 0 || strcmp(s, "O_1") == 0) return O_1;
    if (strcmp(s, "O(log n)") == 0 || strcmp(s, "O_LOG_N") == 0) return O_LOG_N;
    if (strcmp(s, "O(n)") == 0 || strcmp(s, "O_N") == 0) return O_N;
    if (strcmp(s, "O(n log n)") == 0 || strcmp(s, "O_N_LOG_N") == 0) return O_N_LOG_N;
    if (strcmp(s, "O(n²)") == 0 || strcmp(s, "O_N_SQUARED") == 0) return O_N_SQUARED;
    if (strcmp(s, "O(n³)") == 0 || strcmp(s, "O_N_CUBED") == 0) return O_N_CUBED;
    if (strcmp(s, "O(n^k)") == 0 || strcmp(s, "O_N_K") == 0) return O_N_K;
    if (strcmp(s, "O(2^n)") == 0 || strcmp(s, "O_2_N") == 0) return O_2_N;
    if (strcmp(s, "O(n!)") == 0 || strcmp(s, "O_N_FACTORIAL") == 0) return O_N_FACTORIAL;
    return O_UNKNOWN;
}

const char* severity_to_string(Severity s) {
    switch (s) {
        case SEVERITY_INFO: return "info";
        case SEVERITY_WARNING: return "warning";
        case SEVERITY_CRITICAL: return "critical";
        default: return "unknown";
    }
}

const char* pattern_category_to_string(PatternCategory c) {
    switch (c) {
        case PATTERN_DATA_STRUCTURE: return "data_structure";
        case PATTERN_STRING_OPERATION: return "string_operation";
        case PATTERN_ALGORITHM: return "algorithm";
        case PATTERN_IO_OPERATION: return "io_operation";
        case PATTERN_MEMORY: return "memory";
        case PATTERN_COMPUTATION: return "computation";
        default: return "unknown";
    }
}

/* ============================================================================
 * String Utilities
 * ============================================================================ */

char* ciopt_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* dup = (char*)malloc(len);
    if (dup) {
        memcpy(dup, s, len);
    }
    return dup;
}

char** ciopt_split_string(const char* s, char delimiter, int* count) {
    if (!s) {
        *count = 0;
        return NULL;
    }
    
    /* Count delimiters */
    int n = 1;
    for (const char* p = s; *p; p++) {
        if (*p == delimiter) n++;
    }
    
    char** result = (char**)malloc(n * sizeof(char*));
    if (!result) {
        *count = 0;
        return NULL;
    }
    
    /* Split */
    const char* start = s;
    int i = 0;
    for (const char* p = s; ; p++) {
        if (*p == delimiter || *p == '\0') {
            size_t len = p - start;
            result[i] = (char*)malloc(len + 1);
            if (result[i]) {
                memcpy(result[i], start, len);
                result[i][len] = '\0';
            }
            i++;
            if (*p == '\0') break;
            start = p + 1;
        }
    }
    
    *count = n;
    return result;
}

void ciopt_free_string_array(char** arr, int count) {
    if (!arr) return;
    for (int i = 0; i < count; i++) {
        free(arr[i]);
    }
    free(arr);
}

/* ============================================================================
 * Configuration
 * ============================================================================ */

AnalysisConfig* config_create_default(void) {
    AnalysisConfig* config = (AnalysisConfig*)calloc(1, sizeof(AnalysisConfig));
    if (!config) return NULL;
    
    config->complexity_warning_threshold = O_N_SQUARED;
    config->complexity_critical_threshold = O_N_CUBED;
    config->max_nesting_depth = 3;
    config->detect_dead_code = true;
    config->detect_anti_patterns = true;
    config->detect_data_structure_issues = true;
    config->detect_recursion_issues = true;
    config->report_format = FORMAT_TERMINAL;
    config->output_path = NULL;
    config->include_source = false;
    config->verbose = false;
    
    /* Default extensions */
    config->num_extensions = 2;
    config->file_extensions = (char**)malloc(2 * sizeof(char*));
    if (config->file_extensions) {
        config->file_extensions[0] = ciopt_strdup(".py");
        config->file_extensions[1] = ciopt_strdup(".c");
    }
    
    config->num_exclude_dirs = 0;
    config->exclude_dirs = NULL;
    
    return config;
}

void config_free(AnalysisConfig* config) {
    if (!config) return;
    free(config->output_path);
    ciopt_free_string_array(config->file_extensions, config->num_extensions);
    ciopt_free_string_array(config->exclude_dirs, config->num_exclude_dirs);
    free(config);
}

/* ============================================================================
 * IR Node Implementation
 * ============================================================================ */

IRNode* ir_node_create(IROpcode opcode, int lineno) {
    IRNode* node = (IRNode*)calloc(1, sizeof(IRNode));
    if (!node) return NULL;
    
    node->opcode = opcode;
    node->lineno = lineno;
    node->reads = NULL;
    node->num_reads = 0;
    node->writes = NULL;
    node->num_writes = 0;
    node->label = NULL;
    node->target = NULL;
    node->call_target = NULL;
    node->call_args = 0;
    node->loop_type = LOOP_FOR;
    node->loop_var = NULL;
    node->loop_iterable = NULL;
    node->next = NULL;
    
    return node;
}

void ir_node_free(IRNode* node) {
    if (!node) return;
    ciopt_free_string_array(node->reads, node->num_reads);
    ciopt_free_string_array(node->writes, node->num_writes);
    free(node->label);
    free(node->target);
    free(node->call_target);
    free(node->loop_var);
    free(node->loop_iterable);
    free(node);
}

/* ============================================================================
 * Basic Block Implementation
 * ============================================================================ */

BasicBlock* basic_block_create(int id, const char* label, BlockType block_type) {
    BasicBlock* block = (BasicBlock*)calloc(1, sizeof(BasicBlock));
    if (!block) return NULL;
    
    block->id = id;
    block->label = ciopt_strdup(label ? label : "");
    block->block_type = block_type;
    block->instructions = NULL;
    block->num_instructions = 0;
    block->successors = NULL;
    block->num_successors = 0;
    block->predecessors = NULL;
    block->num_predecessors = 0;
    block->loop_depth = 0;
    block->loop_header_id = -1;
    block->start_line = 0;
    block->end_line = 0;
    block->next = NULL;
    
    return block;
}

void basic_block_free(BasicBlock* block) {
    if (!block) return;
    free(block->label);
    
    /* Free instructions */
    IRNode* instr = block->instructions;
    while (instr) {
        IRNode* next = instr->next;
        ir_node_free(instr);
        instr = next;
    }
    
    free(block->successors);
    free(block->predecessors);
    free(block);
}

void basic_block_add_instruction(BasicBlock* block, IRNode* node) {
    if (!block || !node) return;
    
    if (!block->instructions) {
        block->instructions = node;
        block->start_line = node->lineno;
    } else {
        IRNode* tail = block->instructions;
        while (tail->next) tail = tail->next;
        tail->next = node;
    }
    block->num_instructions++;
    block->end_line = node->lineno;
}

void basic_block_add_successor(BasicBlock* block, int succ_id) {
    if (!block) return;
    
    /* Check if already exists */
    for (int i = 0; i < block->num_successors; i++) {
        if (block->successors[i] == succ_id) return;
    }
    
    block->num_successors++;
    block->successors = (int*)realloc(block->successors, 
                                       block->num_successors * sizeof(int));
    if (block->successors) {
        block->successors[block->num_successors - 1] = succ_id;
    }
}

void basic_block_add_predecessor(BasicBlock* block, int pred_id) {
    if (!block) return;
    
    /* Check if already exists */
    for (int i = 0; i < block->num_predecessors; i++) {
        if (block->predecessors[i] == pred_id) return;
    }
    
    block->num_predecessors++;
    block->predecessors = (int*)realloc(block->predecessors,
                                         block->num_predecessors * sizeof(int));
    if (block->predecessors) {
        block->predecessors[block->num_predecessors - 1] = pred_id;
    }
}

/* ============================================================================
 * CFG Implementation
 * ============================================================================ */

CFG* cfg_create(const char* name) {
    CFG* cfg = (CFG*)calloc(1, sizeof(CFG));
    if (!cfg) return NULL;
    
    cfg->name = ciopt_strdup(name ? name : "<unknown>");
    cfg->blocks = NULL;
    cfg->block_count = 0;
    cfg->entry_block_id = 0;
    cfg->exit_block_ids = NULL;
    cfg->num_exit_blocks = 0;
    cfg->loops = NULL;
    cfg->num_loops = 0;
    
    return cfg;
}

void cfg_free(CFG* cfg) {
    if (!cfg) return;
    free(cfg->name);
    
    BasicBlock* block = cfg->blocks;
    while (block) {
        BasicBlock* next = block->next;
        basic_block_free(block);
        block = next;
    }
    
    free(cfg->exit_block_ids);
    
    for (int i = 0; i < cfg->num_loops; i++) {
        free(cfg->loops[i].loop_var);
        free(cfg->loops[i].iterable_name);
    }
    free(cfg->loops);
    
    free(cfg);
}

char* cfg_to_dot(CFG* cfg) {
    if (!cfg) return NULL;
    
    /* Simple DOT export - allocate generous buffer */
    size_t buf_size = 8192;
    char* buf = (char*)malloc(buf_size);
    if (!buf) return NULL;
    
    size_t offset = 0;
    offset += snprintf(buf + offset, buf_size - offset, 
                       "digraph \"%s\" {\n", cfg->name);
    offset += snprintf(buf + offset, buf_size - offset,
                       "  node [shape=box, style=filled];\n");
    
    BasicBlock* block = cfg->blocks;
    while (block) {
        const char* color = "#FFFFFF";
        switch (block->block_type) {
            case BLOCK_ENTRY: color = "#90EE90"; break;
            case BLOCK_EXIT: color = "#FFB6C1"; break;
            case BLOCK_LOOP_HEADER: color = "#87CEEB"; break;
            case BLOCK_LOOP_BODY: color = "#B0E0E6"; break;
            case BLOCK_BRANCH_TRUE: color = "#FFFACD"; break;
            case BLOCK_BRANCH_FALSE: color = "#FFA07A"; break;
            case BLOCK_EXCEPT_HANDLER: color = "#DDA0DD"; break;
            default: break;
        }
        
        offset += snprintf(buf + offset, buf_size - offset,
                           "  %d [label=\"%s (depth=%d)\", fillcolor=\"%s\"];\n",
                           block->id, block->label, block->loop_depth, color);
        
        for (int i = 0; i < block->num_successors; i++) {
            offset += snprintf(buf + offset, buf_size - offset,
                               "  %d -> %d;\n", block->id, block->successors[i]);
        }
        
        block = block->next;
    }
    
    offset += snprintf(buf + offset, buf_size - offset, "}\n");
    
    return buf;
}

BackEdge* cfg_get_back_edges(CFG* cfg, int* num_edges) {
    if (!cfg || !num_edges) {
        if (num_edges) *num_edges = 0;
        return NULL;
    }
    
    /* Simple DFS-based back edge detection */
    *num_edges = 0;
    BackEdge* edges = (BackEdge*)malloc(CIOPT_MAX_LOOPS * sizeof(BackEdge));
    if (!edges) return NULL;
    
    bool* visited = (bool*)calloc(cfg->block_count, sizeof(bool));
    bool* on_stack = (bool*)calloc(cfg->block_count, sizeof(bool));
    
    if (!visited || !on_stack) {
        free(visited);
        free(on_stack);
        free(edges);
        *num_edges = 0;
        return NULL;
    }
    
    /* Simple iterative DFS from entry block */
    int stack[CIOPT_MAX_LOOPS];
    int stack_pos = 0;
    stack[stack_pos++] = cfg->entry_block_id;
    
    while (stack_pos > 0 && *num_edges < CIOPT_MAX_LOOPS) {
        int current = stack[stack_pos - 1];
        
        if (!visited[current]) {
            visited[current] = true;
            on_stack[current] = true;
        }
        
        BasicBlock* block = cfg->blocks;
        while (block && block->id != current) block = block->next;
        
        if (!block) {
            stack_pos--;
            on_stack[current] = false;
            continue;
        }
        
        bool pushed = false;
        for (int i = 0; i < block->num_successors; i++) {
            int succ = block->successors[i];
            if (on_stack[succ]) {
                /* Back edge found */
                edges[*num_edges].from_block = current;
                edges[*num_edges].to_block = succ;
                (*num_edges)++;
            } else if (!visited[succ]) {
                stack[stack_pos++] = succ;
                pushed = true;
            }
        }
        
        if (!pushed) {
            stack_pos--;
            on_stack[current] = false;
        }
    }
    
    free(visited);
    free(on_stack);
    return edges;
}
