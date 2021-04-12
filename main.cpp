//
// Created by schrodinger on 3/29/21.
//
#include "codegen.h"
#include "grammar.h"
int main(int, char** argv) {
    auto a = scc::program().match({
                                std::make_shared<parser::MemoTable>(),
                                argv[1],
                                0,
                                0
                        });
    if (a) {
        a = a->compress<scc::SelectRule>()[0];
        a->display(std::cout);
    }
}

