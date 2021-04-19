//
// Created by schrodinger on 4/16/21.
//

#include "codegen.h"

using namespace parser;
using namespace scc;
using namespace vmips;
using namespace symtable;

std::string codegen::trim(std::string_view view) {
    auto start = view.begin();
    while (start != view.begin() &&
           (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\v' || *start == '\r')) {
        start++;
    }
    auto end = view.end();
    while (end != start &&
           (*(end - 1) == ' ' || *(end - 1) == '\t' || *(end - 1) == '\n' || *(end - 1) == '\v' ||
            *(end - 1) == '\r')) {
        end--;
    }
    return {start, end};
}

/*
 * using SelectRule = Selector<binary_lor,
            binary_land, binary_bor, binary_bxor, binary_band, binary_ne, binary_eq, binary_ge, binary_gt, binary_le, binary_lt, binary_shr, binary_shl, binary_sub, binary_add,
            binary_mod, binary_div, binary_mul, fcall, unary_not, unary_bneg, unary_neg, unary_pos, access, identifier, integer, return_statement, dowhile_statement, while_statement, ifelse_statement,
            code_block, array_assign, var_assign, var_decl, array_decl, extern_decl, function, program>;
 */

int constant(const parser::ParseTree &tree) {
    auto data = codegen::trim(tree.parsed_region);
    try {
        return std::stoi(data);
    } catch (const std::out_of_range &) {
        throw codegen::SemanticError(std::string{"integer literal out of range: "} + data);
    } catch (const std::invalid_argument &) {
        throw codegen::SemanticError(std::string{"ill-format integer literal: "} + data);
    }
}


std::shared_ptr<VirtReg>
codegen::function_generate(vmips::Function &func, const parser::ParseTree &tree,
                           SccSymTable &table) {
    CODEGEN(integer, {
        return func.append<li>(constant(tree));
    })
    CODEGEN(identifier, {
        auto data = trim(tree.parsed_region);
        auto var = table(data);
        try {
            if (var) {
                return std::get<VRegPtr>(*var);
            } else {
                throw SemanticError(std::string{"undefined variable: "} + data);
            }
        } catch (const std::bad_variant_access &) {
            throw SemanticError(std::string{"type error: "} + data);
        }
    })
    CODEGEN(array_decl, {
        auto size = constant(*tree.subtrees[1]);
        if (size <= 0) {
            throw SemanticError{std::string{"invalid array size: "} + std::to_string(size)};
        }
        auto name = trim(tree.subtrees[0]->parsed_region);
        if (table.defined_same_scope(name)) {
            throw SemanticError{std::string{"variable already defined: "} + name};
        }
        table.define(std::move(name), func.new_memory(size));
    })
    CODEGEN(var_decl, {
        auto name = trim(tree.subtrees[0]->parsed_region);
        if (table.defined_same_scope(name)) {
            throw SemanticError{std::string{"variable already defined: "} + name};
        }
        table.define(std::move(name), func.append<li>(0));
    })
    CODEGEN(var_assign, {
        auto name = trim(tree.subtrees[0]->parsed_region);
        auto target = table(name);
        auto value = function_generate(func, *tree.subtrees[1], table);
        if (target) {
            table.update(name, value);
        } else {
            throw SemanticError{std::string{"undefined variable: "} + name};
        }
    })
    CODEGEN(access, {
        auto name = trim(tree.subtrees[0]->parsed_region);
        auto target = table(name);
        auto offset = function_generate(func, *tree.subtrees[1], table);
        if (target) {
            try {
                auto data = std::get<VMemPtr>(*target);
                return func.append<array_load>(offset, data);
            } catch (const std::bad_variant_access &) {
                throw SemanticError{std::string{"variable cannot be indexed: "} + name};
            }
        } else {
            throw SemanticError{std::string{"undefined variable: "} + name};
        }

    })
    CODEGEN(array_assign, {
        auto name = trim(tree.subtrees[0]->subtrees[0]->parsed_region);
        auto target = table(name);
        auto offset = function_generate(func, *tree.subtrees[0]->subtrees[1], table);
        auto value = function_generate(func, *tree.subtrees[1], table);
        if (target) {
            try {
                auto data = std::get<VMemPtr>(*target);
                func.append_void<array_store>(value, offset, data);
            } catch (const std::bad_variant_access &) {
                throw SemanticError{std::string{"variable cannot be indexed: "} + name};
            }
        } else {
            throw SemanticError{std::string{"undefined variable: "} + name};
        }
    })


    return nullptr;
}

void codegen::function_entry(Module &module, codegen::SccSymTable &table, const ParseTree &root) {
    assert(root.instance == typeid(scc::function));
    auto func = module.create_function(trim(root.subtrees[0]->parsed_region), root.subtrees[1]->subtrees.size());
    table.enter();
    func->entry();
    for (const auto & i : root.subtrees[2]->subtrees) {
        function_generate(*func, *i, table);
    }
    table.escape();
}

codegen::SemanticError::SemanticError(std::string message) : message(std::move(message)) {}

const char *codegen::SemanticError::what() const noexcept {
    return message.c_str();
}
