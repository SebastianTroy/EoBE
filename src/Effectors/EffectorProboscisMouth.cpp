#include "EffectorProboscisMouth.h"

#include "Trilobyte.h"
#include "Egg.h"

#include <QPainter>

#include <iomanip>

EffectorProboscisMouth::EffectorProboscisMouth(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, Trilobyte& owner, const double& proboscisLength)
    : Effector(network, inputConnections, owner)
    , proboscisLength_(proboscisLength)
{
}

EffectorProboscisMouth::~EffectorProboscisMouth()
{
}

std::string EffectorProboscisMouth::GetDescription() const
{
    std::stringstream desc;
    desc << "<p>The proboscis allows an entity to leech energy directly from "
    "other trilobytes, or eggs. Energy is consumed with 100% efficiency but the "
    "rate at which energy is stolen is limited.</p>";
    desc << std::fixed << std::setprecision(1);
    desc << "<p>Length: " << proboscisLength_ << "</p>";

    return desc.str();
}

void EffectorProboscisMouth::Draw(QPainter& paint) const
{
    Line proboscis = GetProboscis();
    paint.drawLine(QLineF(QPointF(proboscis.a.x, proboscis.a.y), QPointF(proboscis.b.x, proboscis.b.y)));
}

Energy EffectorProboscisMouth::PerformActions(const std::vector<double>& /*actionValues*/, EntityContainerInterface& entities, const UniverseParameters& /*universeParameters*/)
{
    Line proboscis = GetProboscis();
    std::shared_ptr<Entity> victim;
    entities.ForEachCollidingWith(proboscis.b, [&](const std::shared_ptr<Entity>& entity)
    {
        if (dynamic_cast<Trilobyte*>(entity.get()) || dynamic_cast<Egg*>(entity.get())) {
            if (!victim || entity->GetEnergy() > victim->GetEnergy()) {
                victim = entity;
            }
        }
    });

    if (victim) {
        Energy quantity = std::min(30_mj, victim->GetEnergy());
        owner_.FeedOn(*victim, quantity);
        return quantity;
    } else {
        return 0_j;
    }
}

Line EffectorProboscisMouth::GetProboscis() const
{
    Point start = ApplyOffset(owner_.GetLocation(), owner_.GetTransform().rotation, owner_.GetRadius());
    Point end = ApplyOffset(start, owner_.GetTransform().rotation, proboscisLength_);
    return Line{ start, end };
}
