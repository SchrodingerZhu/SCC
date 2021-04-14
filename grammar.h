#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <frontend.h>
#include <iostream>
#include <memory>
#include <cxxabi.h>

namespace scc {
    // grammar definitions
    using namespace parser;
    struct expression;
    struct statement;
    struct toplevel;
    struct fcall;
    struct access;
    struct paren_expr;
    struct KEYWORD;

    RULE(DIGIT, CharRange<'0', '9'>)
    RULE(IDENTIFIER_START, Ord<Char<'_'>, CharRange<'a', 'z'>, CharRange<'A', 'Z'>>)
    RULE(IDENTIFIER_CHAR, Ord<DIGIT, IDENTIFIER_START>);

    RULE(identifier, Seq<Not<KEYWORD>, Plus<IDENTIFIER_START>, Asterisk<IDENTIFIER_CHAR>>)
    RULE(integer, Plus<DIGIT>)

    RULE(unary_term, Ord<fcall, access, identifier, integer, paren_expr>)
    RULE(access, SpaceInterleaved<identifier, Char<'['>, unary_term, Char<']'>>)
    RULE(unary_pos, SpaceInterleaved<Char<'+'>, unary_term>)
    RULE(unary_neg, SpaceInterleaved<Char<'-'>, unary_term>)
    RULE(unary_bneg, SpaceInterleaved<Char<'~'>, unary_term>)
    RULE(unary_not, SpaceInterleaved<Char<'!'>, unary_term>)
    RULE(unary, Ord<unary_pos, unary_neg, unary_bneg, unary_not>)

    RULE(paren_expr, SpaceInterleaved<Char<'('>, expression, Char<')'>>)
    RULE(expr_list, SpaceInterleaved<Char<'('>, Optional<
            SpaceInterleaved<expression, Asterisk<SpaceInterleaved<Char<','>, expression
            >>>>, Char<')'>>)
    RULE(fcall, SpaceInterleaved<identifier, expr_list>)
    RULE(primary, Ord<unary, fcall, access, identifier, integer, paren_expr>)

#define BINARY_RULE(NAME, THIS, HIGHER, ...) \
    RULE(binary_##NAME, SpaceInterleaved<HIGHER, __VA_ARGS__, THIS>)

#define BINARY_LEVEL(NAME, THIS, HIGHER, ...) \
    BINARY_RULE(NAME, THIS, HIGHER, __VA_ARGS__) \
    RULE(THIS, Ord<binary_##NAME, HIGHER>);

    struct binary0;
    struct binary1;
    struct binary2;
    struct binary3;
    struct binary4;
    struct binary5;
    struct binary6;
    struct binary7;
    struct binary8;

    BINARY_RULE(mul, binary0, primary, Char<'*'>)
    BINARY_RULE(div, binary0, primary, Char<'/'>)
    BINARY_RULE(mod, binary0, primary, Char<'%'>)
    RULE(binary0, Ord<binary_mul, binary_div, binary_mod, primary>);


    BINARY_RULE(add, binary1, binary0, Char<'+'>)
    BINARY_RULE(sub, binary1, binary0, Char<'-'>)
    RULE(binary1, Ord<binary_add, binary_sub, binary0>)

    BINARY_RULE(shl, binary2, binary1, Keyword<'<', '<'>)
    BINARY_RULE(shr, binary2, binary1, Keyword<'>', '>'>)
    RULE(binary2, Ord<binary_shl, binary_shr, binary1>)

    BINARY_RULE(lt, binary3, binary2, Char<'<'>)
    BINARY_RULE(le, binary3, binary2, Keyword<'<', '='>)
    BINARY_RULE(gt, binary3, binary2, Char<'>'>)
    BINARY_RULE(ge, binary3, binary2, Keyword<'>', '='>)
    RULE(binary3, Ord<binary_lt, binary_le, binary_gt, binary_ge, binary2>);

    BINARY_RULE(eq, binary4, binary3, Keyword<'=', '='>)
    BINARY_RULE(ne, binary4, binary3, Keyword<'!', '='>)
    RULE(binary4, Ord<binary_eq, binary_ne, binary3>);

    BINARY_LEVEL(band, binary5, binary4, Char<'&'>)
    BINARY_LEVEL(bxor, binary6, binary5, Char<'^'>)
    BINARY_LEVEL(bor, binary7, binary6, Char<'|'>)
    BINARY_LEVEL(land, binary8, binary7, Keyword<'&', '&'>)
    BINARY_LEVEL(lor, expression, binary8, Keyword<'|', '|'>)
#define DEFINE_KEYWORD(NAME, ...) \
    RULE(NAME,  Seq<Keyword<__VA_ARGS__>, Not<IDENTIFIER_CHAR>>)

    DEFINE_KEYWORD(INT, 'i', 'n', 't')
    DEFINE_KEYWORD(VOID, 'v', 'o', 'i', 'd')
    DEFINE_KEYWORD(EXTERN, 'e', 'x', 't', 'e', 'r', 'n')
    DEFINE_KEYWORD(DO, 'd', 'o')
    DEFINE_KEYWORD(ELSE, 'e', 'l', 's', 'e')
    DEFINE_KEYWORD(IF, 'i', 'f')
    DEFINE_KEYWORD(WHILE, 'w', 'h', 'i', 'l', 'e')
    DEFINE_KEYWORD(RETURN, 'r', 'e', 't', 'u', 'r', 'n')


    RULE(KEYWORD, Ord<INT, VOID, EXTERN, DO, ELSE, IF, WHILE, RETURN>);
    RULE(var_decl, SpaceInterleaved<INT, identifier>)
    RULE(array_decl, SpaceInterleaved<INT, identifier, Char<'['>, integer, Char<']'>>)
    RULE(var_list, SpaceInterleaved<Char<'('>, Optional<
            SpaceInterleaved<var_decl, Asterisk<SpaceInterleaved<Char<','>, var_decl
            >>>>, Char<')'>>)
    RULE(extern_decl, SpaceInterleaved<EXTERN, Ord<INT, VOID>, identifier, var_list>)
    RULE(var_assign, SpaceInterleaved<identifier, Char<'='>, expression>)
    RULE(array_assign, SpaceInterleaved<access, Char<'='>, expression>)

    RULE(code_block, SpaceInterleaved<Char<'{'>, Asterisk<SpaceInterleaved<statement>>, Char<'}'>>)
    RULE(else_stmt, SpaceInterleaved<ELSE, statement>)
    RULE(if_stmt, SpaceInterleaved<IF, paren_expr, statement>)

    RULE(ifelse_statement, SpaceInterleaved<if_stmt, Optional<else_stmt>>)
    RULE(while_statement, SpaceInterleaved<WHILE, paren_expr, statement>)
    RULE(dowhile_statement, SpaceInterleaved<DO, statement, WHILE, paren_expr>)
    RULE(return_statement, SpaceInterleaved<RETURN, Optional<expression>>)

    RULE(statement, Ord<SpaceInterleaved<Ord<
            array_assign, var_assign, array_decl, var_decl, expression, return_statement, dowhile_statement, Nothing >,
         Char<';'>>, code_block, ifelse_statement, while_statement >)

    RULE(function, SpaceInterleaved<Ord<INT, VOID>, identifier, var_list, code_block>)
    RULE(toplevel, Ord<function, Char<';'>, SpaceInterleaved<Ord<extern_decl, var_decl>, Char<';'>>>)
    RULE(program, Seq<Start, Plus<SpaceInterleaved<toplevel>>, End>)

    // selection filter to rule out intermediate structures
    using SelectRule = Selector<binary_lor,
            binary_land, binary_bor, binary_bxor, binary_band, binary_ne, binary_eq, binary_ge, binary_gt, binary_le, binary_lt, binary_shr, binary_shl, binary_sub, binary_add,
            binary_mod, binary_div, binary_mul, fcall, unary_not, unary_bneg, unary_neg, unary_pos, access, identifier, integer, return_statement, dowhile_statement, while_statement, ifelse_statement,
            code_block, array_assign, var_assign, var_decl, array_decl, extern_decl, function, program>;

    // auxiliary function to output the dot graph for visualization
    int visualize_inner(std::ostream &out, const ParseTree &tree, int id = 0) {
        static char buffer[512];
        int status;
        size_t length = 512;
        abi::__cxa_demangle(tree.instance.name(), buffer, &length, &status);
        if (tree.instance == typeid(identifier) || tree.instance == typeid(integer)) {
            out << "\t" << id << " [ label= \"" << tree.parsed_region << "\"]; " << std::endl;
        } else {
            out << "\t" << id << " [ label= \"" << buffer << "\"]; " << std::endl;
        }
        int sub_id = id + 1;
        for (const auto &i : tree.subtrees) {
            out << "\t" << id << " -> " << sub_id << "; " << std::endl;
            sub_id = visualize_inner(out, *i, sub_id) + 1;
        }
        return sub_id;
    }

    // output the dot graph file to target stream
    void visualize(std::ostream &out, const ParseTree &tree) {
        out << "digraph AST {" << std::endl;
        visualize_inner(out, tree);
        out << "}" << std::endl;
    }
}


#endif // GRAMMAR_H
