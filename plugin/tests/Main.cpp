#include <atomic>
#include <cstdlib>
#include <new>

std::atomic<bool> g_trackAllocations{false};
std::atomic<size_t> g_allocationCount{0};

void* operator new(size_t size)
{
    if (g_trackAllocations.load(std::memory_order_relaxed))
        g_allocationCount.fetch_add(1, std::memory_order_relaxed);
    void* ptr = std::malloc(size);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

void operator delete(void* ptr) noexcept
{
    std::free(ptr);
}

void operator delete(void* ptr, size_t) noexcept
{
    std::free(ptr);
}

#include "AudioPluginTest.h"
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <iostream>
int main (int /*argc*/, char** /*argv*/)
{
    juce::initialiseJuce_GUI();
    struct StdoutLogger : public juce::Logger {
        void logMessage(const juce::String& message) override {
            std::cerr << message.toStdString() << "\n";
            std::cerr.flush();
        }
    } logger;
    juce::Logger::setCurrentLogger(&logger);

    struct CustomRunner : public juce::UnitTestRunner {
        void logMessage(const juce::String& message) override {
            std::cout << message.toStdString() << "\n";
            std::cout.flush();
        }
    } runner;
    runner.setAssertOnFailure (false);
    runner.runTestsInCategory ("Synthortion");

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        const auto* res = runner.getResult (i);
        std::cerr << "Test: " << res->unitTestName.toStdString() << " - Passed: " << res->passes << " Failed: " << res->failures << "\n";
        for (auto& msg : res->messages)
        {
            std::cerr << "  " << msg.toStdString() << "\n";
        }
        failures += static_cast<int> (res->failures);
    }
    std::cerr << "TOTAL FAILURES: " << failures << "\n";
    std::cerr.flush();
    std::cout.flush();

    juce::Logger::setCurrentLogger(nullptr);
    juce::shutdownJuce_GUI();

    return failures == 0 ? 0 : 1;
}
