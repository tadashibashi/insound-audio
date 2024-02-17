#include <emscripten/bind.h>

using namespace emscripten;

class ExampleClass
{
public:
    explicit ExampleClass(int v = 0) : v(v) { }

    void setValue(int value) { v = value; }

    [[nodiscard]]
    int getValue() const { return v; }

    [[nodiscard]]
    int doubled() const { return v * 2; }

private:
    int v;
};

EMSCRIPTEN_BINDINGS(InsoundAudioBindings)
{
    class_<ExampleClass>("ExampleClass")
            .constructor<int>()
            .constructor<>()
            .property("value", &ExampleClass::getValue, &ExampleClass::setValue)
            .function("doubled", &ExampleClass::doubled)
            ;
}
