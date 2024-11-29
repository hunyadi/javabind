#include <javabind/codegen.hpp>

JAVA_EXTENSION_IMPORT();

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " OUTPUT_DIRECTORY" << std::endl;
        return -1;
    }
    java_bindings_emit_signatures(argv[1]);
    return 0;
}
