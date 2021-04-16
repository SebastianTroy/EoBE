#include "SenseTraitsRaycast.h"

#include "Trilobyte.h"
#include "EntityContainerInterface.h"

#include <QPainter>

#include <iomanip>
#include <sstream>

SenseTraitsRaycast::SenseTraitsRaycast(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, std::vector<TraitNormaliser>&& toDetect, const Transform& transform, double maxDistance, double angle)
    : SenseTraitsBase(network, outputConnections, owner, transform, std::move(toDetect))
    , rayCastDistance_(maxDistance)
    , rayCastAngle_(angle)
{
}

std::string SenseTraitsRaycast::GetDescription() const
{
    std::stringstream desc;
    desc << std::fixed << std::setprecision(2);

    desc << "<p>This sense detects the nearest entity it collides with, within "
            "its range of " << rayCastDistance_ << " pixels.</p>";

    desc << "<p>Inputs:<ol>";
    for (const TraitNormaliser& trait : GetDetectableTraits()) {
        desc << "<li>" << ToString(trait.trait) << ", "
             << "Detection range[" << trait.range.GetFrom().First() << "->" << trait.range.GetFrom().Last() << "]"
             << "Normalised range[" << trait.range.GetTo().First() << "->" << trait.range.GetTo().Last() << "]"
             << "</li>";
    }
    desc << "</ol></p>";

    return desc.str();
}

void SenseTraitsRaycast::Draw(QPainter& paint) const
{
    auto l = GetLine();
    paint.setPen(QColor::fromRgb(0, 0, 0));
    paint.drawLine(QLineF(l.a.x, l.a.y, l.b.x, l.b.y));
}

Line SenseTraitsRaycast::GetLine() const
{
    // TODO this is really why I need to get on with implementing a real transform class...
    Point begin = ApplyOffset({ owner_.GetTransform().x, owner_.GetTransform().y }, transform_.rotation + owner_.GetTransform().rotation, GetDistance({0, 0}, {transform_.x, transform_.y}));
    Point end = ApplyOffset(begin, transform_.rotation + owner_.GetTransform().rotation + rayCastAngle_, rayCastDistance_);
    return { begin, end };
}

void SenseTraitsRaycast::FilterEntities(const EntityContainerInterface& entities, const std::function<void (const Entity& e)>& forEachEntity) const
{
    Line rayCastLine = GetLine();
    std::shared_ptr<Entity> nearestEntity;
    double distanceToNearestSquared = 0.0;
    entities.ForEachCollidingWith(rayCastLine, [&](const std::shared_ptr<Entity>& e)
    {
        // don't detect ourself
        if (e.get() != &owner_) {
            // TODO this distance assumes head on detection, not incidental ones
            double distanceSquare = GetDistanceSquare(rayCastLine.a, e->GetLocation());
            if (nearestEntity == nullptr || distanceSquare < distanceToNearestSquared) {
                nearestEntity = e;
                distanceToNearestSquared = distanceSquare;
            }
        }
    });

    if (nearestEntity != nullptr) {
        // Only nearest entity detected
        forEachEntity(*nearestEntity);
    }
}
