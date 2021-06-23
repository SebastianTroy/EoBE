#include "FoodPellet.h"

#include <RangeConverter.h>

#include <QPainter>

double FoodPellet::GetPelletRadius(const Energy& energy)
{
    Tril::RangeConverter energyToSize({ 1_mj, 30_mj }, { 0.5, 2.5 });
    return energyToSize.Convert(energy);
}

FoodPellet::FoodPellet(Energy energy, const Transform& transform)
    : Entity(transform, GetPelletRadius(energy), QColor::fromRgb(15, 235, 15), energy)
{
}

std::string_view FoodPellet::GetDescription() const
{
    return "<p>Food pellets are spawned into the game by 'Spawners'. "
           "They can be eaten by trilobytes as a source of energy.</p>";
}
