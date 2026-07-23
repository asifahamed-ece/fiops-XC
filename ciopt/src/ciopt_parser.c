/**
 * @file ciopt_parser.c
 * @brief Lightweight C Parser to generate IR for CiOpt Analysis
 */

#include "ciopt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef struct {
    const char* start;
    int length;
} Token;

typedef enum {
    TOK_EOF, TOK_IDENT, TOK_NUMBER, TOK_STRING,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    TOK_SEMICOLON, TOK_COMMA, TOK_FOR, TOK_WHILE,
    TOK_IF, TOK_ELSE, TOK_RETURN, TOK_OPERATOR, TOK_UNKNOWN
} TokenType;

typedef struct {
    const char* src;
    const char* pos;
    const char* end;
    Token current;
    TokenType type;
    int line;
} Lexer;

static void skip_whitespace_and_comments(Lexer* lex) {
    while (lex->pos < lex->end) {
        if (isspace(*lex->pos)) {
            if (*lex->pos == '\n') lex->line++;
            lex->pos++;
        } else if (*lex->pos == '/' && lex->pos + 1 < lex->end) {
            if (*(lex->pos + 1) == '/') {
                while (lex->pos < lex->end && *lex->pos != '\n') lex->pos++;
            } else if (*(lex->pos + 1) == '*') {
                lex->pos += 2;
                while (lex->pos < lex->end - 1) {
                    if (*lex->pos == '*' && *(lex->pos + 1) == '/') {
                        lex->pos += 2;
                        break;
                    }
                    if (*lex->pos == '\n') lex->line++;
                    lex->pos++;
                }
            } else break;
        } else break;
    }
}

static void next_token(Lexer* lex) {
    skip_whitespace_and_comments(lex);
    if (lex->pos >= lex->end) { lex->type = TOK_EOF; return; }
    
    lex->current.start = lex->pos;
    char c = *lex->pos;

    if (isalpha(c) || c == '_') {
        while (lex->pos < lex->end && (isalnum(*lex->pos) || *lex->pos == '_')) lex->pos++;
        lex->current.length = (int)(lex->pos - lex->current.start);
        if (lex->current.length == 3 && strncmp(lex->current.start, "for", 3) == 0) lex->type = TOK_FOR;
        else if (lex->current.length == 5 && strncmp(lex->current.start, "while", 5) == 0) lex->type = TOK_WHILE;
        else if (lex->current.length == 2 && strncmp(lex->current.start, "if", 2) == 0) lex->type = TOK_IF;
        else if (lex->current.length == 4 && strncmp(lex->current.start, "else", 4) == 0) lex->type = TOK_ELSE;
        else if (lex->current.length == 6 && strncmp(lex->current.start, "return", 6) == 0) lex->type = TOK_RETURN;
        else lex->type = TOK_IDENT;
        return;
    }
    if (isdigit(c)) {
        while (lex->pos < lex->end && isalnum(*lex->pos)) lex->pos++;
        lex->current.length = (int)(lex->pos - lex->current.start);
        lex->type = TOK_NUMBER;
        return;
    }
    if (c == '"') {
        lex->pos++;
        while (lex->pos < lex->end && *lex->pos != '"') {
            if (*lex->pos == '\\') lex->pos++;
            lex->pos++;
        }
        if (lex->pos < lex->end) lex->pos++;
        lex->current.length = (int)(lex->pos - lex->current.start);
        lex->type = TOK_STRING;
        return;
    }
    lex->pos++;
    lex->current.length = 1;
    switch (c) {
        case '(': lex->type = TOK_LPAREN; break;
        case ')': lex->type = TOK_RPAREN; break;
        case '{': lex->type = TOK_LBRACE; break;
        case '}': lex->type = TOK_RBRACE; break;
        case ';': lex->type = TOK_SEMICOLON; break;
        case ',': lex->type = TOK_COMMA; break;
        default: lex->type = TOK_OPERATOR; break;
    }
}

static char* token_to_string(Token* tok) {
    char* s = (char*)malloc(tok->length + 1);
    if (!s) return NULL;
    memcpy(s, tok->start, tok->length);
    s[tok->length] = '\0';
    return s;
}

static BasicBlock* create_or_get_block(CFG* cfg, int id, BlockType type, const char* label) {
    BasicBlock* block = cfg->blocks;
    while (block) {
        if (block->id == id) return block;
        block = block->next;
    }
    block = basic_block_create(id, label, type);
    if (!cfg->blocks) cfg->blocks = block;
    else {
        BasicBlock* tail = cfg->blocks;
        while (tail->next) tail = tail->next;
        tail->next = block;
    }
    cfg->block_count++;
    return block;
}

IRFunction* ciopt_parse_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) { fprintf(stderr, "Error: Cannot open file %s\n", filename); return NULL; }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* src = (char*)malloc(fsize + 1);
    if (!src) { fclose(f); return NULL; }
    fread(src, 1, fsize, f);
    src[fsize] = '\0';
    fclose(f);

    Lexer lex = { .src = src, .pos = src, .end = src + fsize, .line = 1 };
    next_token(&lex);

    IRFunction* func = ir_function_create("parsed_function");
    CFG* cfg = cfg_create("function_cfg");
    
    BasicBlock* entry = create_or_get_block(cfg, 0, BLOCK_ENTRY, "entry");
    BasicBlock* current = entry;
    cfg->entry_block_id = 0;
    
    int block_id = 1;
    int loop_depth = 0;
    int brace_depth = 0;

    while (lex.type != TOK_EOF) {
        if (lex.type == TOK_FOR) {
            BasicBlock* header = create_or_get_block(cfg, block_id++, BLOCK_LOOP_HEADER, "loop_header");
            BasicBlock* body = create_or_get_block(cfg, block_id++, BLOCK_LOOP_BODY, "loop_body");
            
            current->successors = (int*)realloc(current->successors, (current->num_successors + 1) * sizeof(int));
            current->successors[current->num_successors++] = header->id;
            header->predecessors = (int*)realloc(header->predecessors, (header->num_predecessors + 1) * sizeof(int));
            header->predecessors[header->num_predecessors++] = current->id;
            
            IRNode* loop_node = ir_node_create(IR_LOOP_HEADER, lex.line);
            loop_node->loop_type = LOOP_FOR;
            basic_block_add_instruction(header, loop_node);
            
            next_token(&lex);
            if (lex.type == TOK_LPAREN) {
                next_token(&lex);
                while (lex.type != TOK_RPAREN && lex.type != TOK_EOF) {
                    if (lex.type == TOK_IDENT) {
                        IRNode* load = ir_node_create(IR_LOAD, lex.line);
                        load->reads = (char**)malloc(sizeof(char*));
                        load->reads[0] = token_to_string(&lex.current);
                        load->num_reads = 1;
                        basic_block_add_instruction(header, load);
                    }
                    next_token(&lex);
                }
                if (lex.type == TOK_RPAREN) next_token(&lex);
            }
            
            loop_depth++;
            header->loop_depth = loop_depth;
            body->loop_depth = loop_depth;
            current = body;
        }
        else if (lex.type == TOK_WHILE) {
            BasicBlock* header = create_or_get_block(cfg, block_id++, BLOCK_LOOP_HEADER, "while_header");
            BasicBlock* body = create_or_get_block(cfg, block_id++, BLOCK_LOOP_BODY, "while_body");
            
            current->successors = (int*)realloc(current->successors, (current->num_successors + 1) * sizeof(int));
            current->successors[current->num_successors++] = header->id;
            header->predecessors = (int*)realloc(header->predecessors, (header->num_predecessors + 1) * sizeof(int));
            header->predecessors[header->num_predecessors++] = current->id;
            
            IRNode* loop_node = ir_node_create(IR_LOOP_HEADER, lex.line);
            loop_node->loop_type = LOOP_WHILE;
            basic_block_add_instruction(header, loop_node);
            
            next_token(&lex);
            if (lex.type == TOK_LPAREN) {
                next_token(&lex);
                while (lex.type != TOK_RPAREN && lex.type != TOK_EOF) {
                    if (lex.type == TOK_IDENT) {
                        IRNode* load = ir_node_create(IR_LOAD, lex.line);
                        load->reads = (char**)malloc(sizeof(char*));
                        load->reads[0] = token_to_string(&lex.current);
                        load->num_reads = 1;
                        basic_block_add_instruction(header, load);
                    }
                    next_token(&lex);
                }
                if (lex.type == TOK_RPAREN) next_token(&lex);
            }
            
            loop_depth++;
            header->loop_depth = loop_depth;
            body->loop_depth = loop_depth;
            current = body;
        }
        else if (lex.type == TOK_IF) {
            BasicBlock* cond = create_or_get_block(cfg, block_id++, BLOCK_BRANCH_TRUE, "if_cond");
            BasicBlock* true_blk = create_or_get_block(cfg, block_id++, BLOCK_NORMAL, "if_true");
            
            current->successors = (int*)realloc(current->successors, (current->num_successors + 1) * sizeof(int));
            current->successors[current->num_successors++] = cond->id;
            cond->predecessors = (int*)realloc(cond->predecessors, (cond->num_predecessors + 1) * sizeof(int));
            cond->predecessors[cond->num_predecessors++] = current->id;
            
            IRNode* branch = ir_node_create(IR_BRANCH, lex.line);
            basic_block_add_instruction(cond, branch);
            
            next_token(&lex);
            if (lex.type == TOK_LPAREN) {
                next_token(&lex);
                while (lex.type != TOK_RPAREN && lex.type != TOK_EOF) {
                    if (lex.type == TOK_IDENT) {
                        IRNode* load = ir_node_create(IR_LOAD, lex.line);
                        load->reads = (char**)malloc(sizeof(char*));
                        load->reads[0] = token_to_string(&lex.current);
                        load->num_reads = 1;
                        basic_block_add_instruction(cond, load);
                    }
                    next_token(&lex);
                }
                if (lex.type == TOK_RPAREN) next_token(&lex);
            }
            
            current = true_blk;
        }
        else if (lex.type == TOK_ELSE) {
            next_token(&lex);
        }
        else if (lex.type == TOK_RETURN) {
            IRNode* ret = ir_node_create(IR_RETURN, lex.line);
            basic_block_add_instruction(current, ret);
            
            next_token(&lex);
            while (lex.type != TOK_SEMICOLON && lex.type != TOK_EOF) {
                if (lex.type == TOK_IDENT) {
                    IRNode* load = ir_node_create(IR_LOAD, lex.line);
                    load->reads = (char**)malloc(sizeof(char*));
                    load->reads[0] = token_to_string(&lex.current);
                    load->num_reads = 1;
                    basic_block_add_instruction(current, load);
                }
                next_token(&lex);
            }
            
            BasicBlock* exit_blk = create_or_get_block(cfg, block_id++, BLOCK_EXIT, "exit");
            current->successors = (int*)realloc(current->successors, (current->num_successors + 1) * sizeof(int));
            current->successors[current->num_successors++] = exit_blk->id;
            exit_blk->predecessors = (int*)realloc(exit_blk->predecessors, (exit_blk->num_predecessors + 1) * sizeof(int));
            exit_blk->predecessors[exit_blk->num_predecessors++] = current->id;
            cfg->num_exit_blocks++;
            cfg->exit_block_ids = (int*)realloc(cfg->exit_block_ids, cfg->num_exit_blocks * sizeof(int));
            cfg->exit_block_ids[cfg->num_exit_blocks - 1] = exit_blk->id;
            
            current = NULL;
        }
        else if (lex.type == TOK_IDENT) {
            char* name = token_to_string(&lex.current);
            next_token(&lex);
            
            if (lex.type == TOK_LPAREN) {
                IRNode* call = ir_node_create(IR_CALL, lex.line);
                call->call_target = name;
                call->call_args = 0;
                basic_block_add_instruction(current ? current : entry, call);
                
                next_token(&lex);
                while (lex.type != TOK_RPAREN && lex.type != TOK_EOF) {
                    if (lex.type == TOK_IDENT) call->call_args++;
                    next_token(&lex);
                }
                if (lex.type == TOK_RPAREN) next_token(&lex);
                if (lex.type == TOK_SEMICOLON) next_token(&lex);
            } else {
                if (current) {
                    IRNode* assign = ir_node_create(IR_ASSIGN, lex.line);
                    assign->writes = (char**)malloc(sizeof(char*));
                    assign->writes[0] = name;
                    assign->num_writes = 1;
                    basic_block_add_instruction(current, assign);
                } else free(name);
            }
        }
        else if (lex.type == TOK_LBRACE) {
            brace_depth++;
            next_token(&lex);
        }
        else if (lex.type == TOK_RBRACE) {
            brace_depth--;
            if (brace_depth < loop_depth) loop_depth = brace_depth + 1;
            next_token(&lex);
        }
        else if (lex.type == TOK_SEMICOLON) {
            next_token(&lex);
        }
        else {
            next_token(&lex);
        }
    }

    free(src);
    return func;
}

AnalysisReport* ciopt_analyze_c_file(const char* filename, const AnalysisConfig* config) {
    IRFunction* func = ciopt_parse_file(filename);
    if (!func) return NULL;

    LoopAnalysis* loops = detect_loops("", func->name);
    RecursionInfo* recursion = detect_recursion("", func->name);
    DataStructureAnalysis* ds_issues = detect_data_structure_issues("", func->name);
    DeadCodeAnalysis* dead_code = detect_dead_code("");
    PatternAnalysis* patterns = detect_patterns("", func->name);
    ComplexityResult* complexity = estimate_complexity("", func->name);

    FunctionReport* frep = function_report_create(func, (CFG*)func->cfg, complexity, loops, recursion, ds_issues, dead_code, patterns);
    FileReport* filerep = file_report_create(filename, &frep, 1);
    
    AnalysisReport* report = analysis_report_create(filename);
    analysis_report_add_file(report, filerep);
    analysis_report_finalize(report, (AnalysisConfig*)config);

    loop_analysis_free(loops);
    recursion_info_free(recursion);
    data_structure_analysis_free(ds_issues);
    dead_code_analysis_free(dead_code);
    pattern_analysis_free(patterns);
    complexity_result_free(complexity);
    ir_function_free(func);

    return report;
}
