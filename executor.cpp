//
// Created by schrodinger on 4/29/21.
//

#include "executor.h"

#include <utility>

executor::executor(std::filesystem::path path) :
        qemu_path(std::move(path)){
}

int executor::execute(const std::filesystem::path &executable) {
    std::string builder;
    builder.append(qemu_path);
    builder.append(" ");
    builder.append(executable);
    return std::system(builder.c_str());
}
