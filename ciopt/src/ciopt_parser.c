/**
 * @file ciopt_parser.c
 * @brief Lightweight C Parser to generate IR for CiOpt Analysis
 * 
 * This parser tokenizes C code to identify functions, loops, branches,
 * and function calls, populating the IR structures defined in ciopt.h.
 */

#include "ciopt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// --- Internal Helper Structures ---

typedef struct {
    const char* start;
    int length;
} Token;

typedef enum {
    TOK_EOF,
    TOK_IDENT,
    TOK_NUMBER,
    TOK_STRING,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMICOLON,
    TOK_COMMA,
    TOK_FOR,
    TOK_WHILE,
    TOK_IF,
    TOK_ELSE,
    TOK_RETURN,
    TOK_OPERATOR,
    TOK_UNKNOWN
} TokenType;

typedef struct {
    const char* src;
    const char* pos;
    const char* end;
    Token current;
    TokenType type;
    int line;
} Lexer;

// --- Lexer Implementation ---

static void skip_whitespace_and_comments(Lexer* lex) {
    while (lex->pos < lex->end) {
        if (isspace(*lex->pos)) {
            if (*lex->pos == '\n') lex->line++;
            lex->pos++;
        } else if (*lex->pos == '/' && lex->pos + 1 < lex->end) {
            if (*(lex->pos + 1) == '/') {
                // Single line comment
                while (lex->pos < lex->end && *lex->pos != '\n') lex->pos++;
            } else if (*(lex->pos + 1) == '*') {
                // Multi-line comment
                lex->pos += 2;
                while (lex->pos < lex->end - 1) {
                    if (*lex->pos == '*' && *(lex->pos + 1) == '/') {
                        lex->pos += 2;
                        break;
                    }
                    if (*lex->pos == '\n') lex->line++;
                    lex->pos++;
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

static void next_token(Lexer* lex) {
    skip_whitespace_and_comments(lex);
    if (lex->pos >= lex->end) {
        lex->type = TOK_EOF;
        return;
    }

    lex->current.start = lex->pos;
    char c = *lex->pos;

    if (isalpha(c) || c == '_') {
        while (lex->pos < lex->end && (isalnum(*lex->pos) || *lex->pos == '_')) {
            lex->pos++;
        }
        lex->current.length = (int)(lex->pos - lex->current.start);
        
        // Check keywords
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

    // Punctuation
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

// --- Parser Logic ---

static IRNode* create_node_from_token(Lexer* lex, IRNodeType type) {
    IRNode* node = ir_node_create(type);
    if (lex->type == TOK_IDENT || lex->type == TOK_NUMBER || lex->type == TOK_STRING) {
        char* val = malloc(lex->current.length + 1);
        strncpy(val, lex->current.start, lex->current.length);
        val[lex->current.length] = '\0';
        node->value = val;
        node->has_value = true;
    }
    node->line = lex->line;
    return node;
}

// Forward declaration
static IRNodeList* parse_block(Lexer* lex, IRNodeList* parent_list);

static IRNode* parse_statement(Lexer* lex) {
    IRNode* node = NULL;

    if (lex->type == TOK_FOR) {
        node = create_node_from_token(lex, IR_LOOP_FOR);
        next_token(lex);
        if (lex->type == TOK_LPAREN) {
            next_token(lex);
            // Simplified: Treat condition as a single block until ')'
            while (lex->type != TOK_RPAREN && lex->type != TOK_EOF) {
                if (lex->type == TOK_IDENT) {
                    IRNode* var = create_node_from_token(lex, IR_LOAD);
                    ir_node_add_child(node, var);
                }
                next_token(lex);
            }
            if (lex->type == TOK_RPAREN) next_token(lex);
        }
        // Parse body
        if (lex->type == TOK_LBRACE) {
            next_token(lex);
            node->body = parse_block(lex, node->children);
            if (lex->type == TOK_RBRACE) next_token(lex);
        }
        return node;
    } 
    else if (lex->type == TOK_WHILE) {
        node = create_node_from_token(lex, IR_LOOP_WHILE);
        next_token(lex);
        if (lex->type == TOK_LPAREN) {
            next_token(lex);
            while (lex->type != TOK_RPAREN && lex->type != TOK_EOF) {
                if (lex->type == TOK_IDENT) {
                    IRNode* var = create_node_from_token(lex, IR_LOAD);
                    ir_node_add_child(node, var);
                }
                next_token(lex);
            }
            if (lex->type == TOK_RPAREN) next_token(lex);
        }
        if (lex->type == TOK_LBRACE) {
            next_token(lex);
            node->body = parse_block(lex, node->children);
            if (lex->type == TOK_RBRACE) next_token(lex);
        }
        return node;
    }
    else if (lex->type == TOK_IF) {
        node = create_node_from_token(lex, IR_BRANCH_IF);
        next_token(lex);
        if (lex->type == TOK_LPAREN) {
            next_token(lex);
            while (lex->type != TOK_RPAREN && lex->type != TOK_EOF) {
                if (lex->type == TOK_IDENT) {
                    IRNode* var = create_node_from_token(lex, IR_LOAD);
                    ir_node_add_child(node, var);
                }
                next_token(lex);
            }
            if (lex->type == TOK_RPAREN) next_token(lex);
        }
        if (lex->type == TOK_LBRACE) {
            next_token(lex);
            node->body = parse_block(lex, node->children);
            if (lex->type == TOK_RBRACE) next_token(lex);
        }
        // Handle else
        if (lex->type == TOK_ELSE) {
            next_token(lex);
            if (lex->type == TOK_LBRACE) {
                next_token(lex);
                node->else_body = parse_block(lex, node->children);
                if (lex->type == TOK_RBRACE) next_token(lex);
            }
        }
        return node;
    }
    else if (lex->type == TOK_RETURN) {
        node = create_node_from_token(lex, IR_RETURN);
        next_token(lex);
        if (lex->type != TOK_SEMICOLON && lex->type != TOK_EOF) {
            // Parse return expression simplified
            if (lex->type == TOK_IDENT) {
                IRNode* val = create_node_from_token(lex, IR_LOAD);
                ir_node_add_child(node, val);
            }
            while (lex->type != TOK_SEMICOLON && lex->type != TOK_EOF) {
                next_token(lex);
            }
        }
        if (lex->type == TOK_SEMICOLON) next_token(lex);
        return node;
    }
    else if (lex->type == TOK_IDENT) {
        // Could be assignment or function call
        Token name = lex->current;
        next_token(lex);
        
        if (lex->type == TOK_LPAREN) {
            // Function Call
            node = ir_node_create(IR_CALL);
            char* fname = malloc(name.length + 1);
            strncpy(fname, name.start, name.length);
            fname[name.length] = '\0';
            node->value = fname;
            node->has_value = true;
            node->line = lex->line;
            
            next_token(lex); // consume (
            while (lex->type != TOK_RPAREN && lex->type != TOK_EOF) {
                if (lex->type == TOK_IDENT) {
                    IRNode* arg = create_node_from_token(lex, IR_LOAD);
                    ir_node_add_child(node, arg);
                }
                next_token(lex);
            }
            if (lex->type == TOK_RPAREN) next_token(lex);
            if (lex->type == TOK_SEMICOLON) next_token(lex);
            return node;
        } else {
            // Assignment or Expression
            node = ir_node_create(IR_ASSIGN);
            char* vname = malloc(name.length + 1);
            strncpy(vname, name.start, name.length);
            vname[name.length] = '\0';
            node->value = vname;
            node->has_value = true;
            node->line = lex->line;
            
            // Skip until semicolon for simplicity in this lightweight parser
            while (lex->type != TOK_SEMICOLON && lex->type != TOK_EOF) {
                if (lex->type == TOK_IDENT) {
                    IRNode* rhs = create_node_from_token(lex, IR_LOAD);
                    ir_node_add_child(node, rhs);
                }
                next_token(lex);
            }
            if (lex->type == TOK_SEMICOLON) next_token(lex);
            return node;
        }
    }

    // Default: skip unknown statement
    while (lex->type != TOK_SEMICOLON && lex->type != TOK_LBRACE && lex->type != TOK_EOF) {
        next_token(lex);
    }
    if (lex->type == TOK_SEMICOLON) next_token(lex);
    
    return NULL;
}

static IRNodeList* parse_block(Lexer* lex, IRNodeList* parent_list) {
    IRNodeList* list = ir_node_list_create();
    
    while (lex->type != TOK_RBRACE && lex->type != TOK_EOF) {
        IRNode* stmt = parse_statement(lex);
        if (stmt) {
            ir_node_list_add(list, stmt);
            if (parent_list) {
                ir_node_list_add(parent_list, stmt);
            }
        }
    }
    return list;
}

// --- Public API Implementation ---

IRFunction* ciopt_parse_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* src = malloc(fsize + 1);
    fread(src, 1, fsize, f);
    src[fsize] = '\0';
    fclose(f);

    Lexer lex = { .src = src, .pos = src, .end = src + fsize, .line = 1 };
    next_token(&lex);

    IRFunction* func = ir_function_create("main_parsed"); // Default name if not detected
    IRNodeList* global_body = ir_node_list_create();
    func->body = global_body;

    // Simple heuristic: Treat the whole file as one block or try to find functions
    // For this lightweight version, we parse top-level statements into the function body
    // A real parser would detect 'void foo() {' and create new IRFunction objects.
    
    while (lex.type != TOK_EOF) {
        if (lex.type == TOK_IDENT) {
            Token name = lex.current;
            next_token(&lex);
            if (lex.type == TOK_LPAREN) {
                // Detected a function definition
                if (func->body->count > 0) {
                    // If we already parsed something, assume previous was a function or global
                    // For simplicity, we just keep adding to the main function in this demo
                    // or we could create a new one. Let's stick to one big function for demo stability.
                }
                
                // Update function name
                free(func->name);
                func->name = malloc(name.length + 1);
                strncpy(func->name, name.start, name.length);
                func->name[name.length] = '\0';
                
                next_token(&lex); // consume (
                // Skip args
                while (lex.type != TOK_RPAREN && lex.type != TOK_EOF) next_token(&lex);
                if (lex.type == TOK_RPAREN) next_token(&lex);
                
                if (lex.type == TOK_LBRACE) {
                    next_token(&lex);
                    IRNodeList* body = parse_block(&lex, func->body);
                    if (lex.type == TOK_RBRACE) next_token(&lex);
                }
            }
        } else {
            next_token(&lex);
        }
    }

    free(src);
    return func;
}

AnalysisReport* ciopt_analyze_c_file(const char* filename, const CiOptConfig* config) {
    IRFunction* func = ciopt_parse_file(filename);
    if (!func) {
        return NULL;
    }

    // Build CFG
    CFG* cfg = cfg_create(func);
    if (!cfg) {
        ir_function_free(func);
        return NULL;
    }

    // Run Analyses
    LoopAnalysis* loops = detect_loops(cfg);
    RecursionInfo* recursion = detect_recursion(func);
    DataStructureAnalysis* ds_issues = detect_data_structure_issues(func);
    DeadCodeAnalysis* dead_code = detect_dead_code(cfg);
    PatternAnalysis* patterns = detect_patterns(func);
    ComplexityResult* complexity = estimate_complexity(cfg, loops, recursion);

    // Generate Report
    FunctionReport* frep = function_report_create(func, cfg, complexity, loops, recursion, ds_issues, dead_code, patterns);
    FileReport* filerep = file_report_create(filename, &frep, 1); // 1 function for now
    
    AnalysisReport* report = analysis_report_create(filename);
    analysis_report_add_file(report, filerep);
    analysis_report_finalize(report, config);

    // Cleanup intermediate structures (report holds copies or owns data)
    // Note: In a full implementation, we need careful ownership management.
    // Here we assume report_create takes ownership or copies necessary data.
    // For safety in this demo, we free the raw analysis structs if report doesn't own them.
    // (Assuming report functions handle memory correctly as per header design)
    // NOTE: We don't free frep here because file_report_create took ownership
    
    loop_analysis_free(loops);
    recursion_info_free(recursion);
    /* ds_issues, dead_code, patterns already transferred to frep */
    complexity_result_free(complexity);
    /* cfg already transferred to frep */
    ir_function_free(func);

    return report;
}
