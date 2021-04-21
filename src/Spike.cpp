#include "Spike.h"
#include "Trilobyte.h"

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
           "Trilobytes will have to evolve strategies for avoiding them, or minimising the damage they cause. "
           "By creating areas with different survival pressures you can encourage Trilobytes to speciate to more efficiently fill each unique niche</p>";
}

void Spike::TickImpl(EntityContainerInterface& container, const UniverseParameters& /*universeParameters*/)
{
    container.ForEachCollidingWith(Circle{ GetLocation().x, GetLocation().y, GetRadius() }, [](const std::shared_ptr<Entity>& entity)
    {
        if (Trilobyte* trilobyte = dynamic_cast<Trilobyte*>(entity.get())) {
            // FIXME entity rotation and direction of movement may not actually be the same! (they were when writing, but perhaps that should change!)
            Vec2 victimVec = GetMovementVector(entity->GetTransform().rotation, entity->GetVelocity());

            auto [ contactBearing, contactVelocity ] = DeconstructMovementVector({ -victimVec.x, -victimVec.y });
            (void) contactBearing; // unused
            trilobyte->ApplyDamage(std::pow(contactVelocity, 2.0));
        }
    });
}

std::vector<Property> Spike::CollectProperties() const
{
    return std::vector<Property>{
        {
            "Damage",
            []() -> std::string
            {
                return "TrilobyteVelocity²";
            },
            "The damage dealt to a trilobyte per tick is equal to the velocity of that trilobyte squared.",
        }
    };
}
