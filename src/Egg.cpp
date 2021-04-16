#include "Egg.h"

#include "Trilobyte.h"

#include <QPainter>

Egg::Egg(std::shared_ptr<Trilobyte>&& mother, Energy energy, const Transform& transform, std::shared_ptr<Genome> genomeOne, std::shared_ptr<Genome> genomeTwo, unsigned hatchingDelay)
    : Entity(transform, 3.5, QColor::fromRgb(125, 57, 195), energy, mother->GetVelocity())
    , mother_(std::move(mother))
    , genomeOne_(genomeOne)
    , genomeTwo_(genomeTwo)
    , hatchingDelay_(hatchingDelay)
{
}

Egg::~Egg()
{
}

std::string_view Egg::GetDescription() const
{
    return "<p>Trilobytes can lay eggs. "
           "When creating an egg, the genetics of up to two trilobytes can be combined to create the genetics passed onto the egg.</p>";
}

void Egg::TickImpl(EntityContainerInterface& container, const UniverseParameters& universeParameters)
{
    // TODO could delay depending on ammount of energy in the egg
    // could delay based on the number of genes / their complexity
    // could add an iterator to genome, and each tich process the next gene, use some energy and hatch with remaining energy once done

    // TODO add a gene to decide how much energy to pass to eggs


    if (hatchingDelay_ > 0) {
        hatchingDelay_--;
    } else {
        // cross with self for now
        std::shared_ptr<Genome> genome = Genome::CreateOffspring(*genomeOne_, *genomeTwo_, universeParameters);
        if (genome) {
            container.AddEntity(std::make_shared<Trilobyte>(GetEnergy(), GetTransform(), genome, std::move(mother_)));
        }
        Terminate();
    }
}

void Egg::DrawExtras(QPainter& /*paint*/)
{
}

std::vector<Property> Egg::CollectProperties() const
{
    return std::vector<Property>{
        {
            "Delay",
            [&]() -> std::string
            {
                return fmt::format("{}", this->hatchingDelay_);
            },
            "The number of ticks until the egg hatches",
        },
    };
}
