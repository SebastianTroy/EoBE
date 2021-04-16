#include "Spike.h"
#include "Swimmer.h"

#include <QPainter>

Spike::Spike(const Transform& transform) :
    Entity(transform, 12, QColor::fromRgb(225, 225, 225))
{
}

Spike::~Spike()
{
}

std::string_view Spike::GetDescription() const
{
    return "<p>Spikes add variety to the environment. "
           "Swimmers will have to evolve strategies for avoiding them, or minimising the damage they cause. "
           "By creating areas with different survival pressures you can encourage Swimmers to speciate to more efficiently fill each unique niche</p>";
}

void Spike::TickImpl(EntityContainerInterface& container, const UniverseParameters& /*universeParameters*/)
{
    container.ForEachCollidingWith(Circle{ GetLocation().x, GetLocation().y, GetRadius() }, [](const std::shared_ptr<Entity>& entity)
    {
        if (Swimmer* swimmer = dynamic_cast<Swimmer*>(entity.get())) {
            // FIXME entity rotation and direction of movement may not actually be the same! (they were when writing, but perhaps that should change!)
            Vec2 victimVec = GetMovementVector(entity->GetTransform().rotation, entity->GetVelocity());

            auto [ contactBearing, contactVelocity ] = DeconstructMovementVector({ -victimVec.x, -victimVec.y });
            (void) contactBearing; // unused
            swimmer->ApplyDamage(std::pow(contactVelocity, 2.0));
        }
    });
}

void Spike::DrawExtras(QPainter& /*paint*/)
{
}

std::vector<Property> Spike::CollectProperties() const
{
    return std::vector<Property>{
        {
            "Damage",
            []() -> std::string
            {
                return "SwimmerVelocity²";
            },
            "The damage dealt to a swimmer per tick is equal to the velocity of that swimmer squared.",
        }
    };
}
