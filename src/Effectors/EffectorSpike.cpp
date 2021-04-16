#include "EffectorSpike.h"

#include "Trilobyte.h"
#include <Shape.h>

#include <QPainter>
#include <QPainterPath>
#include <QColor>

EffectorSpike::EffectorSpike(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, Trilobyte& owner, double bearing, double length)
    : Effector(network, inputConnections, owner)
    , bearing_(bearing)
    , length_(length)
{
}

EffectorSpike::~EffectorSpike()
{
}

std::string EffectorSpike::GetDescription() const
{
    return "<p>The spike applies damage to trilobytes that are colliding with the"
    " point. The quantity of damage is both speed and angle dependant, glancing"
    " or low speed collisions will not cause much damage, movements away from "
    "the point will cause no damage. Therefore it is safe to swim through a "
    "spike from base to tip, but dangerous to swim through a spike from tip to"
    "base.</p>";
}

void EffectorSpike::Draw(QPainter& paint) const
{
    Point tip = GetTipOfSpike();
    Point baseRight = ApplyOffset(tip, (bearing_ + owner_.GetTransform().rotation) - (Tril::Tau * 0.47), length_ + 2.0);
    Point baseLeft = ApplyOffset(tip, (bearing_ + owner_.GetTransform().rotation) + (Tril::Tau * 0.47), length_ + 2.0);

    QPainterPath spikeTriangle;
    spikeTriangle.moveTo(tip.x, tip.y);
    spikeTriangle.lineTo(baseRight.x, baseRight.y);
    spikeTriangle.lineTo(baseLeft.x, baseLeft.y);
    spikeTriangle.closeSubpath();
    paint.setBrush(QColor::fromRgb(225, 225, 225));
    paint.drawPath(spikeTriangle);
}

Energy EffectorSpike::PerformActions(const std::vector<double>& /*actionValues*/, const EntityContainerInterface& entities, const UniverseParameters& /*universeParameters*/)
{
    entities.ForEachCollidingWith(GetTipOfSpike(), [&](const std::shared_ptr<Entity>& entity)
    {
        if (Trilobyte* victim = dynamic_cast<Trilobyte*>(entity.get())) {
            Vec2 spikeVec = GetMovementVector(bearing_ + owner_.GetTransform().rotation, owner_.GetVelocity());
            // FIXME entity rotation and direction of movement may not actually be the same! (they were when writing, but perhaps that should change!)
            Vec2 victimVec = GetMovementVector(entity->GetTransform().rotation, entity->GetVelocity());

            auto [ contactBearing, contactVelocity ] = DeconstructMovementVector({ spikeVec.x - victimVec.x, spikeVec.y - victimVec.y });

            double collisionBearing = std::fmod(std::abs(bearing_ - contactBearing), Tril::Tau);
            double directHitProportion = std::max(0.0, ((std::abs(collisionBearing - Tril::Pi) / Tril::Pi) - 0.5) * 2);
            victim->ApplyDamage(directHitProportion * (5 * std::pow(contactVelocity, 2.0)));
        }
    });

    // Spike is entirely passive, MAYBE add ability to apply venom for a cost (either constant or neuron controllable)
    return 0_j;
}

Point EffectorSpike::GetTipOfSpike() const
{
    return ApplyOffset(owner_.GetLocation(), bearing_ + owner_.GetTransform().rotation, owner_.GetRadius() + length_);
}
