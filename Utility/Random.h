#ifndef RANDOM_H
#define RANDOM_H

#include "Shape.h"
#include "MathConstants.h"

#include <random>
#include <limits>
#include <vector>
#include <algorithm>

class Random {
public:
    template<typename T>
    class WeightedContainer {
    public:
        void PushBack(T&& value, const double& weight)
        {
            values_.push_back(std::move(value));
            weights_.push_back(weight);
            distribution_ = std::discrete_distribution<size_t>(std::begin(weights_), std::end(weights_));
        }

        const T& RandomItem()
        {
            auto index = Random::Generate(distribution_);
            return values_.at(index);
        }

    private:
        std::vector<T> values_;
        std::vector<double> weights_;
        std::discrete_distribution<size_t> distribution_;
    };

    static void Seed(const std::mt19937::result_type& seed)
    {
        entropy_.seed(seed);
    }

    static double Bearing()
    {
        return Random::Number(0.0, Tril::Tau);
    }

    static bool Boolean()
    {
        static std::bernoulli_distribution d;
        return Generate(d);
    }

    template<typename NumericType>
    static NumericType Sign(const NumericType& value)
    {
        return Boolean() ? value : -value;
    }

    static Point PointInCircle(const Circle& circle)
    {
        const double rotation = Random::Number(0.0, Tril::Tau);
        const double distance = std::sqrt(Random::Number(0.0, 1.0)) * circle.radius;
        const double x = circle.x + distance * std::cos(rotation);
        const double y = circle.y + distance * std::sin(rotation);
        return { x, y };
    }

    template<typename NumericType>
    static NumericType Number(NumericType min, NumericType max)
    {
        if constexpr (std::is_integral<NumericType>::value) {
            static std::uniform_int_distribution<NumericType> distribution;
            distribution.param(typename decltype(distribution)::param_type(min, max));
            return Generate(distribution);
        } else if constexpr (std::is_floating_point<NumericType>::value) {
            static std::uniform_real_distribution<NumericType> distribution;
            distribution.param(typename decltype(distribution)::param_type(min, std::nextafter(max, std::numeric_limits<NumericType>::max())));
            return Generate(distribution);
        } else {
            static_assert(std::is_floating_point<NumericType>::value, "Random::Number requires an integral OR floating point number type to work.");
        }
    }

    template<typename NumericType>
    static NumericType Number(const Tril::Range<NumericType>& range)
    {
        return Number<NumericType>(range.Min(), range.Max());
    }

    static size_t WeightedIndex(std::initializer_list<double>&& weights)
    {
        std::discrete_distribution<size_t> d(std::move(weights));
        return Generate(d);
    }

    static double Percent()
    {
        return Number(0.0, 100.0);
    }

    static bool PercentChance(double chance)
    {
        std::bernoulli_distribution d(chance / 100.0);
        return Generate(d);
    }

    /**
     * Instead of rounding n.0 to n.49... down and n.5 to n.9... up, this
     * function uses the value after the decimal as the propotional chance the
     * value will be rounded up. e.g. 0.1 has a 10% chance of being rounded up
     * to 1, whereas 0.9 has a 90% chance of being rounded up.
     */
    static uint64_t Round(const double& v)
    {
        return static_cast<uint64_t>(v) + (Random::PercentChance(std::fmod(v, 1.0) * 100) ? 1 : 0);
    }

    template<typename NumericType>
    static NumericType Gaussian(NumericType mean = std::numeric_limits<NumericType>::min(), NumericType standardDeviation = NumericType{ 1.0 })
    {
        static std::normal_distribution<NumericType> distribution;
        distribution.param(typename std::normal_distribution<NumericType>::param_type(mean, std::abs(standardDeviation)));
        return Generate(distribution);
    }

    template<typename NumericType>
    static NumericType Poisson(NumericType mean = std::numeric_limits<NumericType>::min())
    {
        static std::poisson_distribution<NumericType> distribution;
        distribution.param(typename std::poisson_distribution<NumericType>::param_type(mean));
        return Generate(distribution);
    }

    template<typename NumericType>
    static std::vector<NumericType> Numbers(typename std::vector<NumericType>::size_type count, NumericType min, NumericType max)
    {
        std::vector<NumericType> rands;
        rands.reserve(count);

        if constexpr (std::is_integral<NumericType>::value) {
            static std::uniform_int_distribution<NumericType> distribution;
            distribution.param(typename std::uniform_int_distribution<NumericType>::param_type(min, max));

            std::generate_n(std::back_inserter(rands), count, [&](){ return Generate(distribution); });
        } else if constexpr (std::is_floating_point<NumericType>::value) {
            static std::uniform_real_distribution<NumericType> distribution;
            distribution.param(typename std::uniform_real_distribution<NumericType>::param_type(min, max));

            std::generate_n(std::back_inserter(rands), count, [&](){ return Generate(distribution); });
        } else {
            static_assert(std::is_floating_point<NumericType>::value, "Random::Numbers requires an integral OR floating point number type to work.");
        }

        return rands;
    }

    template<typename NumericType>
    static std::vector<NumericType> Gaussians(typename std::vector<NumericType>::size_type count, NumericType mean = std::numeric_limits<NumericType>::min(), NumericType standardDeviation = NumericType{ 1.0 })
    {
        static std::normal_distribution<NumericType> distribution;
        distribution.param(typename std::normal_distribution<NumericType>::param_type(mean, standardDeviation));

        std::vector<NumericType> rands;
        rands.reserve(count);
        std::generate_n(std::back_inserter(rands), count, [&](){ return Generate(distribution); });
        return rands;
    }

    template<typename NumericType>
    static std::vector<NumericType> DualPeakGaussians(typename std::vector<NumericType>::size_type count, NumericType meanPeakOne, NumericType standardDeviationPeakOne, NumericType meanPeakTwo, NumericType standardDeviationPeakTwo)
    {
        static std::normal_distribution<NumericType> distributionOne;
        static std::normal_distribution<NumericType> distributionTwo;
        distributionOne.param(typename std::normal_distribution<NumericType>::param_type(meanPeakOne, std::abs(standardDeviationPeakOne)));
        distributionTwo.param(typename std::normal_distribution<NumericType>::param_type(meanPeakTwo, std::abs(standardDeviationPeakTwo)));

        std::vector<NumericType> rands;
        rands.reserve(count);
        std::generate_n(std::back_inserter(rands), count, [&](){ return Random::Boolean() ? Generate(distributionOne) : Generate(distributionTwo); });
        return rands;
    }

    template<typename NumericType>
    static std::vector<NumericType> Poissons(typename std::vector<NumericType>::size_type count, NumericType mean = std::numeric_limits<NumericType>::min())
    {
        static std::poisson_distribution<NumericType> distribution;
        distribution.param(typename std::poisson_distribution<NumericType>::param_type(mean));

        std::vector<NumericType> rands;
        rands.reserve(count);
        std::generate_n(std::back_inserter(rands), count, [&](){ return Generate(distribution); });
        return rands;
    }

    template<typename NumericType>
    static NumericType GaussianAdjustment(const NumericType& toAdjust, double proportion)
    {
        return toAdjust + Gaussian(0.0, std::abs(toAdjust) * (proportion / 3.0));
    }

    template<typename Container>
    static void Shuffle(Container& toShuffle)
    {
        std::shuffle(std::begin(toShuffle), std::end(toShuffle), entropy_);
    }

    template<typename Container>
    static typename Container::value_type& Item(Container& container)
    {
        assert(!container.empty());
        auto iter = std::begin(container);
        std::advance(iter, Random::Number(typename Container::size_type{ 0 }, container.size() - 1));
        return *iter;
    }

    template<typename Container>
    static const typename Container::value_type& Item(const Container& container)
    {
        assert(!container.empty());
        auto iter = std::cbegin(container);
        std::advance(iter, Random::Number(typename Container::size_type{ 0 }, container.size() - 1));
        return *iter;
    }

    template<typename Container>
    static void ForNItems(const Container& container, size_t itemCount, const std::function<void(const typename Container::value_type& item)>& action)
    {
        assert(!container.empty());
        for (size_t count = 0; count < itemCount; ++count) {
            std::invoke(action, Item(container));
        }
    }

private:
    inline static std::mt19937 entropy_ = std::mt19937();

    template<typename DistributionType>
    static typename DistributionType::result_type Generate(DistributionType& distribution)
    {
        return distribution(entropy_);
    }
};

#endif // RANDOM_H
