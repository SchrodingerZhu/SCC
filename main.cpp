//
// Created by schrodinger on 3/29/21.
//
#include "codegen.h"
#include "grammar.h"
#include <fstream>
#include <optional>
#include <error.h>
#include <cstring>

struct Opt {
    bool graphviz = false;
    std::optional <std::ifstream> fin = std::nullopt;
    std::optional <std::ofstream> fout = std::nullopt;
};

const char HELP[] =
        "USAGE: scc [input file] [-o <output file>] [-g]\n"
        "OPTIONS:\n"
        "    -h,--help           print this help message and exit\n"
        "    -o,--output <path>  set output file path\n"
        "    -g,--graphviz       output graphviz format\n"
        "NOTICES:\n"
        "    if no input or output is set, the program will use standard\n"
        "  io on default.";

static inline void error_exit() {
    std::cerr << "invalid arguments; use `scc -h` to see help messages.";
    std::exit(-EINVAL);
}

#define CMDCMP(SHORT, LONG) \
    (strcmp(argv[i], "-" #SHORT ) == 0 || strcmp(argv[i], "--" #LONG) == 0)

Opt parse_option(int argc, const char **argv) {
    Opt opt;
    bool expect_out = false;
    for (int i = 1; i < argc; ++i) {
        if (expect_out) {
            if (opt.fout) error_exit();
            else {
                opt.fout = std::ofstream{argv[i]};
            }
            expect_out = true;
        }
        if (argv[i][0] == '-') {
            if CMDCMP(h, help) {
                std::cout << HELP << std::endl;
                exit(0);
            } else if CMDCMP(o, output) {
                expect_out = true;
            } else if CMDCMP(g, graphviz) {
                opt.graphviz = true;
            } else {
                error_exit();
            }
        } else if (opt.fin) {
            error_exit();
        } else {
            opt.fin = std::ifstream(argv[i]);
        }
    }
    return opt;
}

int main(int argc, const char **argv) {
    Opt opt = parse_option(argc, argv);
    std::istream* input = &std::cin;
    std::ostream* output = &std::cout;
    if (opt.fin) {
        input = &*opt.fin;
    }
    if (opt.fout) {
        output = &*opt.fout;
    }
    std::istreambuf_iterator<char> begin(*input), end;
    std::string data(begin, end);
    auto a = scc::program().match({
                                          std::make_shared<parser::MemoTable>(),
                                          data,
                                          0,
                                          0
                                  });
    if (a) {
        a = a->compress<scc::SelectRule>()[0];
        if (opt.graphviz) scc::visualize(*output, *a);
        else a->display(*output);
    } else {
        std::cerr << "syntax error" << std::endl;
    }
}
