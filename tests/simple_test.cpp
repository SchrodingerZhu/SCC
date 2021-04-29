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
            "int i; int n;"
            "int sum[2];"
            "i = 1;"
            "sum[1] = 0;"
            "n = read();"
            "while (i <= n) {"
            " sum[1] = sum[1] + i;"
            " i = i + 1;"
            " if (i == 3 || i == 5) { write( i ); } "
            "}"
            "write(sum[1]);";
   auto i = scc::match_rule<scc::program>(data);
   auto a = codegen::codegen(*i);
   a.output(std::cout);
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