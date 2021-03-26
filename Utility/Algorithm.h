#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <vector>
#include <functional>
#include <cmath>
#include <assert.h>

namespace Tril {
template <typename T1, typename T2>
void IterateBoth(std::vector<T1>& a, std::vector<T2>& b, std::function<void(T1& a, T2& b)>&& action)
{
    auto aIter = a.begin();
    auto bIter = b.begin();
    for (; aIter != a.end() && bIter != b.end() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T1, typename T2>
void IterateBoth(const std::vector<T1>& a, const std::vector<T2>& b, std::function<void(const T1& a, const T2& b)>&& action)
{
    auto aIter = a.begin();
    auto bIter = b.begin();
    for (; aIter != a.end() && bIter != b.end() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T1, typename T2>
void IterateBoth(const std::vector<T1>& a, std::vector<T2>& b, std::function<void(const T1& a, T2& b)>&& action)
{
    auto aIter = a.begin();
    auto bIter = b.begin();
    for (; aIter != a.end() && bIter != b.end() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T1, typename T2>
void IterateBoth(std::vector<T1>& a, const std::vector<T2>& b, std::function<void(T1& a, const T2& b)>&& action)
{
    auto aIter = a.begin();
    auto bIter = b.begin();
    for (; aIter != a.end() && bIter != b.end() ; ++aIter, ++bIter) {
        action(*aIter, *bIter);
    }
}

template <typename T>
std::vector<T> CreateSeries(T firstValue, uint64_t count, std::function<T(const T& previous)>&& nextValue = [](const T& previous){ return previous + 1; })
{
    std::vector<T> series;
    series.resize(count);
    std::generate_n(std::begin(series), count, [&]() -> T
    {
        T temp = firstValue;
        firstValue = nextValue(temp);
        return temp;
    });
    return series;
}

template <typename T>
std::vector<T> CreateSeries(T firstValue, T lastValue, uint64_t count)
{
    assert(count >= 2);
    long double soFar = firstValue;
    long double increment = (lastValue - firstValue) / (count - 1);
    return CreateSeries<T>(soFar, count, [&](const T& /*prev*/) -> T
    {
        soFar += increment;
        if constexpr (std::is_integral<T>::value) {
            return static_cast<T>(std::round(soFar));
        } else {
            return static_cast<T>(soFar);
        }
    });
}

template <typename T>
std::vector<T> Combine(std::vector<T>&& first, std::vector<T>&& second)
{
    std::vector<T> combined;
    combined.swap(first);
    combined.reserve(combined.size() + second.size());
    std::move(std::begin(second), std::end(second), std::back_inserter(combined));
    return combined;
}

} // namespace Tril

#endif // ALGORITHM_H
