// Lua compiler
#include <sol/sol.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>

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
    if (argc < 2)
    {
        std::cerr << "Error: there must be an argument indicating a file path\n";
        return (int)ErrorCode::ArgumentError;
    }

    const auto program = argv[0];
    const auto filepath = fs::path(argv[1]);
    std::string outputPath = argc < 3 ? "" : argv[2];

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
    std::ofstream outFile(outputPath, std::ios_base::binary | std::ios_base::out);
    if (!outFile.is_open())
    {
        std::cerr << "Error: problem creating output file at path " << outputPath << "\n";
        return (int)ErrorCode::OutputPathError;
    }

    outFile << bytecode.as_string_view();

    if (!outFile)
    {
        std::cerr << "Error: problem writing file\n";
        return (int)ErrorCode::OutputError;
    }

    outFile.close();

    // Done!
    std::cout << "Success: wrote file \"" << outputPath << "\" (" << bytecode.as_string_view().size() << " bytes)\n";
    return (int)ErrorCode::OK;
}
