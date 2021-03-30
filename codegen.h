#ifndef SIMPLIFIED_C_CODEGEN_H
#define SIMPLIFIED_C_CODEGEN_H

#include <iostream>
#include <optional>
#include <memory>
#include <vcfg/virtual_mips.h>
#include <frontend.h>
#include <phmap.h>

struct Variable {
    std::shared_ptr <vmips::VirtReg> reg;
    std::shared_ptr <vmips::MemoryLocation> location;

    explicit Variable(std::shared_ptr <vmips::VirtReg> reg) : reg(std::move(reg)) {}
    explicit Variable(std::shared_ptr <vmips::MemoryLocation> loc) : location(std::move(loc)) {}

    bool uninitialized() const noexcept {
        return location == nullptr && reg == nullptr;
    }
};


struct SemanticException : public std::exception {
    const std::string reason;
    explicit SemanticException(std::string reason) : reason(std::move(reason)) {}
    const char * what() const noexcept override {
        return reason.c_str();
    }
};

using VRegPtr = std::shared_ptr<vmips::VirtReg>;
using FuncPtr = std::shared_ptr<vmips::Function>;
using SymTable = symtable::SymTable<struct Variable>;

class ASTNode {
public:
    virtual VRegPtr codegen(vmips::Function &function, SymTable &table) { return nullptr; };
};

class Constant : public ASTNode {
    const int value;

    explicit Constant(int value) : value(value) {}

    VRegPtr codegen(vmips::Function &function, SymTable &table) override;
};

class IFNode : public ASTNode {
    std::unique_ptr <ASTNode> cond;
    std::unique_ptr <ASTNode> main_body;
    std::unique_ptr <ASTNode> else_body;
    VRegPtr codegen(vmips::Function &function, SymTable &table) override;
};

class WhileNode : public ASTNode {
    std::unique_ptr <ASTNode> cond;
    std::unique_ptr <ASTNode> main_body;
    VRegPtr codegen(vmips::Function &function, SymTable &table) override;
};

class DoWhileNode : public ASTNode {
    std::unique_ptr <ASTNode> cond;
    std::unique_ptr <ASTNode> main_body;
    VRegPtr codegen(vmips::Function &function, SymTable &table) override;

};

class ReturnNode : public ASTNode {
    std::unique_ptr <ASTNode> expression;
    VRegPtr codegen(vmips::Function &function, SymTable &table) override;
};

class CodeBlockNode : public ASTNode {
    std::vector <std::unique_ptr<ASTNode>> statements;
    VRegPtr codegen(vmips::Function &function, SymTable &table) override {
        for (auto & i : statements) {
            i->codegen(function, table);
        }
        return nullptr;
    }
};

class DefNode : public ASTNode {
    std::string identifier;
    VRegPtr codegen(vmips::Function &function, SymTable &table) override {
        if (table.defined_same_scope(identifier)) {
            throw SemanticException { std::string { "repetitious declaration of variable in the same scope: " } + identifier };
        } else {
            table.define(identifier, nullptr);
        }
        return nullptr;
    }
};


class AssignmentNode : public ASTNode {
    std::string identifier;
    std::unique_ptr <ASTNode> expression;
    VRegPtr codegen(vmips::Function &function, SymTable &table) override {
        for (auto & i : statements) {
            i->codegen(function, table);
        }
        return nullptr;
    }
};

class FunctionCall : public ASTNode {
    std::string identifier;
    std::vector <std::unique_ptr<ASTNode>> arguments;
};

class BinaryNode : public ASTNode {
    std::unique_ptr <ASTNode> left;
    std::unique_ptr <ASTNode> right;
};

class VariableNode : public ASTNode {
    std::string identifier;
};

class ArrayDec : public ASTNode {

};

class ArrayAccess : public ASTNode {
    std::string identifier;
    int offset;
};

#endif //SIMPLIFIED_C_CODEGEN_H

