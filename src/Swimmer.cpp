#include "Swimmer.h"

#include "FoodPellet.h"
#include "MeatChunk.h"
#include "Egg.h"
#include "Genome/GeneFactory.h"
#include <Random.h>

#include <QPainter>

#include <sstream>
#include <math.h>

Swimmer::Swimmer(Energy energy, const Transform& transform, std::shared_ptr<Genome> genome)
    : Swimmer(energy, transform, genome, {})
{
}

Swimmer::Swimmer(Energy energy, const Transform& transform, std::shared_ptr<Genome> genome, std::shared_ptr<Swimmer>&& parent)
    : Swimmer(energy, transform, genome, genome->GetPhenoType(*this), std::move(parent))
{
}

Swimmer::~Swimmer()
{
    if (closestLivingAncestor_) {
        closestLivingAncestor_->DescendantDied(generation_);
    }
}

std::string_view Swimmer::GetDescription() const
{
    return "<p>A Swimmer is made up of a few important parts.</p>"
           "<ul>"
             "<li>It has senses, which are the only source of information a swimmer has about the simulation.</li>"
             "<li>It has effectors, which are the only way a swimmer can do anything within the simulation.</li>"
             "<li>It has a brain in the form of a neural network, which is responsible for converting the input from the senses into actions performed by the effectors.</li>"
             "<li>And finally it has a genome, this contains all of the information required to create the brain, the senses and the effectors.</li>"
           "</ul>";
}

std::shared_ptr<Entity> Swimmer::GiveBirth(const std::shared_ptr<Genome>& other)
{
    ++eggsLayed_;
    return std::make_shared<Egg>(shared_from_this(), TakeEnergy(100_mj), GetTransform(), genome_, other ? other : genome_, Random::Poisson(50u));
}

unsigned Swimmer::GetTotalDescendantsCount() const
{
    return std::accumulate(std::cbegin(totalDescentantCounts_), std::cend(totalDescentantCounts_), 0u, [](unsigned total, const auto& pair)
    {
        return total + pair.second;
    });
}

unsigned Swimmer::GetLivingDescendantsCount() const
{
    return std::accumulate(std::cbegin(extantDescentantCounts_), std::cend(extantDescentantCounts_), 0u, [](unsigned total, const auto& pair)
    {
        return total + pair.second;
    });
}

void Swimmer::AdjustVelocity(double adjustment)
{
    SetVelocity(GetVelocity() + adjustment);
}

void Swimmer::AdjustBearing(double adjustment)
{
    SetBearing(GetTransform().rotation + adjustment);
}

void Swimmer::TickImpl(EntityContainerInterface& container, const UniverseParameters& universeParameters)
{
    if (closestLivingAncestor_ && !closestLivingAncestor_->Exists()) {
        closestLivingAncestor_ = FindClosestLivingAncestor();
    }

    if (health_ <= 0.0) {
        // explode into some chunks of meat
        const Energy swimmerEnergy = GetEnergy();
        const double swimmerSpeed = GetVelocity();
        const Transform swimmerTransform = GetTransform();

        const int chunks = swimmerEnergy / 40_mj;
        // make sure we explode evenly outwards, and not all in one direction
        const double rotationOffset = Random::Bearing();
        const double rotationStep = Tril::Tau / chunks;
        for (int i = 0; i < chunks; i++) {
            Vec2 swimmerMovement = GetMovementVector(swimmerTransform.rotation, swimmerSpeed);
            Vec2 relativeMovement = GetMovementVector(rotationOffset + (i * rotationStep) + Random::Gaussian(0.0, Tril::Tau / 10), + Random::Gaussian(10.0, 2.5));
            Vec2 chunkMovement = { swimmerMovement.x + relativeMovement.x, swimmerMovement.y + relativeMovement.y };
            auto [ rotation, speed ] = DeconstructMovementVector(chunkMovement);{}
            Transform chunkTransform = { swimmerTransform.x + (chunkMovement.x / 10.0), swimmerTransform.y + (chunkMovement.y / 10.0), rotation };
            container.AddEntity(std::make_shared<MeatChunk>(40_mj, chunkTransform, speed));
        }
        Terminate();
    } else {
        Energy energyUsed = 0_j;
        if (brain_ && brain_->GetInputCount() > 0) {
            std::fill(std::begin(brainValues_), std::end(brainValues_), 0.0);
            for (auto& sense : senses_) {
                sense->Tick(brainValues_, container, universeParameters);
            }

            brain_->ForwardPropogate(brainValues_);

            for (auto& effector : effectors_) {
                energyUsed += effector->Tick(brainValues_, container, universeParameters);
            }
        }

        // TODO put a bunch of these parameters into genes
        if (health_ < 100.0) {
            // Heal up to 100.0 a max of 1.5 per tick
            double toHeal = std::min(0.05, 100.0 - health_);
            health_ += toHeal;
            energyUsed += toHeal * 10_uj;
        }

        UseEnergy(baseMetabolism_ + energyUsed);

        std::shared_ptr<Genome> otherGenes;
        container.ForEachCollidingWith(Circle{ GetTransform().x, GetTransform().y, GetRadius() }, [&](const std::shared_ptr<Entity>& other) -> void
        {
            if (other.get() != this) {
                if (Swimmer* s = dynamic_cast<Swimmer*>(other.get())) {
                    otherGenes = s->genome_;
                }
            }
        });

        if (GetEnergy() > 300_mj) {
            container.AddEntity(GiveBirth(otherGenes));
        } else if (GetEnergy() <= 0) {
            Terminate();
        }
    }
}

void Swimmer::DrawImpl(QPainter& paint)
{
    paint.save();
    paint.setBrush(GetColour());
    paint.drawEllipse({ GetTransform().x, GetTransform().y }, GetRadius(), GetRadius());
    if (health_ < 100.0) {
        paint.fillRect(QRectF(GetTransform().x - (26.0), GetTransform().y - GetRadius() + 13, 52.0, 7), Qt::black);
        paint.fillRect(QRectF(GetTransform().x - (25.0), GetTransform().y - GetRadius() + 14, 50.0 * health_ / 100.0, 5), Qt::red);
    }
    paint.restore();

    for (auto& sense : senses_) {
        paint.save();
        sense->Draw(paint);
        paint.restore();
    }
    for (auto& effector : effectors_) {
        paint.save();
        effector->Draw(paint);
        paint.restore();
    }
}

Swimmer::Swimmer(Energy energy, const Transform& transform, std::shared_ptr<Genome> genome, const Phenotype& phenotype, std::shared_ptr<Swimmer>&& mother)
    : Entity(transform, 6.0, phenotype.colour, energy)
    , closestLivingAncestor_(std::move(mother))
    , generation_(closestLivingAncestor_ ? closestLivingAncestor_->generation_ + 1 : 0)
    , baseMetabolism_(phenotype.baseMetabolism)
    , health_(100.0)
    , genome_(genome)
    , brain_(phenotype.brain)
    , senses_(phenotype.senses)
    , effectors_(phenotype.effectors)
    , brainValues_(brain_->GetInputCount(), 0.0)
    , eggsLayed_(0)
{
    if (closestLivingAncestor_) {
        closestLivingAncestor_->DescendantBorn(generation_);
    }
}

std::vector<Property> Swimmer::CollectProperties() const
{
    return std::vector<Property>{
        {
            "Fitness",
            [&]() -> std::string
            {
                return fmt::format("{}", this->GetTotalDescendantsCount(2));
            },
            "A good measure of fitness biologically is the number of grand-children an individual has had. Children is meaningless if you have a mutation to spawn many thousands of weak and doomed children, and great-great... grand-children e.t.c. unfairly favours older individuals.",
        },
        {
            "Generation",
            [&]() -> std::string
            {
                return fmt::format("{}", this->generation_);
            },
            "When 0, this swimmer was spawned into the simulation artificially. Generation is equal to the generation of the swimmers mother + 1.",
        },
        {
            "Metabolism",
            [&]() -> std::string
            {
                return fmt::format("{:.2f}μj", this->baseMetabolism_);
            },
            "The quantity of energy, in micro-joules, that the swimmer consumes each tick by default. A swimmer will consume more energy than this if it does any other action, e.g. moving.",
        },
        {
            "Health",
            [&]() -> std::string
            {
                return fmt::format("{:.2f}%", this->health_);
            },
            "When less than 100% a swimmer will gradually convert energy into health. If a swimmer reaches 0% health it will die, and any remaining energy it had will be converted into meat chunks.",
        },
        {
            "Genome",
            [&]() -> std::string
            {
                return fmt::format("{}", *this->genome_);
            },
            "The complete instructions used to create this swimmer.",
        },
        {
            "Brain",
            [&]() -> std::string
            {
                return fmt::format("{}", *this->brain_);
            },
            "The neural network responsible for controlling actions of the swimmer.",
        },
        {
            "Senses",
            [&]() -> std::string
            {
                return fmt::format("{}", this->senses_.size());
            },
            [&]() -> std::string
            {
                std::stringstream description;
                description << "<p>Senses are required for a swimmer to detect anything within the simulation.</p><p>The swimmer has the following senses:</p><ul>";
                for (const auto& sense : this->senses_) {
                    description << "<li>" << sense->GetName() << ": " << sense->GetDescription() << "</li>";
                }
                description << "</ul>";
                return description.str();
            }(),
        },
        {
            "Effectors",
            [&]() -> std::string
            {
                return fmt::format("{}", this->effectors_.size());
            },
            [&]() -> std::string
            {
                std::stringstream description;
                description << "<p>Effectors are required for a swimmer to do anything within the simulation.</p>"
                               "<p>The swimmer has the following effectors:</p>"
                               "<ul>";
                for (const auto& effector : this->effectors_) {
                    description << "<li>" << effector ->GetName() << ": " << effector ->GetDescription() << "</li>";
                }
                description << "</ul>";
                return description.str();
            }(),
        },
        {
            "Eggs laid",
            [&]() -> std::string
            {
                return fmt::format("{}", this->eggsLayed_);
            },
            "The number of eggs the swimmer has laid. Not all of them will hatch, depending on the genetic compatibility of the father.",
        },
        {
            "Living descendants",
            [&]() -> std::string
            {
                return fmt::format("{}", this->GetLivingDescendantsCount());
            },
            "The total number of descendants that this swimmer has had, which are currently still living.",
        },
        {
            "Total descendants",
            [&]() -> std::string
            {
                return fmt::format("{}", this->GetTotalDescendantsCount());
            },
            "The total number of descendants that this swimmer has had.",
        },
    };
}

std::shared_ptr<Swimmer> Swimmer::FindClosestLivingAncestor() const
{
    std::shared_ptr<Swimmer> ancestor = closestLivingAncestor_;
    while (ancestor && !ancestor->Exists()) {
        ancestor = ancestor->closestLivingAncestor_;
    }
    return ancestor;
}

void Swimmer::DescendantBorn(unsigned generation)
{
    // record generations of descendants relative to this's generation
    ++totalDescentantCounts_[generation - generation_];
    ++extantDescentantCounts_[generation - generation_];
    if (closestLivingAncestor_) {
        closestLivingAncestor_->DescendantBorn(generation);
    }
}

void Swimmer::DescendantDied(unsigned generation)
{
    // record generations of descendants relative to this's generation
    --extantDescentantCounts_[generation - generation_];
    if (closestLivingAncestor_) {
        closestLivingAncestor_->DescendantDied(generation);
    }
}
