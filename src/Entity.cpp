#include "Entity.h"

#include <Random.h>
#include <Algorithm.h>

#include <fmt/core.h>

#include <QPainter>

#include <assert.h>

Entity::Entity(const Transform& transform, double radius, QColor colour, Energy energy, double speed)
    : energy_(energy)
    , transform_(transform)
    , radius_(radius)
    , speed_(speed)
    , age_(0)
    , colour_(colour)
{
    assert(radius_ <= MAX_RADIUS);
}

Entity::~Entity()
{
}

std::vector<Property> Entity::GetProperties() const
{
    std::vector<Property> entityProperties{
        {
            "Name",
            [&]() -> std::string
            {
                return fmt::format("{}", this->GetName());
            },
            std::string(GetDescription()),
        },
        {
            "Energy",
            [&]() -> std::string
            {
                return fmt::format("{:.2f}mj", this->energy_ / 1_mj);
            },
            "The current energy available to the entity in milli-joules."
        },
        {
            "Exists",
            [&]() -> std::string
            {
                return fmt::format("{}", !this->terminated_);
            },
            "True if the entity is currently part of a simulation."
        },
        {
            "Location",
            [&]() -> std::string
            {
                return fmt::format("{:.2f}", this->transform_);
            },
            "The location of the entity in the simulation"
        },
        {
            "Size",
            [&]() -> std::string
            {
                return fmt::format("{:.2f}", this->radius_ * 2);
            },
            "The width of the entity. All entities are considered to be circular."
        },
        {
            "Velocity",
            [&]() -> std::string
            {
                return fmt::format("{:.2f}", this->speed_);
            },
            "The current velocity of the entity, in pixels per tick."
        },
        {
            "Age",
            [&]() -> std::string
            {
                return fmt::format("{}", this->age_);
            },
            "The number of ticks that this entity has existed within a simulation."
        },
        {
            "Colour",
            [&]() -> std::string
            {
                return fmt::format("R={}, G={}, B={}", this->colour_.red(), this->colour_.green(), this->colour_.blue());
            },
            "The colour that this entity will appear to be to other entities."
        },
    };

    return Tril::Combine(std::move(entityProperties), CollectProperties());
}

void Entity::FeedOn(Entity& other, Energy quantity)
{
    energy_ += other.TakeEnergy(quantity);
    if (other.GetEnergy() <= 0.0) {
        other.Terminate();
    }
}

bool Entity::Tick(EntityContainerInterface& container, const UniverseParameters& universeParameters)
{
    TickImpl(container, universeParameters);
    age_++;
    return Move(/* TODO user adjustable coefficient of friction */);
}

void Entity::Draw(QPainter& paint)
{
    paint.save();
    paint.setBrush(colour_);
    DrawImpl(paint);
    paint.restore();
}

Energy Entity::TakeEnergy(Energy quantity)
{
    Energy toGive = std::min(energy_, quantity);
    energy_ -= quantity;
    return toGive;
}

void Entity::SetBearing(double bearing)
{
    if (bearing < 0.0) {
        bearing += Tril::Tau;
    } else if (bearing > Tril::Tau) {
        bearing -= Tril::Tau;
    }
    transform_.rotation = bearing;
}

bool Entity::Move()
{
    if (std::abs(speed_) > 0.05) {
        Point newLocation = ApplyOffset({ transform_.x, transform_.y }, transform_.rotation, speed_);
        transform_.x = newLocation.x;
        transform_.y = newLocation.y;
        speed_ *= 0.9;
        return true;
    }
    return false;
}


