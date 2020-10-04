#ifndef SENSEENTITIESTOUCHING_H
#define SENSEENTITIESTOUCHING_H

#include "Sense.h"
#include "Entity.h"

class QPainter;

class SenseEntitiesTouching : public Sense {
    public:
        SenseEntitiesTouching(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Swimmer& owner, Transform transform, double genericDetectionWeight, const std::vector<std::pair<double, Trait>>&& toDetect);

        virtual std::string_view GetName() const override { return "SenseEntitiesTouching"; }
        virtual void PrimeInputs(std::vector<double>& inputs, const EntityContainerInterface& entities) const override;

        virtual void Draw(QPainter& paint) const override;

    private:
        Transform transform_;
        double genericDetectionWeight_;
        std::vector<std::pair<double, Trait>> toDetect_;
};

#endif // SENSEENTITIESTOUCHING_H