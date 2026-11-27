//
// Created by NHCM on 2026/11/26.
//

#include "Compiler.h"
#include "util/FileUtil.h"
#include "util/PrintUtil.h"

namespace javac
{
    void Compiler::printTerminate()
    {
        jcutils::printErrorCustom("Compiler terminated. " +
                std::to_string(compileRtData->errors.size()) + " errors occurred, " +
                std::to_string(compileRtData->warnings.size()) + " warnings generated.");
    }

    void Compiler::noticeError(const std::string& error, std::string_view owner)
    {
        compileRtData->errors.push_back(error);
        jcutils::printErrorCustom(error, owner);
    }
    void Compiler::noticeWarning(const std::string& warning)
    {
        if (!compilerConfig.nowarn)
        {
            compileRtData->warnings.push_back(warning);
            jcutils::printWarningCustom(warning);
        }
    }

    bool Compiler::addParentPath(const Path& sourcePath, const Path& parentPath)
    {
        return compileRtData->addParentPath(sourcePath, parentPath);
    }

    Path* Compiler::getParentPath(const Path& sourceFilePath)
    {
        return compileRtData->getParentPath(sourceFilePath);
    }

    Path& Compiler::outputPath()
    {
        return compilerConfig.outputDir;
    }

    TextEncoding& Compiler::encoding()
    {
        return compilerConfig.encoding;
    }

    bool Compiler::putFileToWrite(const Path& filePath, const ByteArray& data)
    {
        return compileRtData->filesToWrite.try_emplace(fs::absolute(filePath), data).second;
    }

    Path Compiler::getWritePath(const Path& filePath)
    {
        Path writePath = jcutils::joinAbsolutePath(outputPath(), filePath.filename());
        Path* parentPath = getParentPath(filePath);
        if (parentPath)
        {
            writePath = jcutils::joinAbsolutePath(outputPath(), jcutils::getRelativePath(filePath, *parentPath));
        }
        return writePath;
    }
    Path Compiler::getWritePath(const Path& filePath, const std::string& newExtension)
    {
        return jcutils::changeExtension(getWritePath(filePath), newExtension);
    }

    bool Compiler::verbose() const
    {
        return compilerConfig.verbose;
    }
}
