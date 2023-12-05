#include <catch2/catch_session.hpp>

int main(int argc, char *argv[])
{
    // Pre-setup here
    // ...

    // Run tests
    return Catch::Session().run(argc, argv);
}
