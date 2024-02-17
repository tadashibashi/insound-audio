// Lua compiler
#include <sol/sol.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace fs = std::filesystem;

enum class ErrorCode : int
{
    OK,
    ArgumentError,
    FilePathError,
    FileTypeError,
    LuaError,
    OutputPathError,
    OutputError,
};

int main(int argc, char *argv[])
{
    // Check and collect args
    if (argc < 3)
    {
        std::cerr << "Error: there must be an argument indicating a file path\n";
        return (int)ErrorCode::ArgumentError;
    }

    const auto filepath = fs::path(argv[1]);
    const std::string symbol = argv[2];
    std::string outputPath = argc < 4 ? "" : argv[3];

    if (!fs::exists(filepath))
    {
        std::cerr << "Error: file does not exist at provided path \"" << filepath << "\"\n";
        return (int)ErrorCode::FilePathError;
    }

    if (!fs::is_regular_file(filepath))
    {
        std::cerr << "Error: object is not a file at provided path \"" << filepath << "\"\n";
        return (int)ErrorCode::FileTypeError;
    }


    // Load and lua dump bytecode
    sol::state lua;
    auto result = lua.load_file(filepath.c_str());
    if (!result.valid())
    {
        sol::error err = result;
        std::cerr << err.what() << '\n';
        return (int)ErrorCode::LuaError;
    }

    auto bytecode = result.get<sol::protected_function>().dump();


    // Get output path
    if (outputPath.empty())
    {
        auto filename = filepath.stem().string();
        auto folderPath = filepath.parent_path().string();
        outputPath = (folderPath.empty() ? filename : folderPath + "/" + filename) + ".lua.bc";

        fs::create_directories(folderPath); // create folders if they don't exist
    }
    else
    {
        auto folderPath = fs::path(outputPath).parent_path();
        fs::create_directories(folderPath); // create folders if they don't exist
    }


    // Write bytecode to the path
    std::ofstream outFile(outputPath, std::ios_base::out);
    if (!outFile.is_open())
    {
        std::cerr << "Error: problem creating output file at path " << outputPath << "\n";
        return (int)ErrorCode::OutputPathError;
    }
    auto byteString = bytecode.as_string_view();

    outFile << std::hex << "#pragma once\n"
               "#include <cstdint>\n"
        "const uint8_t " << symbol << "[] = {";

    const auto maxIndex = byteString.size() - 1;
    for (int i = 0; i < maxIndex; ++i)
    {
        outFile << "0x" << (int)(uint8_t)byteString[i] << ",";
    }
    outFile << "0x" << (int)(uint8_t)byteString[maxIndex] << "};\n";

    if (!outFile)
    {
        std::cerr << "Error: problem writing file\n";
        return (int)ErrorCode::OutputError;
    }

    outFile.close();

    // Done!
    std::cout << "Success: wrote file \"" << outputPath << "\"";
    return (int)ErrorCode::OK;
}
