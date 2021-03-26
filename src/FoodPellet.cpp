#include "FoodPellet.h"

#include "FeedDispenser.h"
#include <RangeConverter.h>

#include <QPainter>

double FoodPellet::GetPelletRadius(const Energy& energy)
{
    static Tril::RangeConverter energyToSize({ 1_mj, 30_mj }, { 0.5, 2.5 });
    return energyToSize.Convert(energy);
}

FoodPellet::FoodPellet(const std::shared_ptr<FeedDispenser>& spawner, Energy energy, const Transform& transform)
    : Entity(transform, GetPelletRadius(energy), QColor::fromRgb(15, 235, 15), energy)
    , spawner_(spawner)
{
}

FoodPellet::~FoodPellet()
{
    spawner_->PelletEaten();
}

std::string_view FoodPellet::GetDescription() const
{
    return "<p>Food pellets are spawned into the game by 'Feed Dispensers'. "
           "The can be eaten by swimmers as a source of energy.</p>";
}

void FoodPellet::DrawImpl(QPainter& paint)
{
    paint.drawEllipse(QPointF{ GetTransform().x, GetTransform().y }, GetRadius(), GetRadius());
}
