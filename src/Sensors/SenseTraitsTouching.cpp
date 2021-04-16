#include "SenseTraitsTouching.h"

#include "Trilobyte.h"

#include <QPainter>

#include <iomanip>

SenseTraitsTouching::SenseTraitsTouching(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, Transform transform, std::vector<TraitNormaliser>&& toDetect)
    : SenseTraitsBase(network, outputConnections, owner, transform, std::move(toDetect))
{
}

std::string SenseTraitsTouching::GetDescription() const
{
    std::stringstream desc;
    desc << std::fixed << std::setprecision(2);

    desc << "<p>This sense detects all entities touching it, except the owner."
            "</p>"
            "<p>When multiple entities are touching this sense, the detected "
            "trait values are all summed, then the final total is normalised"
            "</p>";

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

void SenseTraitsTouching::Draw(QPainter& paint) const
{
    Point location = GetPoint();
    paint.setPen(QColor::fromRgb(255, 0, 0));
    paint.setBrush(QColor::fromRgb(0, 0, 0, 0));
    paint.drawEllipse(QPointF(location.x, location.y), 1.0, 1.0);
}

Point SenseTraitsTouching::GetPoint() const
{
    return ApplyOffset(owner_.GetLocation(), transform_.rotation + owner_.GetTransform().rotation, GetDistance({ 0, 0 }, { transform_.x, transform_.y }));
}

void SenseTraitsTouching::FilterEntities(const EntityContainerInterface& entities, const std::function<void (const Entity&)>& forEachEntity) const
{
    Point location = GetPoint();
    entities.ForEachCollidingWith(location, [&](const std::shared_ptr<Entity>& e)
    {
        // don't detect ourself
        if (e.get() != &owner_) {
            // All entities touching are detected
            forEachEntity(*e);
        }
    });
}
