#include "javabind/codegen.hpp"
#include <iostream>

JNIEXPORT void codegen(const std::filesystem::path& output_path);

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " OUTPUT_DIRECTORY" << std::endl;
        return -1;
    }
    codegen(argv[1]);
    return 0;
}