#include "codegen.h"
VRegPtr Constant::codegen(vmips::Function &function, SymTable &table, FunctionTable &ftable) {
    return function.append<vmips::li>(value);
}

VRegPtr WhileNode::codegen(vmips::Function &function, SymTable &table, FunctionTable &ftable) {
    auto block = function.new_section();
    auto condition = cond->codegen(function, table, ftable);
    auto after = function.new_section_branch<vmips::beqz>(condition);
    function.switch_to(block);

    table.enter();
    main_body->codegen(function, table, ftable);
    auto updates = table.local_updates<phmap::flat_hash_map<std::string, Variable>>();
    table.escape();

    function.switch_to(after);

    // unify local updates with previous registers
    for (auto & i : updates) {
        if (i.second.reg) {
            function.add_phi(table(i.first)->reg, i.second.reg);
        }
    }
    return nullptr;
}

VRegPtr IFNode::codegen(vmips::Function &function, SymTable &table, FunctionTable &ftable) {
    auto condition = cond->codegen(function, table, ftable);
    auto blocks = function.branch<vmips::beqz>(condition);

    table.enter();
    main_body->codegen(function, table, ftable);
    auto lv1 = table.local_updates<phmap::flat_hash_map<std::string, Variable>>();
    table.escape();

    // unify registers
    for (auto & i : lv1) {
        if (i.second.reg) {
            function.add_phi(table(i.first)->reg, i.second.reg);
        }
    }


    if (else_body) {
        function.switch_to(blocks.second);

        table.enter();
        else_body->codegen(function, table, ftable);
        auto lv2 = table.local_updates<phmap::flat_hash_map<std::string, Variable>>();
        table.escape();

        function.join(blocks.first, blocks.second);
        // unify registers
        for (auto & i : lv2) {
            if (i.second.reg) {
                function.add_phi(table(i.first)->reg, i.second.reg);
            }
        }

    }

    return nullptr;
}

VRegPtr DoWhileNode::codegen(vmips::Function &function, SymTable &table, FunctionTable &ftable) {
    auto block = function.new_section();
    table.enter();
    main_body->codegen(function, table, ftable);
    auto updates = table.local_updates<phmap::flat_hash_map<std::string, Variable>>();
    table.escape();

    // unify local updates with previous registers
    for (auto & i : updates) {
        if (i.second.reg) {
            function.add_phi(table(i.first)->reg, i.second.reg);
        }
    }

    auto condition = cond->codegen(function, table, ftable);
    function.branch_existing<vmips::bnez>(block, condition);
    function.new_section();
    return nullptr;
}

VRegPtr ReturnNode::codegen(vmips::Function &function, SymTable &table, FunctionTable &ftable) {
    if (expression) {
        auto expr = expression->codegen(function, table, ftable);
        function.assign_special(vmips::SpecialReg::v0, expr);
        function.add_ret();
    }
    return nullptr;
}
