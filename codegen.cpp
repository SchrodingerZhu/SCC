#include "codegen.h"
VRegPtr Constant::codegen(vmips::Function &function, SymTable &table) {
    return function.append<vmips::li>(value);
}

VRegPtr WhileNode::codegen(vmips::Function &function, SymTable &table) {
    auto block = function.new_section();
    auto condition = cond->codegen(function, table);
    auto after = function.new_section_branch<vmips::beqz>(condition);
    function.switch_to(block);

    table.enter();
    main_body->codegen(function, table);
    table.escape();

    function.switch_to(after);
    return nullptr;
}

VRegPtr IFNode::codegen(vmips::Function &function, SymTable &table) {
    auto condition = cond->codegen(function, table);
    auto blocks = function.branch<vmips::beqz>(condition);

    table.enter();
    main_body->codegen(function, table);
    auto lv1 = table.local_updates<phmap::flat_hash_map<std::string, Variable>>();
    table.escape();

    if (else_body) {
        function.switch_to(blocks.second);

        table.enter();
        else_body->codegen(function, table);
        auto lv2 = table.local_updates<phmap::flat_hash_map<std::string, Variable>>();
        table.escape();

        function.join(blocks.first, blocks.second);
        for (auto & i : lv1) {
            if (lv2.contains(i.first) && i.second.reg) {
                function.add_phi(i.second.reg, lv2.at(i.first).reg);
            }
        }
    }

    return nullptr;
}

VRegPtr DoWhileNode::codegen(vmips::Function &function, SymTable &table) {
    auto block = function.new_section();
    table.enter();
    main_body->codegen(function, table);
    table.escape();
    auto condition = cond->codegen(function, table);
    function.branch_existing<vmips::bnez>(block, condition);
    function.new_section();
    return nullptr;
}

VRegPtr ReturnNode::codegen(vmips::Function &function, SymTable &table) {
    if (expression) {
        auto expr = expression->codegen(function, table);
        function.assign_special(vmips::SpecialReg::v0, expr);
        function.add_ret();
    }
    return nullptr;
}
