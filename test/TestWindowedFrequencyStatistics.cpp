#include <WindowedFrequencyStatistics.h>
#include <Random.h>

#include <catch2/catch.hpp>
#include <fmt/core.h>

#include <thread>

using namespace Tril;

namespace {
template <typename T>
    double GetHz(const T& start, const T& end, size_t count)
    {
        double secondsPassed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
        double frequency = count / secondsPassed;
        return frequency;
    }

}

TEST_CASE("WindowedFrequencyStatistics", "[stats]")
{
    SECTION("Evenly spaced events")
    {
        constexpr size_t windowSize = 10;

        for (size_t eventCount = 1; eventCount < windowSize * 2; ++eventCount) {
            WindowedFrequencyStatistics testStats(windowSize);
            auto startTime = testStats.GetTimestampOfLastEvent();

            for (size_t i = 0; i < eventCount; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(77));
                testStats.AddValue();

                // We don't want to include events that are no longer in our buffer
                // in the comparison, startTime -> endTime should be the events
                // in the buffer only or the check will be unfair and could fail
                // due to the inherent unreliableness of sleeping a thread for an
                // exact time period
                if (eventCount > windowSize && i < eventCount - windowSize) {
                    startTime = testStats.GetTimestampOfLastEvent();
                }
            }

            auto endTime = testStats.GetTimestampOfLastEvent();
            double expectedFrequency = GetHz(startTime, endTime, std::min(eventCount, windowSize));
            REQUIRE_THAT(testStats.MeanHz(), Catch::Matchers::WithinRel(expectedFrequency, 0.000001));
        }
    }

    SECTION("Randomly spaced events")
    {
        constexpr size_t windowSize = 10;

        for (size_t eventCount = 1; eventCount < windowSize * 2; ++eventCount) {
            WindowedFrequencyStatistics testStats(windowSize);
            auto startTime = testStats.GetTimestampOfLastEvent();

            for (size_t i = 0; i < eventCount; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(Random::Number<unsigned>(3, 77)));
                testStats.AddValue();

                // We don't want to include events that are no longer in our buffer
                // in the comparison, startTime -> endTime should be the events
                // in the buffer only or the check will be unfair and could fail
                // due to the inherent unreliableness of sleeping a thread for an
                // exact time period
                if (eventCount > windowSize && i < eventCount - windowSize) {
                    startTime = testStats.GetTimestampOfLastEvent();
                }
            }

            auto endTime = testStats.GetTimestampOfLastEvent();
            double expectedFrequency = GetHz(startTime, endTime, std::min(eventCount, windowSize));
            REQUIRE_THAT(testStats.MeanHz(), Catch::Matchers::WithinRel(expectedFrequency, 0.000001));
        }
    }

    SECTION("Clustered events")
    {
        constexpr size_t windowSize = 10;

        for (size_t eventCount = 2; eventCount < windowSize * 2; ++eventCount) {
            WindowedFrequencyStatistics testStats(windowSize);
            auto startTime = testStats.GetTimestampOfLastEvent();

            for (size_t i = 0; i < eventCount - 1; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(3));
                testStats.AddValue();

                // We don't want to include events that are no longer in our buffer
                // in the comparison, startTime -> endTime should be the events
                // in the buffer only or the check will be unfair and could fail
                // due to the inherent unreliableness of sleeping a thread for an
                // exact time period
                if (eventCount > windowSize && i < eventCount - windowSize) {
                    startTime = testStats.GetTimestampOfLastEvent();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(77 * eventCount));
            testStats.AddValue();

            auto endTime = testStats.GetTimestampOfLastEvent();
            double expectedFrequency = GetHz(startTime, endTime, std::min(eventCount, windowSize));
            REQUIRE_THAT(testStats.MeanHz(), Catch::Matchers::WithinRel(expectedFrequency, 0.000001));
        }
    }
}
