#ifndef SENSE_H
#define SENSE_H

class EntityContainerInterface;
class QPainter;

#include "UniverseParameters.h"

#include <NeuralNetwork.h>
#include <NeuralNetworkConnector.h>

#include <fmt/format.h>

#include <string_view>
#include <memory>

class Trilobyte;

/**
 * All senses contain a small Neural network which is propogated each tick.
 */
class Sense {
public:
    Sense(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner);
    virtual ~Sense();

    virtual std::string_view GetName() const = 0;
    virtual std::string GetDescription() const = 0;

    virtual void PrimeInputs(std::vector<double>& inputs, const EntityContainerInterface& entities, const UniverseParameters& universeParameters) const = 0;

    virtual void Draw(QPainter& paint) const;
    virtual void Tick(std::vector<double>& outputs, const EntityContainerInterface& entities, const UniverseParameters& universeParameters) final;

    unsigned GetOutputCount() const { return network_->GetOutputCount(); }

    const NeuralNetwork& Inspect() const { return *network_; }
    const NeuralNetworkConnector& InspectConnection() const { return *outputConnections_; }

protected:
    const Trilobyte& owner_;

private:
    std::shared_ptr<NeuralNetwork> network_;
    std::shared_ptr<NeuralNetworkConnector> outputConnections_;
    std::vector<double> inputs_;

    virtual void PrepareToPrime() {}
};

template<>
struct fmt::formatter<Sense>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& context)
    {
        return context.begin();
    }

    template <typename FormatContext>
    auto format(const Sense& sense, FormatContext& context)
    {
        return fmt::format_to(context.out(), "{} inputs, {} layers", sense.Inspect().GetInputCount(), sense.Inspect().GetLayerCount());
    }
};

#endif // SENSE_H
