#ifndef EFFECTOR_H
#define EFFECTOR_H

#include "UniverseParameters.h"

#include <Energy.h>
#include <NeuralNetwork.h>
#include <NeuralNetworkConnector.h>

#include <fmt/format.h>

#include <vector>

class Swimmer;
class EntityContainerInterface;
class UniverseInfoStructRef;
class QPainter;

class Effector {
public:
    Effector(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, Swimmer& owner);
    virtual ~Effector();

    virtual std::string_view GetName() const = 0;
    virtual std::string GetDescription() const = 0;

    virtual void Draw(QPainter& paint) const = 0;
    virtual Energy Tick(const std::vector<double>& inputs, const EntityContainerInterface& entities, const UniverseParameters& universeParameters) final;

    unsigned GetInputCount() const { return network_->GetInputCount(); }

    const NeuralNetwork& Inspect() const { return *network_; }
    const NeuralNetworkConnector& InspectConnection() const { return *inputConnections_; }

protected:
    Swimmer& owner_;

private:
    std::shared_ptr<NeuralNetwork> network_;
    std::shared_ptr<NeuralNetworkConnector> inputConnections_;
    std::vector<double> outputs_;

    virtual Energy PerformActions(const std::vector<double>& actionValues, const EntityContainerInterface& entities, const UniverseParameters& universeParameters) = 0;
};

template<>
struct fmt::formatter<Effector>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& context)
    {
        return context.begin();
    }

    template <typename FormatContext>
    auto format(const Effector& effector, FormatContext& context)
    {
        return fmt::format_to(context.out(), "{} inputs, {} layers", effector.Inspect().GetInputCount(), effector.Inspect().GetLayerCount());
    }
};

#endif // EFFECTOR_H
