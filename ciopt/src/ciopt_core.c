/**
 * ciopt — C Implementation of FiOpt
 * Core utilities and configuration management
 */

#include "ciopt.h"

/* ============================================================================
 * String Utilities
 * ============================================================================ */

char* str_duplicate(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* dup = (char*)malloc(len);
    if (dup) {
        memcpy(dup, s, len);
    }
    return dup;
}

char** str_split(const char* s, char delimiter, int* count) {
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
    
    /* Split string */
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
    
    *count = i;
    return result;
}

void str_array_free(char** arr, int count) {
    if (!arr) return;
    for (int i = 0; i < count; i++) {
        free(arr[i]);
    }
    free(arr);
}

char* get_timestamp_iso(void) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char* buffer = (char*)malloc(32);
    if (buffer) {
        strftime(buffer, 32, "%Y-%m-%dT%H:%M:%S", tm_info);
    }
    return buffer;
}

/* ============================================================================
 * Complexity Class Functions
 * ============================================================================ */

int complexity_rank(ComplexityClass c) {
    switch (c) {
        case O_1: return 0;
        case O_LOG_N: return 1;
        case O_N: return 2;
        case O_N_LOG_N: return 3;
        case O_N_SQUARED: return 4;
        case O_N_CUBED: return 5;
        case O_N_K: return 6;
        case O_2_N: return 7;
        case O_N_FACTORIAL: return 8;
        case O_UNKNOWN: return 9;
        default: return 9;
    }
}

int complexity_compare(ComplexityClass c1, ComplexityClass c2) {
    int r1 = complexity_rank(c1);
    int r2 = complexity_rank(c2);
    if (r1 < r2) return -1;
    if (r1 > r2) return 1;
    return 0;
}

const char* complexity_to_string(ComplexityClass c) {
    switch (c) {
        case O_1: return "O(1)";
        case O_LOG_N: return "O(log n)";
        case O_N: return "O(n)";
        case O_N_LOG_N: return "O(n log n)";
        case O_N_SQUARED: return "O(n²)";
        case O_N_CUBED: return "O(n³)";
        case O_N_K: return "O(nᵏ)";
        case O_2_N: return "O(2ⁿ)";
        case O_N_FACTORIAL: return "O(n!)";
        case O_UNKNOWN: return "Unknown";
        default: return "Unknown";
    }
}

ComplexityClass combine_complexities(ComplexityClass c1, ComplexityClass c2) {
    /* Use polynomial degree for multiplication */
    float degree[10] = {
        0,    /* O_1 */
        0.5,  /* O_LOG_N */
        1,    /* O_N */
        1.5,  /* O_N_LOG_N */
        2,    /* O_N_SQUARED */
        3,    /* O_N_CUBED */
        4,    /* O_N_K */
        100,  /* O_2_N */
        200,  /* O_N_FACTORIAL */
        0     /* O_UNKNOWN */
    };
    
    float d1 = (c1 >= 0 && c1 <= 9) ? degree[c1] : 0;
    float d2 = (c2 >= 0 && c2 <= 9) ? degree[c2] : 0;
    float sum = d1 + d2;
    
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

/* ============================================================================
 * Analysis Configuration
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
    config->include_source = true;
    config->verbose = false;
    
    /* Default file extensions */
    config->num_extensions = 1;
    config->file_extensions = (char**)malloc(sizeof(char*));
    if (config->file_extensions) {
        config->file_extensions[0] = str_duplicate(".py");
    }
    
    /* Default exclude directories */
    const char* defaults[] = {
        "__pycache__", ".git", ".venv", "venv", "node_modules",
        ".tox", ".eggs", "dist", "build", ".mypy_cache"
    };
    config->num_exclude_dirs = sizeof(defaults) / sizeof(defaults[0]);
    config->exclude_dirs = (char**)malloc(config->num_exclude_dirs * sizeof(char*));
    if (config->exclude_dirs) {
        for (int i = 0; i < config->num_exclude_dirs; i++) {
            config->exclude_dirs[i] = str_duplicate(defaults[i]);
        }
    }
    
    return config;
}

void config_free(AnalysisConfig* config) {
    if (!config) return;
    
    free(config->output_path);
    str_array_free(config->file_extensions, config->num_extensions);
    str_array_free(config->exclude_dirs, config->num_exclude_dirs);
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
    
    str_array_free(node->reads, node->num_reads);
    str_array_free(node->writes, node->num_writes);
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
    block->label = str_duplicate(label);
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
    
    for (int i = 0; i < block->num_instructions; i++) {
        ir_node_free(block->instructions[i]);
    }
    free(block->instructions);
    free(block->label);
    free(block->successors);
    free(block->predecessors);
    free(block);
}

void basic_block_add_instruction(BasicBlock* block, IRNode* node) {
    if (!block || !node) return;
    
    block->num_instructions++;
    block->instructions = (IRNode**)realloc(
        block->instructions,
        block->num_instructions * sizeof(IRNode*)
    );
    if (block->instructions) {
        block->instructions[block->num_instructions - 1] = node;
    }
    
    if (!block->start_line) {
        block->start_line = node->lineno;
    }
    block->end_line = node->lineno;
}

void basic_block_add_successor(BasicBlock* block, int successor_id) {
    if (!block) return;
    
    /* Check if already exists */
    for (int i = 0; i < block->num_successors; i++) {
        if (block->successors[i] == successor_id) return;
    }
    
    block->num_successors++;
    block->successors = (int*)realloc(
        block->successors,
        block->num_successors * sizeof(int)
    );
    if (block->successors) {
        block->successors[block->num_successors - 1] = successor_id;
    }
}

void basic_block_add_predecessor(BasicBlock* block, int predecessor_id) {
    if (!block) return;
    
    /* Check if already exists */
    for (int i = 0; i < block->num_predecessors; i++) {
        if (block->predecessors[i] == predecessor_id) return;
    }
    
    block->num_predecessors++;
    block->predecessors = (int*)realloc(
        block->predecessors,
        block->num_predecessors * sizeof(int)
    );
    if (block->predecessors) {
        block->predecessors[block->num_predecessors - 1] = predecessor_id;
    }
}

bool basic_block_has_call(BasicBlock* block) {
    if (!block) return false;
    
    for (int i = 0; i < block->num_instructions; i++) {
        if (block->instructions[i]->opcode == IR_CALL) {
            return true;
        }
    }
    return false;
}

/* ============================================================================
 * CFG Implementation
 * ============================================================================ */

CFG* cfg_create(const char* name) {
    CFG* cfg = (CFG*)calloc(1, sizeof(CFG));
    if (!cfg) return NULL;
    
    cfg->name = str_duplicate(name);
    cfg->blocks = NULL;
    cfg->num_blocks = 0;
    cfg->entry_block_id = 0;
    cfg->exit_block_ids = NULL;
    cfg->num_exit_blocks = 0;
    cfg->loops = NULL;
    cfg->num_loops = 0;
    
    return cfg;
}

void cfg_free(CFG* cfg) {
    if (!cfg) return;
    
    for (int i = 0; i < cfg->num_blocks; i++) {
        basic_block_free(cfg->blocks[i]);
    }
    free(cfg->blocks);
    free(cfg->name);
    free(cfg->exit_block_ids);
    free(cfg->loops);
    free(cfg);
}

BasicBlock* cfg_entry_block(CFG* cfg) {
    if (!cfg || cfg->num_blocks == 0) return NULL;
    return cfg->blocks[cfg->entry_block_id];
}

int cfg_max_loop_depth(CFG* cfg) {
    if (!cfg || cfg->num_blocks == 0) return 0;
    
    int max_depth = 0;
    for (int i = 0; i < cfg->num_blocks; i++) {
        if (cfg->blocks[i]->loop_depth > max_depth) {
            max_depth = cfg->blocks[i]->loop_depth;
        }
    }
    return max_depth;
}

char* cfg_to_dot(CFG* cfg) {
    if (!cfg) return NULL;
    
    /* Allocate buffer for DOT output */
    size_t buf_size = 4096;
    char* buffer = (char*)malloc(buf_size);
    if (!buffer) return NULL;
    
    size_t offset = 0;
    offset += snprintf(buffer + offset, buf_size - offset,
        "digraph \"%s\" {\n", cfg->name ? cfg->name : "CFG");
    offset += snprintf(buffer + offset, buf_size - offset,
        "  node [shape=box, style=filled, fontname=\"Courier\"];\n");
    offset += snprintf(buffer + offset, buf_size - offset,
        "  rankdir=TB;\n");
    
    const char* colors[] = {
        "#90EE90",  /* ENTRY */
        "#FFB6C1",  /* EXIT */
        "#FFFFFF",  /* NORMAL */
        "#87CEEB",  /* LOOP_HEADER */
        "#B0E0E6",  /* LOOP_BODY */
        "#DDA0DD",  /* LOOP_EXIT */
        "#FFFACD",  /* BRANCH_TRUE */
        "#FFA07A",  /* BRANCH_FALSE */
        "#DDA0DD",  /* EXCEPT_HANDLER */
        "#DDA0DD"   /* FINALLY_BLOCK */
    };
    
    for (int i = 0; i < cfg->num_blocks; i++) {
        BasicBlock* block = cfg->blocks[i];
        const char* color = (block->block_type >= 0 && block->block_type <= 9)
            ? colors[block->block_type] : "#FFFFFF";
        
        /* Build instruction label (limit to first 8) */
        char inst_label[512] = "";
        int inst_count = block->num_instructions < 8 ? block->num_instructions : 8;
        for (int j = 0; j < inst_count; j++) {
            IRNode* inst = block->instructions[j];
            const char* op_names[] = {
                "ASSIGN", "LOAD", "STORE", "CALL", "RETURN",
                "JUMP", "BRANCH", "LOOP_HEADER", "LOOP_END", "LOOP_CONTINUE",
                "LOOP_BREAK", "TRY_ENTER", "TRY_EXIT", "EXCEPT", "FINALLY",
                "WITH_ENTER", "WITH_EXIT", "YIELD", "AWAIT", "RAISE",
                "ASSERT", "PASS", "NOP"
            };
            const char* op_name = (inst->opcode >= 0 && inst->opcode <= 22)
                ? op_names[inst->opcode] : "UNKNOWN";
            
            char line_buf[64];
            snprintf(line_buf, sizeof(line_buf), "L%d: %s\\n", inst->lineno, op_name);
            strncat(inst_label, line_buf, sizeof(inst_label) - strlen(inst_label) - 1);
        }
        
        if (block->num_instructions > 8) {
            char more_buf[64];
            snprintf(more_buf, sizeof(more_buf), "... +%d more", block->num_instructions - 8);
            strncat(inst_label, more_buf, sizeof(inst_label) - strlen(inst_label) - 1);
        }
        
        offset += snprintf(buffer + offset, buf_size - offset,
            "  %d [label=\"%s\\n(depth=%d)\\n%s\", fillcolor=\"%s\"];\n",
            block->id, block->label ? block->label : "",
            block->loop_depth, inst_label, color);
        
        for (int j = 0; j < block->num_successors; j++) {
            offset += snprintf(buffer + offset, buf_size - offset,
                "  %d -> %d;\n", block->id, block->successors[j]);
        }
    }
    
    offset += snprintf(buffer + offset, buf_size - offset, "}\n");
    
    return buffer;
}
