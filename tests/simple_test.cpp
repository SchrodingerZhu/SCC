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
            "while (1) {"
            " int i; \n"
            " i = 1; "
            " do {\n"
            "   i = i; exit(0);"
            " } while (!i); "
            "}";
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