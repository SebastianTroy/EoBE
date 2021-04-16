#ifndef GENEPIGMENT_H
#define GENEPIGMENT_H

#include "Gene.h"
#include <TypeName.h>

class GenePigment : public Gene {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GenePigment>()); }
    static std::shared_ptr<Gene> Generate();
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GenePigment();
    GenePigment(double a, double r, double g, double b, double dominance);
    virtual ~GenePigment() override;

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

private:
    static const inline std::string KEY_A = "a";
    static const inline std::string KEY_R = "r";
    static const inline std::string KEY_G = "g";
    static const inline std::string KEY_B = "b";

    double a_;
    double r_;
    double g_;
    double b_;
};

#endif // GENEPIGMENT_H
