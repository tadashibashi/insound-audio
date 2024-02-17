#include "test.h"
#include <insound/params/types/NumberParam.h>

TEST_CASE("NumberParam constructors set as expected")
{
    SECTION("Default Constructor")
    {
        NumberParam p;

        REQUIRE(p.min() == 0);
        REQUIRE(p.max() == 0);
        REQUIRE(p.step() == 0);
        REQUIRE(p.defaultValue() == 0);
    }

    SECTION("Integer Constructor")
    {
        NumberParam p(0, 10, 5);

        REQUIRE(p.min() == 0);
        REQUIRE(p.max() == 10);
        REQUIRE(p.step() == 1);
        REQUIRE(p.defaultValue() == 5);
    }

    SECTION("Float Constructor")
    {
        NumberParam p(-1000.0f, 1000.0f, .05f, 500.0f);

        REQUIRE(p.min() == -1000.0f);
        REQUIRE(p.max() == 1000.0f);
        REQUIRE(p.step() == .05f);
        REQUIRE(p.defaultValue() == 500.0f);
    }
}
