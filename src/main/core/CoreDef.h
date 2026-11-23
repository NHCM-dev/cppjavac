//
// Created by NHCM on 2026/11/22.
//

#ifndef JAVACOMPILER_COREDEF_H
#define JAVACOMPILER_COREDEF_H

#include <filesystem>
#include <vector>
#include <set>
#include <stdexcept>

#include "def/TextEncoding.h"
#include "def/JavaDef.h"

namespace fs = std::filesystem;

using Path = fs::path;
using ByteArray = std::vector<std::byte>;

#endif //JAVACOMPILER_COREDEF_H
