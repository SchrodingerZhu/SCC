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
    while (start != view.end() &&
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
            code_block, array_assign, var_assign, var_decl, array_decl, function, program>;
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
codegen::function_generate(EFuncFactory &factory, vmips::Function &func, const parser::ParseTree &tree,
                           SccSymTable &table, NodePtr &node) {
    CODEGEN(binary_mul, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        return func.append<mul>(a, b);
    })
    CODEGEN(binary_land, {
        auto left = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto sections = func.branch<beqz>(left);

        auto right = function_generate(factory, func, *tree.subtrees[1], table, sections.first);
        auto right_result = func.append<sltu>(get_special(SpecialReg::zero), right);

        func.switch_to(sections.second);
        auto left_result = func.append<move>(get_special(SpecialReg::zero));

        node = func.join(sections.first, sections.second);
        func.add_phi(left_result, right_result);

        return left_result;
    })
    CODEGEN(binary_lor, {
        auto left = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto sections = func.branch<bnez>(left);

        auto right = function_generate(factory, func, *tree.subtrees[1], table, sections.first);
        auto right_result = func.append<sltu>(get_special(SpecialReg::zero), right);

        func.switch_to(sections.second);
        auto left_result = func.append<li>(1);

        node = func.join(sections.first, sections.second);
        func.add_phi(left_result, right_result);

        return left_result;
    })
    CODEGEN(binary_band, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        return func.append<vmips::band>(a, b);
    })
    CODEGEN(binary_bor, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        return func.append<vmips::bor>(a, b);
    })
    CODEGEN(binary_bxor, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        return func.append<vmips::bxor>(a, b);
    })
    CODEGEN(binary_div, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        func.append_void<vmips::div>(a, b);
        return func.append<mflo>();
    })
    CODEGEN(binary_mod, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        func.append_void<vmips::div>(a, b);
        return func.append<mfhi>();
    })
    CODEGEN(binary_add, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        return func.append<vmips::add>(a, b);
    })
    CODEGEN(binary_sub, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        return func.append<vmips::sub>(a, b);
    })
    CODEGEN(binary_shl, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        return func.append<vmips::sllv>(a, b);
    })
    CODEGEN(binary_shr, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        return func.append<vmips::srlv>(a, b);
    })
    CODEGEN(binary_lt, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        auto ret = func.append<vmips::slt>(a, b);
        return ret;
    })
    CODEGEN(binary_eq, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        auto c = func.append<vmips::bxor>(a, b);
        auto ret = func.append<sltiu>(c, 1);
        return ret;
    })
    CODEGEN(binary_gt, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        auto ret = func.append<vmips::slt>(b, a);
        return ret;
    })
    CODEGEN(binary_ne, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        auto c = func.append<vmips::bxor>(a, b);
        auto ret = func.append<sltu>(get_special(SpecialReg::zero), c);
        return ret;
    })
    CODEGEN(binary_le, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        auto c = func.append<vmips::slt>(b, a);
        auto ret = func.append<xori>(c, 1);
        return ret;
    })
    CODEGEN(binary_ge, {
        auto a = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto b = function_generate(factory, func, *tree.subtrees[1], table, node);
        auto c = func.append<vmips::slt>(a, b);
        auto ret = func.append<xori>(c, 1);
        return ret;
    })
    CODEGEN(integer, {
        return func.append<li>(constant(tree));
    })
    CODEGEN(return_statement, {
        func.assign_special(SpecialReg::v0, 0);
        func.add_ret();
    })
    CODEGEN(unary_pos, {
        return function_generate(factory, func, *tree.subtrees[0], table, node);
    })
    CODEGEN(unary_not, {
        auto term = function_generate(factory, func, *tree.subtrees[0], table, node);
        auto ret = func.append<sltiu>(term, 1);
        return ret;
    })
    CODEGEN(unary_neg, {
        auto term = function_generate(factory, func, *tree.subtrees[0], table, node);
        return func.append<negu>(term);
    })
    CODEGEN(unary_bneg, {
        auto term = function_generate(factory, func, *tree.subtrees[0], table, node);
        return func.append<bnot>(term);
    })
    CODEGEN(while_statement, {
        auto body = func.new_section();
        auto start = body;
        auto cond = function_generate(factory, func, *tree.subtrees[0], table, body);
        auto outside = func.new_section();
        func.switch_to(body);
        func.branch_existing<beqz>(outside, cond);
        func.switch_to(body);
        function_generate(factory, func, *tree.subtrees[1], table, body);
        func.branch_existing<j>(start);
        func.switch_to(outside);
        node = func.new_section();
    })
    CODEGEN(fcall, {
        auto name = trim(tree.subtrees[0]->parsed_region);
        auto function = factory.get_or_create(name, tree.subtrees.size() - 1);
        std::vector<VRegPtr> args;
        for (auto i = tree.subtrees.begin() + 1; i != tree.subtrees.end(); ++i) {
            args.push_back(function_generate(factory, func, **i, table, node));
        };
        return func.call(function, std::move(args));
    })
    CODEGEN(dowhile_statement, {
        auto body = func.new_section();
        NodePtr _tmp = body;
        function_generate(factory, func, *tree.subtrees[0], table, _tmp);
        NodePtr now = func.cursor;
        auto cond = function_generate(factory, func, *tree.subtrees[1], table, now);
        func.branch_existing<bnez>(body, cond);
        func.switch_to(now);
        node = now;
    })
    CODEGEN(code_block, {
        table.enter();
        for (const auto &i : tree.subtrees) {
            function_generate(factory, func, *i, table, node);
        }
        table.escape();
    })
    CODEGEN(ifelse_statement, {
        assert(tree.subtrees[0]->instance == typeid(if_stmt));
        auto cond = function_generate(factory, func, *tree.subtrees[0]->subtrees[0], table, node);
        auto sections = func.branch<bnez>(cond);
        // false
        if (tree.subtrees.size() == 1) {
            func.append_void<text>("# empty else branch");
        } else {
            assert(tree.subtrees[1]->instance == typeid(else_stmt));
            function_generate(factory, func, *tree.subtrees[1]->subtrees[0], table, sections.first);
        }
        // true
        func.switch_to(sections.second);
        function_generate(factory, func, *tree.subtrees[0]->subtrees[1], table, sections.second);
        // after
        node = func.join(sections.first, sections.second);
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
            if (var->index() == 1) {
                throw SemanticError(std::string{"type error: "} + data);
            } else {
                throw SemanticError(std::string{"target uninitialized: "} + data);
            }
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
        table.define(std::move(name), func.new_memory(size * 4));
    })
    CODEGEN(var_decl, {
        auto name = trim(tree.subtrees[0]->parsed_region);
        if (table.defined_same_scope(name)) {
            throw SemanticError{std::string{"variable already defined: "} + name};
        }
        table.define(std::move(name), UninitializedValue{});
    })
    CODEGEN(var_assign, {
        auto name = trim(tree.subtrees[0]->parsed_region);
        auto target = table(name);
        auto value = function_generate(factory, func, *tree.subtrees[1], table, node);
        if (target && target->index() == 0) {
            table.update(name, value);
            func.add_phi(std::get<VRegPtr>(*target), value); // merge liveness
        } else if (target->index() == 2) {
            table.update(name, value);
        } else if (target) {
            throw SemanticError{std::string{"target variable is not assignable: "} + name};
        } else {
            throw SemanticError{std::string{"undefined variable: "} + name};
        }
    })
    CODEGEN(access, {
        auto name = trim(tree.subtrees[0]->parsed_region);
        auto target = table(name);
        auto offset = function_generate(factory, func, *tree.subtrees[1], table, node);
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
        auto offset = function_generate(factory, func, *tree.subtrees[0]->subtrees[1], table, node);
        auto value = function_generate(factory, func, *tree.subtrees[1], table, node);
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

    END:
    return nullptr;
}

vmips::Module codegen::codegen(const ParseTree &root) {
    assert(root.instance == typeid(scc::program));
    auto table = SccSymTable{};
    auto module = vmips::Module{"program"};
    auto factory = EFuncFactory{
            .module = &module,
            .cache = {}
    };
    configure_builtin(module, factory);
    auto func = module.create_function("main", 3);
    func->append_void<text>(".set noat");
    table.enter();
    auto bottom = func->cursor;
    for (const auto &statement : root.subtrees) {
        function_generate(factory, *func, *statement, table, bottom);
    }
    func->append_void<text>(".set at");
    func->assign_special(SpecialReg::v0, 0);
    table.escape();
    module.finalize();
    return module;
}

codegen::SemanticError::SemanticError(std::string message) : message(std::move(message)) {}

const char *codegen::SemanticError::what() const noexcept {
    return message.c_str();
}

codegen::FuncPtr codegen::EFuncFactory::get_or_create(std::string_view name, size_t argc) {
    if (cache.contains(name)) {
        return cache.at(name);
    }
    auto func = module->create_extern({name.begin(), name.end()}, argc);
    cache.insert(std::make_pair(name, func));
    return func;
}
