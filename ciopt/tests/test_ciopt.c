/**
 * test_ciopt.c - Unit tests for CiOpt library
 */

#include "ciopt.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) static void name(void)
#define RUN_TEST(name) do { \
    tests_run++; \
    printf("Running %s... ", #name); \
    name(); \
    tests_passed++; \
    printf("PASSED\n"); \
} while(0)

#define ASSERT(cond, msg) do { \
    if (!(cond)) { \
        printf("FAILED: %s\n", msg); \
        return; \
    } \
} while(0)

/* Test complexity class utilities */
TEST(test_complexity_rank) {
    ASSERT(complexity_rank(O_1) == 0, "O_1 rank should be 0");
    ASSERT(complexity_rank(O_N) == 2, "O_N rank should be 2");
    ASSERT(complexity_rank(O_N_SQUARED) == 4, "O_N_SQUARED rank should be 4");
    ASSERT(complexity_rank(O_2_N) == 7, "O_2_N rank should be 7");
}

TEST(test_complexity_compare) {
    ASSERT(complexity_compare(O_1, O_N) < 0, "O_1 < O_N");
    ASSERT(complexity_compare(O_N, O_N) == 0, "O_N == O_N");
    ASSERT(complexity_compare(O_N_SQUARED, O_N) > 0, "O_N_SQUARED > O_N");
}

TEST(test_complexity_to_string) {
    ASSERT(strcmp(complexity_to_string(O_1), "O(1)") == 0, "O_1 string");
    ASSERT(strcmp(complexity_to_string(O_N), "O(n)") == 0, "O_N string");
    ASSERT(strcmp(complexity_to_string(O_N_LOG_N), "O(n log n)") == 0, "O_N_LOG_N string");
    ASSERT(strcmp(complexity_to_string(O_2_N), "O(2^n)") == 0, "O_2_N string");
}

TEST(test_combine_complexities) {
    ASSERT(combine_complexities(O_1, O_1) == O_1, "O(1) * O(1) = O(1)");
    ASSERT(combine_complexities(O_N, O_N) == O_N_SQUARED, "O(n) * O(n) = O(n²)");
    ASSERT(combine_complexities(O_N, O_N_SQUARED) == O_N_CUBED, "O(n) * O(n²) = O(n³)");
    ASSERT(combine_complexities(O_LOG_N, O_N) == O_N, "O(log n) * O(n) = O(n)");
}

/* Test configuration */
TEST(test_config_create) {
    AnalysisConfig* config = config_create_default();
    ASSERT(config != NULL, "Config should not be NULL");
    ASSERT(config->complexity_warning_threshold == O_N_SQUARED, "Default warning threshold");
    ASSERT(config->complexity_critical_threshold == O_N_CUBED, "Default critical threshold");
    ASSERT(config->detect_dead_code == true, "Dead code detection enabled by default");
    ASSERT(config->num_extensions == 2, "Should have 2 default extensions");
    config_free(config);
}

/* Test string utilities */
TEST(test_strdup) {
    const char* original = "test string";
    char* dup = ciopt_strdup(original);
    ASSERT(dup != NULL, "strdup should not return NULL");
    ASSERT(strcmp(dup, original) == 0, "strdup should copy string correctly");
    free(dup);
}

TEST(test_split_string) {
    int count = 0;
    char** parts = ciopt_split_string("a,b,c", ',', &count);
    ASSERT(count == 3, "Should split into 3 parts");
    ASSERT(strcmp(parts[0], "a") == 0, "First part should be 'a'");
    ASSERT(strcmp(parts[1], "b") == 0, "Second part should be 'b'");
    ASSERT(strcmp(parts[2], "c") == 0, "Third part should be 'c'");
    ciopt_free_string_array(parts, count);
}

/* Test IR node creation */
TEST(test_ir_node) {
    IRNode* node = ir_node_create(IR_ASSIGN, 42);
    ASSERT(node != NULL, "IR node should not be NULL");
    ASSERT(node->opcode == IR_ASSIGN, "Opcode should match");
    ASSERT(node->lineno == 42, "Line number should match");
    ir_node_free(node);
}

/* Test basic block creation */
TEST(test_basic_block) {
    BasicBlock* block = basic_block_create(1, "test_block", BLOCK_NORMAL);
    ASSERT(block != NULL, "Basic block should not be NULL");
    ASSERT(block->id == 1, "Block ID should match");
    ASSERT(strcmp(block->label, "test_block") == 0, "Label should match");
    
    /* Add instruction */
    IRNode* instr = ir_node_create(IR_LOAD, 10);
    basic_block_add_instruction(block, instr);
    ASSERT(block->num_instructions == 1, "Should have 1 instruction");
    ASSERT(block->instructions->opcode == IR_LOAD, "Instruction opcode should match");
    
    basic_block_free(block);
}

/* Test CFG creation */
TEST(test_cfg) {
    CFG* cfg = cfg_create("test_function");
    ASSERT(cfg != NULL, "CFG should not be NULL");
    ASSERT(strcmp(cfg->name, "test_function") == 0, "CFG name should match");
    
    /* Export to DOT */
    char* dot = cfg_to_dot(cfg);
    ASSERT(dot != NULL, "DOT export should not be NULL");
    ASSERT(strstr(dot, "digraph") != NULL, "DOT should contain digraph");
    free(dot);
    
    cfg_free(cfg);
}

/* Test severity and category strings */
TEST(test_severity_strings) {
    ASSERT(strcmp(severity_to_string(SEVERITY_INFO), "info") == 0, "INFO severity");
    ASSERT(strcmp(severity_to_string(SEVERITY_WARNING), "warning") == 0, "WARNING severity");
    ASSERT(strcmp(severity_to_string(SEVERITY_CRITICAL), "critical") == 0, "CRITICAL severity");
}

TEST(test_category_strings) {
    ASSERT(strcmp(pattern_category_to_string(PATTERN_DATA_STRUCTURE), "data_structure") == 0, 
           "DATA_STRUCTURE category");
    ASSERT(strcmp(pattern_category_to_string(PATTERN_ALGORITHM), "algorithm") == 0,
           "ALGORITHM category");
}

/* Main test runner */
int main(void) {
    printf("=== CiOpt Unit Tests ===\n\n");
    
    /* Complexity tests */
    RUN_TEST(test_complexity_rank);
    RUN_TEST(test_complexity_compare);
    RUN_TEST(test_complexity_to_string);
    RUN_TEST(test_combine_complexities);
    
    /* Configuration tests */
    RUN_TEST(test_config_create);
    
    /* String utility tests */
    RUN_TEST(test_strdup);
    RUN_TEST(test_split_string);
    
    /* IR tests */
    RUN_TEST(test_ir_node);
    RUN_TEST(test_basic_block);
    RUN_TEST(test_cfg);
    
    /* Enum string tests */
    RUN_TEST(test_severity_strings);
    RUN_TEST(test_category_strings);
    
    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    
    return (tests_passed == tests_run) ? 0 : 1;
}
