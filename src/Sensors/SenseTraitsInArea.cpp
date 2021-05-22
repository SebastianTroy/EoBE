#include "SenseTraitsInArea.h"

#include "Trilobyte.h"

#include <QPainter>

#include <iomanip>

SenseTraitsInArea::SenseTraitsInArea(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, double radius, const Transform& transform, std::vector<TraitNormaliser>&& toDetect)
    : SenseTraitsBase(network, outputConnections, owner, transform, std::move(toDetect))
    , senseRadius_(radius)
{
}

std::string SenseTraitsInArea::GetDescription() const
{
    std::stringstream desc;
    desc << std::fixed << std::setprecision(2);

    desc << "<p>This sense detects every entity whose centre is within its "
            "radius of " << senseRadius_ << " pixels.</p>"
            "<p>When multiple entities are detected by this sense, the detected"
            " trait values are all summed, then the final total is normalised"
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
void SenseTraitsInArea::Draw(QPainter& paint) const
{
    Circle c = GetArea();
    paint.setPen(Qt::black);
    paint.setBrush(Qt::NoBrush);
    paint.drawEllipse(QPointF(c.x, c.y), senseRadius_, senseRadius_);
}

Circle SenseTraitsInArea::GetArea() const
{
    Point centre = ApplyOffset(owner_.GetLocation(), transform_.rotation + owner_.GetTransform().rotation, GetDistance({ 0, 0 }, { transform_.x, transform_.y }));
    return { centre.x, centre.y, senseRadius_ };
}

void SenseTraitsInArea::FilterEntities(const EntityContainerInterface& entities, const std::function<void (const Entity&)>& forEachEntity) const
{
    const Circle senseArea = GetArea();
    const Point senseCentre = { senseArea.x, senseArea.y };
    const double senseRadiusSquare = std::pow(senseArea.radius, 2.0);
    entities.ForEachCollidingWith(senseArea, [&](const Entity& e)
    {
        // don't detect ourself
        if (&e != &owner_) {
            double distanceSquare = GetDistanceSquare(senseCentre, e.GetLocation());
            if (distanceSquare < senseRadiusSquare) {
                forEachEntity(e);
            }
        }
    });
}
