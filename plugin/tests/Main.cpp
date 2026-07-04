#include "AudioPluginTest.h"
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

int main (int /*argc*/, char** /*argv*/)
{
    juce::initialiseJuce_GUI();

    juce::UnitTestRunner runner;
    runner.runTestsInCategory ("Synthortion", juce::Random::getSystemRandom().nextInt64());

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += static_cast<int> (runner.getResult (i)->failures);

    juce::shutdownJuce_GUI();

    return failures == 0 ? 0 : 1;
}
