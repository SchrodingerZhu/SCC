//
// Created by schrodinger on 4/29/21.
//

#ifndef SIMPLIFIED_C_EXECUTOR_H
#define SIMPLIFIED_C_EXECUTOR_H


#include <filesystem>
#include <optional>

struct executor {
    std::filesystem::path qemu_path;
    explicit executor(std::filesystem::path  path);
    int execute(const std::filesystem::path& executable);
};


#endif //SIMPLIFIED_C_EXECUTOR_H
