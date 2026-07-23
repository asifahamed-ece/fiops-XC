/**
 * ciopt — C Implementation of FiOpt
 * Test program for basic functionality
 */

#include "ciopt.h"
#include <stdio.h>
#include <string.h>

static int test_complexity_ranking(void) {
    printf("Testing complexity ranking...\n");
    
    /* Test that complexities are properly ordered */
    if (complexity_rank(O_1) != 0) return 1;
    if (complexity_rank(O_LOG_N) != 1) return 2;
    if (complexity_rank(O_N) != 2) return 3;
    if (complexity_rank(O_N_LOG_N) != 3) return 4;
    if (complexity_rank(O_N_SQUARED) != 4) return 5;
    if (complexity_rank(O_N_CUBED) != 5) return 6;
    if (complexity_rank(O_2_N) != 7) return 7;
    if (complexity_rank(O_N_FACTORIAL) != 8) return 8;
    
    /* Test comparison */
    if (complexity_compare(O_1, O_N) >= 0) return 9;
    if (complexity_compare(O_N, O_1) <= 0) return 10;
    if (complexity_compare(O_N, O_N) != 0) return 11;
    
    /* Test string conversion */
    if (strcmp(complexity_to_string(O_1), "O(1)") != 0) return 12;
    if (strcmp(complexity_to_string(O_N), "O(n)") != 0) return 13;
    if (strcmp(complexity_to_string(O_N_SQUARED), "O(n²)") != 0) return 14;
    
    printf("  ✓ Complexity ranking tests passed\n");
    return 0;
}

static int test_complexity_combination(void) {
    printf("Testing complexity combination...\n");
    
    /* Test multiplication rules */
    if (combine_complexities(O_1, O_1) != O_1) return 1;
    if (combine_complexities(O_N, O_1) != O_N) return 2;
    if (combine_complexities(O_N, O_N) != O_N_SQUARED) return 3;
    if (combine_complexities(O_N, O_N_SQUARED) != O_N_CUBED) return 4;
    if (combine_complexities(O_LOG_N, O_N) != O_N) return 5;
    if (combine_complexities(O_N, O_N_LOG_N) != O_N_SQUARED) return 6;
    
    printf("  ✓ Complexity combination tests passed\n");
    return 0;
}

static int test_config(void) {
    printf("Testing configuration...\n");
    
    AnalysisConfig* config = config_create_default();
    if (!config) return 1;
    
    /* Check defaults */
    if (config->complexity_warning_threshold != O_N_SQUARED) {
        config_free(config);
        return 2;
    }
    if (config->detect_dead_code != true) {
        config_free(config);
        return 3;
    }
    if (config->detect_anti_patterns != true) {
        config_free(config);
        return 4;
    }
    if (config->num_extensions != 1) {
        config_free(config);
        return 5;
    }
    
    config_free(config);
    printf("  ✓ Configuration tests passed\n");
    return 0;
}

static int test_source_file(void) {
    printf("Testing source file loading...\n");
    
    /* Create a test file */
    const char* test_content = 
        "def foo():\n"
        "    pass\n"
        "\n"
        "def bar(x):\n"
        "    return x * 2\n";
    
    FILE* f = fopen("/tmp/test_ciopt.py", "w");
    if (!f) return 1;
    fputs(test_content, f);
    fclose(f);
    
    /* Load the file */
    SourceFile* file = source_file_load("/tmp/test_ciopt.py");
    if (!file) return 2;
    
    /* Check properties */
    if (file->line_count != 5) {
        source_file_free(file);
        return 3;
    }
    
    /* Get a specific line */
    char* line = source_file_get_line(file, 2);
    if (!line || strcmp(line, "    pass") != 0) {
        free(line);
        source_file_free(file);
        return 4;
    }
    free(line);
    
    source_file_free(file);
    
    /* Clean up test file */
    remove("/tmp/test_ciopt.py");
    
    printf("  ✓ Source file tests passed\n");
    return 0;
}

static int test_basic_block(void) {
    printf("Testing basic block...\n");
    
    BasicBlock* block = basic_block_create(0, "ENTRY", BLOCK_ENTRY);
    if (!block) return 1;
    
    /* Add instructions */
    IRNode* node1 = ir_node_create(IR_ASSIGN, 1);
    IRNode* node2 = ir_node_create(IR_CALL, 2);
    
    basic_block_add_instruction(block, node1);
    basic_block_add_instruction(block, node2);
    
    if (block->num_instructions != 2) {
        basic_block_free(block);
        return 2;
    }
    
    if (block->start_line != 1) {
        basic_block_free(block);
        return 3;
    }
    
    if (block->end_line != 2) {
        basic_block_free(block);
        return 4;
    }
    
    /* Test has_call */
    if (!basic_block_has_call(block)) {
        basic_block_free(block);
        return 5;
    }
    
    /* Test successors/predecessors */
    basic_block_add_successor(block, 1);
    basic_block_add_successor(block, 2);
    basic_block_add_predecessor(block, 0);
    
    if (block->num_successors != 2) {
        basic_block_free(block);
        return 6;
    }
    
    if (block->num_predecessors != 1) {
        basic_block_free(block);
        return 7;
    }
    
    basic_block_free(block);
    printf("  ✓ Basic block tests passed\n");
    return 0;
}

static int test_cfg(void) {
    printf("Testing CFG...\n");
    
    CFG* cfg = cfg_create("test_function");
    if (!cfg) return 1;
    
    /* Create blocks */
    BasicBlock* entry = basic_block_create(0, "ENTRY", BLOCK_ENTRY);
    BasicBlock* body = basic_block_create(1, "BODY", BLOCK_NORMAL);
    BasicBlock* exit_block = basic_block_create(2, "EXIT", BLOCK_EXIT);
    
    /* Add to CFG */
    cfg->num_blocks = 3;
    cfg->blocks = (BasicBlock**)malloc(3 * sizeof(BasicBlock*));
    cfg->blocks[0] = entry;
    cfg->blocks[1] = body;
    cfg->blocks[2] = exit_block;
    
    /* Set up edges */
    basic_block_add_successor(entry, 1);
    basic_block_add_successor(body, 2);
    basic_block_add_predecessor(body, 0);
    basic_block_add_predecessor(exit_block, 1);
    
    /* Test entry block */
    if (cfg_entry_block(cfg) != entry) {
        cfg_free(cfg);
        return 2;
    }
    
    /* Test max loop depth */
    if (cfg_max_loop_depth(cfg) != 0) {
        cfg_free(cfg);
        return 3;
    }
    
    /* Test DOT export */
    char* dot = cfg_to_dot(cfg);
    if (!dot || strlen(dot) == 0) {
        cfg_free(cfg);
        return 4;
    }
    free(dot);
    
    cfg_free(cfg);
    printf("  ✓ CFG tests passed\n");
    return 0;
}

int main(void) {
    printf("Running ciopt tests...\n\n");
    
    int failed = 0;
    
    failed += test_complexity_ranking();
    failed += test_complexity_combination();
    failed += test_config();
    failed += test_source_file();
    failed += test_basic_block();
    failed += test_cfg();
    
    printf("\n");
    if (failed == 0) {
        printf("All tests passed! ✓\n");
        return 0;
    } else {
        printf("%d test(s) failed ✗\n", failed);
        return 1;
    }
}
