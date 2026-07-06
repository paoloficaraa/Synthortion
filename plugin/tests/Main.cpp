#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "AudioPluginTest.h"

int main(int argc, char* argv[])
{
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure(false);
    runner.runTestsInCategory("Synthortion");

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        if (auto* result = runner.getResult(i))
            failures += result->failures;

    return failures > 0 ? 1 : 0;
}
