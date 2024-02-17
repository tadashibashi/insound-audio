#include <insound/System.h>
#include <iostream>

using namespace Insound::Audio;

// Test sandbox until bindings are stable

// emscripten module needs a main function
int main()
{
    System sys;

    auto didInit = sys.init();
    if (!didInit)
    {
        std::cout << "Failed to initialize.\n";
        return -1;
    }

    sys.close();

    return 0;
}
