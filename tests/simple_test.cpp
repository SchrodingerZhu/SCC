//
// Created by schrodinger on 4/18/21.
//
#include "../codegen.h"

void test_trim() {
    assert(codegen::trim(" 123 ") == "123");
    assert(codegen::trim(" abc\n") == "abc");
}

void test_function() {
    const static char data[] =
            "int i;"
            "int sum;"
            "i = 1;"
            "sum = 0;"
            "while (i < 100 || i == 100) {"
            " sum = sum + i;"
            " i = i + 1;"
            "}"
            "print(sum);";
   auto i = scc::match_rule<scc::program>(data);
   i->display(std::cout);
   codegen::codegen(*i);
}
int main() {
    try {
        test_trim();
        test_function();
    } catch (const codegen::SemanticError & e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}