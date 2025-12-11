/* decl.c */

#include "decl.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "encoder.h"
#include "error.h"
#include "expr.h"
#include "label.h"
#include "param_list.h"
#include "scope.h"
#include "scratch.h"
#include "stmt.h"
#include "symbol.h"
#include "type.h"

// Declarations
struct decl* decl_create(char* name, struct type* type, struct expr* value, struct stmt* code, struct decl* next) {
    struct decl* d = malloc(sizeof(*d));

    d->name = name;
    d->type = type;
    d->value = value;
    d->code = code;
    d->next = next;

    return d;
}

void decl_print(struct decl* d, int indent) {
    if (!d) {
        return;
    }

    for (int i = 0; i < indent; i++) putc('\t', stdout);
    printf("%s: ", d->name);
    type_print(d->type, stdout);

    if (d->value) {
        printf(" = ");
        expr_print(d->value, stdout);
        putc(';', stdout);
    } else if (d->code) {
        printf(" = \n");
        stmt_print(d->code, indent);
    } else {
        putc(';', stdout);
    }

    putc('\n', stdout);
    decl_print(d->next, indent);
}

void decl_resolve(struct decl* d, struct symbol* prototype) {
    if (!d) return;

    symbol_t kind = scope_level() > 1 ? SYMBOL_LOCAL : SYMBOL_GLOBAL;
    d->symbol = symbol_create(kind, d->type, d->name);
    struct symbol* binding = scope_lookup_current(d->name);
    expr_resolve(d->value);

    if (!binding) {
        /* Case 1: (LEGAL) this name has never been bound. */
        scope_bind(d->name, d->symbol);

        // Increment function symbol structure count of locals (required during codegen)
        if (prototype) {
            prototype->local_count++;
        }

        if (d->code) {
            d->symbol->func_defined = true;
            scope_enter(SCOPE_FUNCTION);
            param_list_resolve(d->type->params);
            scope_enter(SCOPE_FUNCTION);
            stmt_resolve(d->code, prototype);
            scope_exit();
            scope_exit();
        }
    } else {
        if (d->type->kind == TYPE_FUNCTION && binding->type->kind == TYPE_FUNCTION) {
            if (binding->func_defined) {
                if (d->code) {
                    /* Case 7: (ERROR) declaration is a function definition, and this name has been bound to a function definition */
                    fprintf(stderr, "resolver: competing definitions for function %s\n", d->name);
                    set_error(true);
                    // TODO: Destroy symbol.
                } else {
                    /* Case 5: (LEGAL, if types match) declaration is a function prototype, and this name has already been bound to a function definition */
                    if (type_equals(d->type, binding->type)) {
                        d->symbol = binding;
                    }
                    // TODO: Destroy symbol.
                }
            } else {
                if (d->code) {
                    /* Case 6: (LEGAL, if types match) declaration is a function definition, and this name has only been bound to a function prototype */
                    // TODO: Check if types match, destroy symbol if yes, throw error if no
                    if (type_equals(d->type, binding->type)) {
                        d->symbol = binding;
                        d->symbol->func_defined = true;
                        scope_enter(SCOPE_FUNCTION);
                        param_list_resolve(d->type->params);
                        scope_enter(SCOPE_FUNCTION);
                        stmt_resolve(d->code, prototype);
                        scope_exit();
                        scope_exit();
                    }
                    // TODO: Destroy symbol.

                } else {
                    /* Case 4: (LEGAL, if types match) declaration is a function prototype, and this name has not been bound to a function definition */
                    // TODO: Check if types match, destroy symbol if yes, throw error if no
                    d->symbol = binding;
                }
            }
        } else {
            /* Case 2: (ERROR) declaration is not a function, and this name has already been bound. */
            /* Case 3: (ERROR) declaration is a function, and this name has already been bound to something other than a function. */
            // TODO: destroy symbol
            fprintf(stderr, "resolver: redeclaration of variable %s\n", d->name);
            set_error(true);
        }
    }

    decl_resolve(d->next, prototype);
}

void decl_typecheck(struct decl* d) {
    if (!d) return;

    /**
     * The following “oddities” should be flagged as errors, even though it is possible to express them syntactically:
     * Functions that return arrays or other functions.
     * Functions as parameters to other functions.
     * Arrays of functions.
     * Non-constant initializers for global variables.
     * Array initializers for local variables.
     */

    if (d->symbol->kind == SYMBOL_GLOBAL) {
        // Prevent non-constant initializers for global variables
        switch (d->type->kind) {
            case TYPE_ARRAY:
            case TYPE_CARRAY:
                if (d->type->array_size->kind != EXPR_INT) {
                    fprintf(stderr, "typecheck error: global array %s cannot have non-constant size (", d->name);
                    expr_print(d->type->array_size, stderr);
                    fprintf(stderr, ")\n");
                    set_error(true);
                }
                break;
            case TYPE_AUTO:
            case TYPE_BOOLEAN:
            case TYPE_CHARACTER:
            case TYPE_INTEGER:
            case TYPE_DOUBLE:
            case TYPE_STRING:
                if (d->value && d->value->kind < EXPR_NAME) {
                    fprintf(stderr, "typecheck error: global variable %s cannot have non-constant initializer (", d->name);
                    expr_print(d->value, stderr);
                    fprintf(stderr, ")\n");
                    set_error(true);
                }
                break;
            default:
                // Shoo, gcc errors!
                break;
        }
    }

    // Check for size 0 arrays
    if (d->type->kind == TYPE_ARRAY || d->type->kind == TYPE_CARRAY) {
        if (d->type->array_size->kind == EXPR_INT && d->type->array_size->int_literal == 0) {
            fprintf(stderr, "typecheck error: array %s cannot have size of 0 (", d->name);
            expr_print(d->type->array_size, stderr);
            fprintf(stderr, ")\n");
            set_error(true);
        }
    }

    if (d->value) {
        struct type* t = expr_typecheck(d->value);

        /**
         * Infer type of symbol from initializing expression. Do not allow auto to be initialized with auto.
         */
        if (d->type->kind == TYPE_AUTO) {
            if (t->kind == TYPE_AUTO) {
                fprintf(stderr, "typecheck error: auto (%s) cannot be initialized with auto (", d->name);
                expr_print(d->value, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            } else if (t->kind == TYPE_VOID) {
                fprintf(stderr, "typecheck error: auto (%s) cannot be initialized with void (", d->name);
                expr_print(d->value, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            } else {
                type_delete(d->type);
                d->type = type_copy(t);
            }
        }

        if (d->type->kind == TYPE_ARRAY || d->type->kind == TYPE_CARRAY) {
            if (d->symbol->kind == SYMBOL_LOCAL) {
                fprintf(stderr, "typecheck error: cannot initialize local array %s\n", d->name);
                set_error(true);
            } else if (d->type->subtype->kind == TYPE_AUTO) {
                type_delete(d->type->subtype);
                d->type->subtype = type_copy(t);
            } else if (!type_equals(t->subtype, d->symbol->type)) {
                fprintf(stderr, "typecheck error: decl of type ");
                type_print(d->type, stderr);
                fprintf(stderr, " (%s) cannot be initialized with value of type ", d->symbol->name);
                type_print(t->subtype, stderr);
                fprintf(stderr, " (");
                expr_print(d->value, stderr);
                fprintf(stderr, ")\n");
                set_error(true);
            }
        } else if (!type_equals(t, d->symbol->type)) {
            fprintf(stderr, "typecheck error: decl of type ");
            type_print(d->type, stderr);
            fprintf(stderr, " (%s) cannot be initialized with value of type ", d->symbol->name);
            type_print(t, stderr);
            fprintf(stderr, " (");
            expr_print(d->value, stderr);
            fprintf(stderr, ")\n");
            set_error(true);
        }
    }

    if (d->code) {
        stmt_typecheck(d->code, d->symbol);
        switch (d->type->subtype->kind) {
            case TYPE_AUTO:
                fprintf(stderr, "typecheck error: cannot infer type of automatically typed function (%s) since it has no return statement.\n", d->name);
                set_error(true);
                break;
            case TYPE_ARRAY:
            case TYPE_CARRAY:
            case TYPE_FUNCTION:
                fprintf(stderr, "typecheck error: cannot return %s type from function (%s).\n", ttos(d->type->subtype), d->name);
                set_error(true);
                break;
            default:
                // Shoo, gcc warnings!
                break;
        }
    }

    decl_typecheck(d->next);
}

void decl_codegen(struct decl* d, FILE* outfile) {
    if (!d) return;
    char encoded_string[256] = {0};
    struct expr* array_elem = NULL;
    int label;
    int stack_offset;

    if (d->symbol->kind == SYMBOL_GLOBAL) {
        switch (d->type->kind) {
            case TYPE_FUNCTION:
                if (d->code) {
                    // Global function definition
                    fprintf(outfile, ".text\n");
                    fprintf(outfile, ".global %s\n", d->name);
                    fprintf(outfile, "%s:\n", d->name);
                    fprintf(outfile, "# ** Preamble for function %s **\n", d->name);

                    // Save base pointer and set base pointer to stack pointer
                    fprintf(outfile, "PUSH %%rbp\n");
                    fprintf(outfile, "MOVQ %%rsp, %%rbp\n");

                    /**
                     * The stack needs to be 16 byte aligned to support the System V ABI calling convention.
                     * See chapter "The Stack Frame" https://refspecs.linuxbase.org/elf/x86_64-abi-0.99.pdf
                     * Basically, you need (parameters + locals + callee saved registers + 3) to be EVEN. The 3 at the end represents rbp, rsp, and rip.
                     * Since the number of callee saved registers is even, and in this implementation we always push all 6 argument registers, and we always add the extra three for the stack, base, and instruction pointers, we should add an extra 8 bytes of padding on the stack if the number of locals is EVEN.
                     */
                    if (d->symbol->local_count % 2 == 0) {
                        // Need 8 bytes padding.
                        fprintf(outfile, "SUBQ $%d, %%rsp\n", 8 * (d->symbol->local_count + 1));
                    } else {
                        // Already 16 byte aligned.
                        fprintf(outfile, "SUBQ $%d, %%rsp\n", 8 * (d->symbol->local_count));
                    }

                    // Push callee saved registers
                    fprintf(outfile, "PUSH %%rbx\n");
                    fprintf(outfile, "PUSH %%r12\n");
                    fprintf(outfile, "PUSH %%r13\n");
                    fprintf(outfile, "PUSH %%r14\n");
                    fprintf(outfile, "PUSH %%r15\n");

                    fprintf(outfile, "# ** START Body for function %s **\n", d->name);
                    stmt_codegen(d->code, d->symbol, outfile);
                    fprintf(outfile, "# ** END Body for function %s **\n", d->name);
                    fprintf(outfile, "# ** START Epilogue for function %s **\n", d->name);
                    // Restore callee saved register, move base pointer into stack pointer, pop base pointer from stack
                    fprintf(outfile, "%s_epilogue:\n", d->symbol->name);
                    fprintf(outfile, "POP %%r15\n");
                    fprintf(outfile, "POP %%r14\n");
                    fprintf(outfile, "POP %%r13\n");
                    fprintf(outfile, "POP %%r12\n");
                    fprintf(outfile, "POP %%rbx\n");
                    fprintf(outfile, "LEAVE\n");
                    fprintf(outfile, "RET\n");
                    fprintf(outfile, "# ** END Epilogue for function %s **\n", d->name);
                }
                break;
            // Global variable declaration
            case TYPE_BOOLEAN:
                fprintf(outfile, ".data\n");
                if (d->value) {
                    fprintf(outfile, "%s: .quad %ld\n", d->name, d->value->bool_literal ? 1L : 0L);
                } else {
                    fprintf(outfile, "%s: .quad 0\n", d->name);
                }
                break;
            case TYPE_CHARACTER:
                fprintf(outfile, ".data\n");
                if (d->value) {
                    char_encode(d->value->char_literal, encoded_string);
                    fprintf(outfile, "%s: .quad %s\n", d->name, encoded_string);
                } else {
                    fprintf(outfile, "%s: .quad 0\n", d->name);
                }
                break;
            case TYPE_INTEGER:
                fprintf(outfile, ".data\n");
                if (d->value) {
                    fprintf(outfile, "%s: .quad %ld\n", d->name, d->value->int_literal);
                } else {
                    fprintf(outfile, "%s: .quad 0\n", d->name);
                }
                break;
            case TYPE_DOUBLE:
                fprintf(stderr, "codegen: floating points are not supported.\n");
                exit(EXIT_FAILURE);
            case TYPE_STRING:
                /* A bit of weirdness with global strings. Consider the case where a global string is uninitialized. Here, we say the default value of this string is an empty string. But what if, later in the program, the user assigns a new value to this global string? We cannot write the contents of global string since it is in the data segment. Instead, we will sling some pointers around. When we LEAQ on this symbol in the future, we will get a pointer which we can modify. */
                fprintf(outfile, ".data\n");
                if (d->value) {
                    string_encode(d->value->string_literal, encoded_string);
                    label = label_create();
                    fprintf(outfile, "%s: .string %s\n", label_name(label), encoded_string);
                    fprintf(outfile, "%s: .quad %s\n", d->name, label_name(label));
                } else {
                    fprintf(outfile, "%s: .quad 0\n", d->name);
                }
                break;
            case TYPE_ARRAY:
            case TYPE_CARRAY:
                fprintf(outfile, ".data\n");
                switch (d->type->subtype->kind) {
                    case TYPE_ARRAY:
                    case TYPE_CARRAY:
                    case TYPE_DOUBLE:
                        fprintf(stderr, "codegen: array with elements of type %s not supported.\n", ttos(d->type->subtype));
                        exit(EXIT_FAILURE);
                        break;
                    case TYPE_STRING:
                        // 1. Produce indirect labels

                        if (d->value) {
                            array_elem = d->value->left;
                            for (int i = 0; i < d->type->array_size->int_literal; i++) {
                                if (i < 1) {
                                    label = label_create();
                                    string_encode(array_elem->left->string_literal, encoded_string);
                                    fprintf(outfile, "%s: .string %s\n", label_name(label), encoded_string);
                                } else {
                                    label_create();
                                    string_encode(array_elem->left->string_literal, encoded_string);
                                    fprintf(outfile, "%s: .string %s\n", label_name(label + i), encoded_string);
                                }
                                array_elem = array_elem->right;
                            }

                            fprintf(outfile, "%s: .quad ", d->name);
                            if (d->type->kind == TYPE_ARRAY) {
                                fprintf(outfile, "%ld, ", d->type->array_size->int_literal);
                            }
                            for (int i = 0; i < d->type->array_size->int_literal; i++) {
                                if (i != 0) {
                                    fprintf(outfile, ", ");
                                }
                                fprintf(outfile, "%s", label_name(label + i));
                            }
                            putc('\n', outfile);
                        } else {
                            // If array initializer is omitted, fill with zeroes. Still need to prefix array with length if it is a bminor array.
                            fprintf(outfile, "%s: .quad ", d->name);
                            if (d->type->kind == TYPE_ARRAY) {
                                fprintf(outfile, "%ld, ", d->type->array_size->int_literal);
                            }
                            for (int i = 0; i < d->type->array_size->int_literal; i++) {
                                if (i != 0) {
                                    fprintf(outfile, ", ");
                                }
                                putc('0', outfile);
                            }
                            putc('\n', outfile);
                        }
                        break;
                    default:
                        fprintf(outfile, "%s: .quad ", d->name);
                        if (d->type->kind == TYPE_ARRAY) {
                            fprintf(outfile, "%ld, ", d->type->array_size->int_literal);
                        }
                        if (d->value) {
                            array_elem = d->value->left;
                            for (int i = 0; i < d->type->array_size->int_literal; i++) {
                                if (i != 0) {
                                    fprintf(outfile, ", ");
                                }
                                switch (d->type->subtype->kind) {
                                    case TYPE_INTEGER:
                                        fprintf(outfile, "%ld", array_elem->left->int_literal);
                                        break;
                                    case TYPE_BOOLEAN:
                                        fprintf(outfile, "%ld", array_elem->left->bool_literal ? 1L : 0L);
                                        break;
                                    case TYPE_CHARACTER:
                                        char_encode(array_elem->left->char_literal, encoded_string);
                                        fprintf(outfile, "%s", encoded_string);
                                        break;
                                }
                                array_elem = array_elem->right;
                            }
                        } else {
                            for (int i = 0; i < d->type->array_size->int_literal; i++) {
                                if (i != 0) {
                                    fprintf(outfile, ", ");
                                }
                                putc('0', outfile);
                            }
                        }
                        putc('\n', outfile);
                }
                break;
        }
    } else {
        // Guard unimplemented stuff
        switch (d->type->kind) {
            case TYPE_FUNCTION:
                // Local function declaration
                fprintf(stderr, "codegen: local functions are not supported.\n");
                exit(EXIT_FAILURE);
            case TYPE_ARRAY:
            case TYPE_CARRAY:
                fprintf(stderr, "codegen: local array declarations are not supported.\n");
                exit(EXIT_FAILURE);
            case TYPE_DOUBLE:
                fprintf(stderr, "codegen: floating points are not supported.\n");
                exit(EXIT_FAILURE);
            default:
                break;
        }

        // If local declaration is uninitialized, we just dont do anything.
        if (!d->value) goto decl_codegen_next;

        stack_offset = 8 * (d->symbol->which + 1);
        expr_codegen(d->value, outfile);
        fprintf(outfile, "MOVQ %s, -%d(%%rbp)\n", scratch_name(d->value->reg), stack_offset);
        scratch_free(d->value->reg);
    }

decl_codegen_next:
    decl_codegen(d->next, outfile);
}