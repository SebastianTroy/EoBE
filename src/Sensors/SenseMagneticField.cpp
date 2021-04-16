#include "SenseMagneticField.h"

#include "Trilobyte.h"
#include <Shape.h>

#include <MathConstants.h>

#include <QPainter>

#include <iomanip>

SenseMagneticField::SenseMagneticField(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, const Point& target, const Tril::RangeConverter& distanceNormaliser, const Tril::Range<double>& frontBackNormaliser, const Tril::Range<double>& leftRightNormaliser)
    : Sense(network, outputConnections, owner)
    , target_(target)
    , distanceNormaliser_(distanceNormaliser)
    , frontBackNormaliser_({0, Tril::Pi}, frontBackNormaliser)
    , leftRightNormaliser_({0, Tril::Pi}, leftRightNormaliser)
{
}

std::string SenseMagneticField::GetDescription() const
{
    std::stringstream desc;
    desc << std::fixed << std::setprecision(2);

    desc << "<p>This sense allows the owner to track itself relative to the "
            "fixed point {" << target_.x << ", " << target_.y << "}. The three "
            "inputs correspont to:</p>"
            "<p><ol>"
              "<li>Distance to the point, Detected range [" << distanceNormaliser_.GetFrom().First() << "->" << distanceNormaliser_.GetFrom().Last() << "],"
                                        " Normalised range [" << distanceNormaliser_.GetTo().First() << "->" << distanceNormaliser_.GetTo().Last() << "]</li>"
              "<li>Orientation, Detected range [front->back],"
                              " Normalised range [" << frontBackNormaliser_.GetTo().First() << "->" << frontBackNormaliser_.GetTo().Last() << "]</li>"
              "<li>Orientation, Detected range [left->right],"
                              " Normalised range [" << frontBackNormaliser_.GetTo().First() << "->" << frontBackNormaliser_.GetTo().Last() << "]</li>"
            "</ol></p>"
            "<p>Orientation is implemented as two inputs as it allows for more "
            "intuitive input values.</p>";
    return desc.str();
}

void SenseMagneticField::PrimeInputs(std::vector<double>& inputs, const EntityContainerInterface&, const UniverseParameters& /*universeParameters*/) const
{
    double distance = GetDistance(Point{ owner_.GetTransform().x, owner_.GetTransform().y }, target_);
    double bearing = GetBearing(Point{ owner_.GetTransform().x, owner_.GetTransform().y }, target_);

    // Distance to the target location
    inputs.at(0) = distanceNormaliser_.ConvertAndClamp(distance);
    // 1 Facing target, -1 back to target (left/right both 0)
    inputs.at(1) = BearingToFrontBack(bearing);
    // 1 Left facing target, -1 right facing target (front/back both 0)
    inputs.at(2) = BearingToLeftRight(bearing);
}

void SenseMagneticField::Draw(QPainter& paint) const
{
    Point begin{ owner_.GetTransform().x, owner_.GetTransform().y };
    double distance = GetDistance(begin, target_);
    double bearing = GetBearing(begin, target_);

    // Adjust distance so it can be drawn reasonably
    distance = 30 * (distance / distanceNormaliser_.GetFrom().Max());

    Point end = ApplyOffset(begin, bearing, distance);

    paint.setPen(Qt::blue);
    paint.drawLine(QLineF(begin.x, begin.y, end.x, end.y));
}

double SenseMagneticField::BearingToFrontBack(double bearing) const
{
    // Get bearing from direction facing to target
    bearing -= owner_.GetTransform().rotation;

    // Ensure we are still in the range if 0 to Tau
    if (bearing < 0.0) {
        bearing += Tril::Tau;
    }

    // Convert range from 0-Tau to 0 to Pi
    bearing = std::abs(bearing - Tril::Pi);

    return frontBackNormaliser_.Convert(bearing);
}

double SenseMagneticField::BearingToLeftRight(double bearing) const
{
    // Get bearing from right side to target
    bearing -= owner_.GetTransform().rotation + (Tril::Tau / 4.0);

    // Ensure we are still in the range if 0 to Tau
    while (bearing < 0.0) {
        bearing += Tril::Tau;
    }

    // Convert range from 0-Tau to 0 to Pi
    bearing = std::abs(bearing - Tril::Pi);

    return leftRightNormaliser_.Convert(bearing);
}
