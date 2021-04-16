#include "SenseTraitsSelf.h"

#include "Trilobyte.h"

#include <iomanip>

SenseTraitsSelf::SenseTraitsSelf(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect)
    : SenseTraitsBase(network, outputConnections, owner, {}, std::move(toDetect))
{
}

SenseTraitsSelf::~SenseTraitsSelf()
{
}

std::string SenseTraitsSelf::GetDescription() const
{
    std::stringstream desc;
    desc << std::fixed << std::setprecision(2);

    desc << "<p>This sense detects the owner's own traits.</p>";

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
void SenseTraitsSelf::FilterEntities(const EntityContainerInterface&, const std::function<void (const Entity&)>& forEachEntity) const
{
    forEachEntity(owner_);
}
