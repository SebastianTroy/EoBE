#include "WindowedRollingStatistics.h"


Tril::WindowedRollingStatistics::WindowedRollingStatistics(size_t windowSize)
    : values_(windowSize)
{
    Reset();
}

void Tril::WindowedRollingStatistics::AddValue(double newValue)
{
    sumOfValues_ += newValue;
    sumOfValuesSquared_ += newValue;
    min_ = std::min(min_, newValue);
    max_ = std::max(max_, newValue);

    if (values_.Full()) {
        double oldValue = values_.Oldest();
        sumOfValues_ -= oldValue;
        sumOfValuesSquared_ -= std::pow(oldValue, 2.0);
        updateMinMax = (oldValue != newValue) && ((oldValue == min_) || (oldValue == max_));
    }

    values_.PushBack(newValue);
}

void Tril::WindowedRollingStatistics::Reset()
{
    values_.Clear();
    sumOfValues_ = 0.0;
    sumOfValuesSquared_ = 0.0;
    min_ = std::numeric_limits<double>::max();
    max_ = std::numeric_limits<double>::lowest();
    updateMinMax = false;
}

double Tril::WindowedRollingStatistics::Min()
{
    if (updateMinMax) {
        UpdateMinMax();
    }
    return min_;
}

double Tril::WindowedRollingStatistics::Max()
{
    if (updateMinMax) {
        UpdateMinMax();
    }
    return max_;
}

double Tril::WindowedRollingStatistics::StandardDeviation() const
{
    if (Count() > 1) {
        return std::sqrt((sumOfValuesSquared_ - (std::pow(sumOfValues_, 2) / Count())) / Count());
    } else {
        return 0;
    }
}

void Tril::WindowedRollingStatistics::UpdateMinMax()
{
    updateMinMax = false;
    min_ = std::numeric_limits<double>::max();
    max_ = std::numeric_limits<double>::lowest();
    values_.ForEach([&](const double& value)
    {
        min_ = std::min(min_, value);
        max_ = std::max(max_, value);
    });
}
