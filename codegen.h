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
    function_generate(EFuncFactory &factory, vmips::Function &func, const parser::ParseTree &tree, SccSymTable &table,
                      NodePtr &node);

    class SemanticError : std::exception {
        std::string message;
    public:
        [[nodiscard]] const char *what() const noexcept override;


        explicit SemanticError(std::string message);

    };

    void codegen(const parser::ParseTree &root);

    static inline void add_read(vmips::Module &module, EFuncFactory& factory) {
        using namespace vmips;
        auto scanf = module.create_extern("scanf", 3);
        auto read = module.create_function("read", 0);
        auto format = read->create_data<asciiz>(true, "%d");
        auto data = read->new_memory(4);
        auto offset = read->append<address>(data);
        auto address = read->append<add>(offset, get_special(SpecialReg::s8));
        auto faddr = read->append<la>(format);
        read->call_void(scanf, faddr, address);
        auto result = read->append<lw>(data);
        read->assign_special(SpecialReg::v0, result);
        factory.cache.insert(std::make_pair("read", read));
    }

    static inline void add_write(vmips::Module &module, EFuncFactory& factory) {
        using namespace vmips;
        auto printf = module.create_extern("printf", 3);
        auto write = module.create_function("write", 1);
        auto format = write->create_data<asciiz>(true, "%d\n");
        auto faddr = write->append<la>(format);
        write->call_void(printf, faddr, get_special(SpecialReg::a0));
        factory.cache.insert(std::make_pair("write", write));
    }

    static inline void configure_builtin(vmips::Module &mod, EFuncFactory& factory) {
        add_read(mod, factory);
        add_write(mod, factory);
    }
}

#endif //SIMPLIFIED_C_CODEGEN_H
