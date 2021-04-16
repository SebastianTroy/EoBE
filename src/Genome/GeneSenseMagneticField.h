#ifndef GENESENSEMAGNETICFIELD_H
#define GENESENSEMAGNETICFIELD_H

#include "GeneSenseBase.h"
#include <Range.h>
#include <RangeConverter.h>
#include <Shape.h>
#include <TypeName.h>

class GeneSenseMagneticField : public GeneSenseBase {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GeneSenseMagneticField>()); }
    static std::shared_ptr<Gene> Generate(unsigned brainWidth);
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GeneSenseMagneticField(unsigned hiddenLayers, unsigned outputCount, const Point& point);
    GeneSenseMagneticField(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Point& target, const Tril::RangeConverter& distanceNormaliser, const Tril::Range<double>& frontBackNormaliser, const Tril::Range<double>& leftRightNormaliser, double dominance);
    virtual ~GeneSenseMagneticField() override {}

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;

private:
    static const inline std::string KEY_TARGET = "Target";
    static const inline std::string KEY_DISTANCE = "Distance";
    static const inline std::string KEY_FRONT_BACK = "FrontBack";
    static const inline std::string KEY_LEFT_RIGHT = "LeftRight";

    Point target_;
    Tril::RangeConverter distanceNormaliser_;
    Tril::Range<double> frontBackNormaliser_;
    Tril::Range<double> leftRightNormaliser_;
};

#endif // GENESENSEMAGNETICFIELD_H
