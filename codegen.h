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
    using NodePtr = std::shared_ptr<vmips::CFGNode>;
    using FuncPtr = std::shared_ptr<vmips::Function>;
    struct UninitializedValue {
    };

    struct EFuncFactory {
        vmips::Module *module{};
        phmap::flat_hash_map<std::string_view, FuncPtr> cache;

        FuncPtr get_or_create(std::string_view name, size_t argc);
    };

    using TableItem = std::variant<VRegPtr, VMemPtr, UninitializedValue>;
    using SccSymTable = symtable::SymTable<TableItem>;

#define CODEGEN(RULE, BLOCK) \
    if ( tree.instance == typeid (RULE) )  { BLOCK; goto END; }

    std::string trim(std::string_view view);

    std::shared_ptr<vmips::VirtReg>
    function_generate(EFuncFactory& factory, vmips::Function &func, const parser::ParseTree &tree, SccSymTable &table, NodePtr &node);

    class SemanticError : std::exception {
        std::string message;
    public:
        [[nodiscard]] const char *what() const noexcept override;


        explicit SemanticError(std::string message);

    };

    void codegen(const parser::ParseTree &root);
}

#endif //SIMPLIFIED_C_CODEGEN_H
