#ifndef SENSETRAITSSELF_H
#define SENSETRAITSSELF_H

#include "SenseTraitsBase.h"

class SenseTraitsSelf : public SenseTraitsBase {
public:
    SenseTraitsSelf(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, std::vector<TraitNormaliser>&& toDetect);
    virtual ~SenseTraitsSelf() override;

    virtual std::string_view GetName() const override { return "TraitsSelf"; }
    virtual std::string GetDescription() const override;

private:
    virtual void FilterEntities(const EntityContainerInterface& /*entities*/, const std::function<void (const Entity&)>& forEachEntity) const override;
};

#endif // SENSETRAITSSELF_H
