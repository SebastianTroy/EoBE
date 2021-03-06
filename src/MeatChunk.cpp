#include "MeatChunk.h"

#include <RangeConverter.h>

#include <QPainter>

MeatChunk::MeatChunk(const Energy& energy, const Transform& transform, const double& speed)
    : Entity(transform, GetMeatChunkRadius(energy), QColor::fromRgb(184, 68, 68), energy, speed)
{
}

MeatChunk::~MeatChunk()
{
}

double MeatChunk::GetMeatChunkRadius(const Energy& energy)
{
    // Size range 0.5->3.5 scaling linearly with energy quantity
    Tril::RangeConverter energyToSize({ 1_mj, 20_mj }, { 0.5, 3.5 });
    return energyToSize.ConvertAndClamp(energy);
}

std::string_view MeatChunk::GetDescription() const
{
    return "<p>Meat chunks can be eaten by trilobytes as a source of energy. "
           "When a trilobyte loses all of its health, all remaining energy is converted into meat chunks, "
           "which are scattered near the body of the deceased trilobyte.</p>";
}

void MeatChunk::TickImpl(EntityContainerInterface& /*container*/, const UniverseParameters& /*universeParameters*/)
{
    UseEnergy(GetEnergy() / 500.0);
    SetRadius(GetMeatChunkRadius(GetEnergy()));
    if (GetEnergy() < 1_mj) {
        Terminate();
    }
}
