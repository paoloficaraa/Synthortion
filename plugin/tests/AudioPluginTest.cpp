#include "AudioPluginTest.h"

#include <iostream>

class ConsoleTestRunner : public juce::UnitTestRunner
{
public:
    void logMessage(const juce::String& message) override
    {
        std::cout << message << std::endl;
    }
};

int main(int /*argc*/, char* /*argv*/[])
{
    juce::initialiseJuce_GUI();

    ConsoleTestRunner runner;
    runner.runAllTests(juce::Random::getSystemRandom().nextInt64());

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += static_cast<int>(runner.getResult(i)->failures);

    juce::shutdownJuce_GUI();

    return failures == 0 ? 0 : 1;
}
