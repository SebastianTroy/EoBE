#include "EffectorFilterMouth.h"

#include "EntityContainerInterface.h"
#include "Trilobyte.h"

#include <QPainter>

EffectorFilterMouth::EffectorFilterMouth(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, Trilobyte& owner)
    : Effector(network, inputConnections, owner)
{
}

EffectorFilterMouth::~EffectorFilterMouth()
{
}

std::string EffectorFilterMouth::GetDescription() const
{
    return "<p>The filter mouth consumes any entity on contact, so long as it "
    "is small enough. It consumes food with a 75% energy efficiency.</p>";
}

Energy EffectorFilterMouth::PerformActions(const std::vector<double>& /*actionValues*/, EntityContainerInterface& entities, const UniverseParameters& /*universeParameters*/)
{
    // This is stupid, no point in having mouth closed ever for any reason
    //static Tril::RangeConverter inputToMouthOpen{ { -1.0, 1.0 }, { 0.0, 1.0 } };

    //const double mouthOpenProportion = inputToMouthOpen.ConvertAndClamp(actionValues.at(0));
    Energy foodEaten = 0_j;
    entities.ForEachCollidingWith(Circle{ owner_.GetTransform().x, owner_.GetTransform().y, owner_.GetRadius() }, [&](const std::shared_ptr<Entity>& entity)
    {
        if (entity->GetRadius() <= FOOD_RADIUS_THRESHOLD /*&& Random::PercentChance(mouthOpenProportion * 100.0)*/) {
            foodEaten += entity->GetEnergy();
            owner_.FeedOn(*entity, entity->GetEnergy());
        }
    });

    // Filter mouth is only 75% efficient, so burn 25% of what was eaten
    return foodEaten * 0.25;
}
