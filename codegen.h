#ifndef SIMPLIFIED_C_CODEGEN_H
#define SIMPLIFIED_C_CODEGEN_H

#include <iostream>
#include <optional>
#include <memory>
#include <vcfg/virtual_mips.h>
#include <frontend.h>
#include <phmap.h>

struct Variable {
    std::shared_ptr <vmips::VirtReg> reg = nullptr;
    std::shared_ptr <vmips::MemoryLocation> location = nullptr;
    std::shared_ptr <vmips::Data> data = nullptr;

    Variable() = default;
    explicit Variable(std::shared_ptr <vmips::VirtReg> reg) : reg(std::move(reg)) {}
    explicit Variable(std::shared_ptr <vmips::MemoryLocation> loc) : location(std::move(loc)) {}
    explicit Variable(std::shared_ptr <vmips::Data> data) : data(std::move(data)) {}
};


struct SemanticException : public std::exception {
    const std::string reason;
    explicit SemanticException(std::string reason) : reason(std::move(reason)) {}
    const char * what() const noexcept override {
        return reason.c_str();
    }
};

struct CompilerInternalException : public std::exception {
    const std::string reason;
    explicit CompilerInternalException(std::string reason) : reason(std::move(reason)) {}
    const char * what() const noexcept override {
        return reason.c_str();
    }
};

using VRegPtr = std::shared_ptr<vmips::VirtReg>;
using FuncPtr = std::shared_ptr<vmips::Function>;
using SymTable = symtable::SymTable<struct Variable>;
using FunctionTable = phmap::flat_hash_map<std::string, FuncPtr>;

class ASTNode {
public:
    virtual VRegPtr codegen(vmips::Function &function, SymTable &table, FunctionTable& ftable) { return nullptr; };

};

class Constant : public ASTNode {
    const int value;

    explicit Constant(int value) : value(value) {}

    VRegPtr codegen(vmips::Function &function, SymTable &table, FunctionTable& ftable) override;
};

class IFNode : public ASTNode {
    std::unique_ptr <ASTNode> cond;
    std::unique_ptr <ASTNode> main_body;
    std::unique_ptr <ASTNode> else_body;
    VRegPtr codegen(vmips::Function &function, SymTable &table, FunctionTable& ftable) override;
};

class WhileNode : public ASTNode {
    std::unique_ptr <ASTNode> cond;
    std::unique_ptr <ASTNode> main_body;
    VRegPtr codegen(vmips::Function &function, SymTable &table, FunctionTable& ftable) override;
};

class DoWhileNode : public ASTNode {
    std::unique_ptr <ASTNode> cond;
    std::unique_ptr <ASTNode> main_body;
    VRegPtr codegen(vmips::Function &function, SymTable &table, FunctionTable& ftable) override;

};

class ReturnNode : public ASTNode {
    std::unique_ptr <ASTNode> expression;
    VRegPtr codegen(vmips::Function &function, SymTable &table, FunctionTable& ftable) override;
};

class CodeBlockNode : public ASTNode {
    std::vector <std::unique_ptr<ASTNode>> statements;
    VRegPtr codegen(vmips::Function &function, SymTable &table, FunctionTable& ftable) override {
        for (auto & i : statements) {
            i->codegen(function, table, ftable);
        }
        return nullptr;
    }
};

class DefNode : public ASTNode {
    std::string identifier;
    VRegPtr codegen(vmips::Function &function, SymTable &table, FunctionTable& ftable) override {
        if (table.defined_same_scope(identifier)) {
            throw SemanticException { std::string { "repetitious declaration of variable in the same scope: " } + identifier };
        } else {
            table.define(identifier, function.append<vmips::li>(0));
        }
        return nullptr;
    }
};


class AssignmentNode : public ASTNode {
    std::string identifier;
    std::unique_ptr <ASTNode> expression;
    VRegPtr codegen(vmips::Function &function, SymTable &table, FunctionTable& ftable) override {
        auto expr = expression->codegen(function, table, ftable);
        auto var = table(identifier);
        if (var) {
            table.update(identifier, expr);
        }
        return nullptr;
    }
};

class FunctionCall : public ASTNode {
    std::string identifier;
    std::vector <std::unique_ptr<ASTNode>> arguments;
    VRegPtr codegen(vmips::Function &function, SymTable &table, FunctionTable& ftable) override {
        std::vector<VRegPtr> vals;
        for (auto & i : arguments) {
            vals.emplace_back(i->codegen(function, table, ftable));
        }
        auto func = ftable.at(identifier);
        if (func == nullptr) throw SemanticException {std::string { "undefined function symbol: "} + identifier };
        return function.call(func, vals);
    }
};

enum class UnaryOperation {
    POS, NEG, BNEG, NOT
};
enum class BinaryOperation {
    ADD, SUB, DIV, MUT, MOD,
    LOGIC_AND, LOGIC_OR,
    BITWISE_AND, BITWISE_OR, BITWISE_XOR,
    SHIFT_LEFT, SHIFT_RIGHT
};

class BinaryNode : public ASTNode {
    std::unique_ptr <ASTNode> left;
    std::unique_ptr <ASTNode> right;

};

class UnaryNode : public ASTNode {
    UnaryOperation op;
    std::unique_ptr <ASTNode> operand;
    VRegPtr codegen(vmips::Function &function, SymTable &table, FunctionTable& ftable) override {
        switch (op) {
            case UnaryOperation::NOT: return nullptr;
             // function.append<sl>
        }
    }
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

