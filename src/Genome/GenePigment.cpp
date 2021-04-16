#include "GenePigment.h"

#include "Trilobyte.h"
#include <Random.h>

#include <QPixmap>
#include <QPainter>

using namespace nlohmann;

std::shared_ptr<Gene> GenePigment::Generate()
{
    return std::make_shared<GenePigment>();
}

GenePigment::GenePigment()
    : GenePigment(Random::Number(0.0, 1.0), Random::Number(0.0, 1.0), Random::Number(0.0, 1.0), Random::Number(0.0, 1.0), Random::Number(0.0, 100.0))
{
}

GenePigment::GenePigment(double a, double r, double g, double b, double dominance)
    : Gene(dominance)
    , a_(std::clamp(a, 0.0, 1.0))
    , r_(std::clamp(r, 0.0, 1.0))
    , g_(std::clamp(g, 0.0, 1.0))
    , b_(std::clamp(b, 0.0, 1.0))
{
    // Mutate Alpha
    AddMutation(BASE_WEIGHT, [&]()
    {
        a_ = std::clamp(a_ + Random::Gaussian(0.0, 0.1), 0.0, 1.0);
    });

    // Mutate Red
    AddMutation(BASE_WEIGHT, [&]()
    {
        r_ = std::clamp(r_ + Random::Gaussian(0.0, 0.1), 0.0, 1.0);
    });
    // Mutate Green
    AddMutation(BASE_WEIGHT, [&]()
    {
        g_ = std::clamp(g_ + Random::Gaussian(0.0, 0.1), 0.0, 1.0);
    });
    // Mutate Blue
    AddMutation(BASE_WEIGHT, [&]()
    {
        b_ = std::clamp(b_ + Random::Gaussian(0.0, 0.1), 0.0, 1.0);
    });

    // Mutate all
    AddMutation(0.25 * BASE_WEIGHT, [&]()
    {
        a_ = std::clamp(a_ + Random::Gaussian(0.0, 0.1), 0.0, 1.0);
        r_ = std::clamp(r_ + Random::Gaussian(0.0, 0.1), 0.0, 1.0);
        g_ = std::clamp(g_ + Random::Gaussian(0.0, 0.1), 0.0, 1.0);
        b_ = std::clamp(b_ + Random::Gaussian(0.0, 0.1), 0.0, 1.0);
    });

    // Randomise colour
    AddMutation(0.01 * BASE_WEIGHT, [&]()
    {
        a_ = Random::Number(0.0, 1.0);
        r_ = Random::Number(0.0, 1.0);
        g_ = Random::Number(0.0, 1.0);
        b_ = Random::Number(0.0, 1.0);
    });
}

GenePigment::~GenePigment()
{
}

json GenePigment::Serialise() const
{
    return {
        {KEY_DOMINANCE, GetDominance()},
        {KEY_A, a_},
        {KEY_R, r_},
        {KEY_G, g_},
        {KEY_B, b_},
    };
}

std::shared_ptr<Gene> GenePigment::Copy() const
{
    return std::make_shared<GenePigment>(a_, r_, g_, b_, GetDominance());
}

void GenePigment::ExpressGene(Trilobyte& /*owner*/, Phenotype& target) const
{
    QPixmap canvas(1, 1);
    QPainter p(&canvas);

    p.setPen(Qt::NoPen);
    p.fillRect(canvas.rect(), Qt::white);
    p.fillRect(canvas.rect(), target.colour);
    p.fillRect(canvas.rect(), QColor::fromRgbF(r_, g_, b_, a_));

    target.colour = canvas.toImage().pixel(0, 0);
    target.baseMetabolism += 0.1_uj;
}
