//
// Created by schrodinger on 4/16/21.
//

#ifndef SIMPLIFIED_C_CODEGEN_H
#define SIMPLIFIED_C_CODEGEN_H

#include <variant>
#include "grammar.h"
#include "sym_table.h"
#include "vcfg/virtual_mips.h"


namespace codegen {
    using VRegPtr = std::shared_ptr<vmips::VirtReg>;
    using VMemPtr = std::shared_ptr<vmips::MemoryLocation>;
    using TableItem = std::variant<VRegPtr, VMemPtr>;
    using SccSymTable = symtable::SymTable<TableItem>;

#define CODEGEN(RULE, BLOCK) \
    if ( tree.instance == typeid (RULE) ) BLOCK

    std::string trim(std::string_view view);

    std::shared_ptr<vmips::VirtReg>
    function_generate(vmips::Function &func, const parser::ParseTree &tree, SccSymTable &table);

    class SemanticError : std::exception {
        std::string message;

        [[nodiscard]] const char *what() const noexcept override;

    public:
        explicit SemanticError(std::string message);

    };

    void function_entry(vmips::Module &module, SccSymTable &table, const parser::ParseTree &root);

    void codegen(const parser::ParseTree &root) {
        assert(root.instance == typeid(scc::program));
        auto table = SccSymTable{};
        auto module = vmips::Module{"program"};
        for (const auto &i : root.subtrees) {
            if (i->instance == typeid(scc::function)) {
                function_entry(module, table, *i);
            } else {
                /// TODO: fix global variables
            }
        }
        module.finalize();
        module.output(std::cout);
    }
}

#endif //SIMPLIFIED_C_CODEGEN_H
