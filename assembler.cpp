//
// Created by schrodinger on 4/29/21.
//

#include "assembler.h"

#include <utility>

int assembler::compile(const std::filesystem::path &input_path, const std::filesystem::path &output_path) {
    std::string builder;
    builder.append(asm_path);
    builder.append(" -static -x assembler ");
    builder.append(input_path);
    builder.append(" -o ");
    builder.append(output_path);
    return std::system(builder.c_str());
}

assembler::assembler(std::filesystem::path path) : asm_path(std::move(path)) {}
