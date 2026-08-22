#include "AudioPluginTest.h"
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <iostream>

int main (int /*argc*/, char** /*argv*/)
{
    juce::initialiseJuce_GUI();
    juce::UnitTestRunner runner;
    runner.runTestsInCategory ("Synthortion");

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        const auto* res = runner.getResult (i);
        std::cout << "Test: " << res->unitTestName << " - Passed: " << res->passes << " Failed: " << res->failures << std::endl;
        failures += static_cast<int> (res->failures);
    }

    juce::shutdownJuce_GUI();

    return failures == 0 ? 0 : 1;
}
