//
// Created by schrodinger on 4/29/21.
//

#ifndef SIMPLIFIED_C_ASSEMBLER_H
#define SIMPLIFIED_C_ASSEMBLER_H
#include <filesystem>
struct assembler {
    std::filesystem::path asm_path;
    explicit assembler(std::filesystem::path path);
    int compile(const std::filesystem::path& input_path, const std::filesystem::path& output_path);
};


#endif //SIMPLIFIED_C_ASSEMBLER_H
