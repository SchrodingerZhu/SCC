//
// Created by schrodinger on 3/29/21.
//
#include "codegen.h"
#include "grammar.h"
#include "assembler.h"
#include "executor.h"
#include <fstream>
#include <optional>
#include <error.h>
#include <cstring>
#include <filesystem>

static const char HELP[] =
        "USAGE:\n"
        "  scc [input] [-o output] [ options ]\n"
        "OPTIONS:"
        "  -o,--output <path>          executable output path [default: a.out]\n"
        "  -O,--asm-output <path>      asm output path [default: a.S]\n"
        "  -a,--assembler <path>       assembler path [default: mipsel-linux-musl-cc]\n"
        "  -e,--execute                execute after compile\n"
        "  -X,--emulator <path>        set emulator path [qemu-mipsel]"
        "";


enum class OptParseState {
    NORMAL,
    WAIT_FOR_OUTPUT,
    WAIT_FOR_ASM,
    WAIT_FOR_ASM_OUTPUT,
    WAIT_FOR_EMULATOR
};

#define OPTION(short, long, ...) \
    else if (std::strcmp(argv[i], "-" #short) == 0 || std::strcmp(argv[i], "--" #long) == 0) __VA_ARGS__

static inline void exit_with_error(std::string_view msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
    std::exit(-EINVAL);
}

struct Opt {
    std::optional<std::ifstream> input = std::nullopt;
    std::optional<std::ofstream> asm_out = std::nullopt;
    std::optional<std::ofstream> exec_out = std::nullopt;
    assembler asm_opt {"mipsel-linux-musl-cc" };
    std::optional<executor> exec_opt = std::nullopt;
    std::filesystem::path asm_path, exec_path;

    Opt(int argc, const char **argv) {
        auto state = OptParseState::NORMAL;
        bool asm_set = false;
        bool emulator_set = false;
        for (auto i = 1; i < argc; ++i) {
            switch (state) {
                case OptParseState::NORMAL:
                    if (argv[i][0] != '-') {
                        if (!input) {
                            input = std::ifstream{argv[i]};
                        } else {
                            exit_with_error("multiple input file is not supported");
                        }
                    } OPTION(e, execute, {
                        if (!exec_opt) {
                            exec_opt = executor("qemu-mipsel");
                        }
                    })
                    OPTION(o, output, {
                        if (!exec_out) {
                            state = OptParseState::WAIT_FOR_OUTPUT;
                        } else {
                            exit_with_error("multiple output file is not supported");
                        }
                    })
                    OPTION(O, asm-output, {
                        if (!asm_out) {
                            state = OptParseState::WAIT_FOR_ASM_OUTPUT;
                        } else {
                            exit_with_error("multiple asm output file is not supported");
                        }
                    })
                    OPTION(a, assembler, {
                        if (!asm_set) {
                            asm_set = true;
                            state = OptParseState::WAIT_FOR_ASM;
                        } else {
                            exit_with_error("assembler path is already provided");
                        }
                    })
                    OPTION(X, emulator, {
                        if (!exec_opt) {
                            exit_with_error("emulator path can only be provided after the execution mode is enabled");
                        }
                        if (!emulator_set) {
                            emulator_set = true;
                            state = OptParseState::WAIT_FOR_EMULATOR;
                        } else {
                            exit_with_error("emulator path is already provided");
                        }
                    })
                    break;
                case OptParseState::WAIT_FOR_OUTPUT:
                    exec_out = std::ofstream { argv[i] };
                    exec_path = argv[i];
                    state = OptParseState::NORMAL;
                    break;
                case OptParseState::WAIT_FOR_ASM:
                    asm_opt.asm_path =  argv[i];
                    state = OptParseState::NORMAL;
                    break;
                case OptParseState::WAIT_FOR_ASM_OUTPUT:
                    asm_out = std::ofstream { argv[i] };
                    asm_path = argv[i];
                    state = OptParseState::NORMAL;
                    break;
                case OptParseState::WAIT_FOR_EMULATOR:
                    exec_opt->qemu_path = argv[i];
                    state = OptParseState::NORMAL;
                    break;
            }
        }
        if (state != OptParseState::NORMAL) {
            exit_with_error("incomplete options");
        }
        if (!exec_out) {
            exec_out = std::ofstream { "a.out"};
            exec_path = "a.out";
        }
        if (!asm_out) {
            asm_out = std::ofstream { "a.S"};
            asm_path = "a.S";
        }
    }
};

int main(int argc, const char** argv) {
    Opt opt {argc, argv};
    std::istream* input = &std::cin;
    if (opt.input) {
        input = &*opt.input;
    }
    std::istreambuf_iterator<char> begin(*input), end;
    std::string source(begin, end);
    auto ast = scc::program().match({
                                          std::make_shared<parser::MemoTable>(),
                                          source,
                                          0,
                                          0
                                  });
    if (ast) {
        ast = ast->compress<scc::SelectRule>()[0];
        auto module = codegen::codegen(*ast);
        module.output(*opt.asm_out);
        opt.asm_out->flush();
        opt.asm_out->close();
        try {
            auto ret = opt.asm_opt.compile(opt.asm_path, opt.exec_path);
            if (ret) throw std::runtime_error("non-zero return code from assembler");
            if (opt.exec_opt) {
                ret = opt.exec_opt->execute(opt.exec_path);
                if (ret) throw std::runtime_error("non-zero return code from emulator");
            }
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    } else {
        std::cerr << "syntax error" << std::endl;
    }
}


