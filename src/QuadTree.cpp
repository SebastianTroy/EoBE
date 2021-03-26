#include "QuadTree.h"

#include "Entity.h"

#include <QPainter>

#include <algorithm>
#include <assert.h>

QuadTree::QuadTree(const Rect& startingArea)
    : targetCount_(1)
    , leewayCount_(0)
    , requiresRebalance_(true)
    , rootExpandedCount_(0)
{
    Reset(startingArea);
}

void QuadTree::Tick(const UniverseParameters& universeParameters)
{
    if (requiresRebalance_) {
        root_->Rebalance(targetCount_, leewayCount_);
        requiresRebalance_ = false;
    }
    root_->TickRecursive(universeParameters);
    root_->ResolveRecursive();
}

void QuadTree::Draw(QPainter& paint, const Rect& renderArea) const
{
    double minX = renderArea.left - Entity::MAX_RADIUS;
    double maxX = renderArea.right + Entity::MAX_RADIUS;
    double minY = renderArea.top - Entity::MAX_RADIUS;
    double maxY = renderArea.bottom + Entity::MAX_RADIUS;
    /*
     * Expand the area slightly to catch edge cases where entites overlap into
     * the visible area from quads that do not intersect the visible area.
     */
    Rect expandedArea { minX, minY, maxX, maxY };
    root_->DrawRecursive(paint, expandedArea);
}

void QuadTree::SetEntityTargetPerQuad(uint64_t target, uint64_t leeway)
{
    // target cannot be 0 (requires infinite quads)
    targetCount_ = std::max(target, uint64_t{1});
    // FIXME BUG leeway cannot be greater than target, causes repeat recalculation (probably uint underflow somewhere)
    leewayCount_ = std::min(leeway, target);
    root_->Rebalance(targetCount_, leewayCount_);
    requiresRebalance_ = false;
}

std::shared_ptr<Entity> QuadTree::PickEntity(const Point& location, bool remove)
{
    double minX = location.x - Entity::MAX_RADIUS;
    double maxX = location.x + Entity::MAX_RADIUS;
    double minY = location.y - Entity::MAX_RADIUS;
    double maxY = location.y + Entity::MAX_RADIUS;
    Rect searchArea { minX, minY, maxX, maxY };
    return root_->PickRecursive(searchArea, location, remove);
}

void QuadTree::ForEach(const std::function<void (const std::shared_ptr<Entity>&)>& action) const
{
    if (root_) {
        root_->ForEachRecursive(action);
    }
}

void QuadTree::Reset(const Rect& area)
{
    root_ = std::make_shared<Quad>(area, *this, nullptr);
}

void QuadTree::Clear()
{
    Reset(root_->GetRect());
}

void QuadTree::ExpandRoot()
{
    bool expandOutwards = rootExpandedCount_++ % 2 == 0;
    const Rect& oldRootRect = root_->GetRect();
    double width = oldRootRect.right - oldRootRect.left;
    double height = oldRootRect.bottom - oldRootRect.top;

    Rect newRootRect = {
        oldRootRect.left - (expandOutwards ? 0.0 : width),
        oldRootRect.top - (expandOutwards ? 0.0 : height),
        oldRootRect.right + (expandOutwards ? width : 0.0),
        oldRootRect.bottom + (expandOutwards ? height : 0.0)
    };
    std::vector<std::shared_ptr<Quad>> children = Quad::CreateChildren(newRootRect, *this, nullptr);
    for (auto& child : children) {
        if (root_->GetRect() == child->GetRect()) {
            child.swap(root_);
            break;
        }
    }
    root_ = std::make_shared<Quad>(newRootRect, *this, std::move(children));
}

void QuadTree::ContractRoot()
{
    if (auto newRoot = root_->GetNewRoot()) {
        root_ = newRoot;
        --rootExpandedCount_;
    }
}

QuadTree::Quad::Quad(const Rect& area, QuadTree& owner, QuadTree::Quad* parent)
    : baseTree_(owner)
    , parent_(parent)
    , rect_(area)
{
}

QuadTree::Quad::Quad(const Rect& area, QuadTree& owner, std::vector<std::shared_ptr<QuadTree::Quad> >&& children)
    : Quad(area, owner, nullptr)
{
    children_.swap(children);
    for (auto& child : children_) {
        child->parent_ = this;
    }
}

QuadTree::Quad::~Quad()
{
}

std::vector<std::shared_ptr<QuadTree::Quad> > QuadTree::Quad::CreateChildren(Rect parentRect, QuadTree& baseTree, QuadTree::Quad* parent)
{
    double halfWidth = (parentRect.right - parentRect.left) / 2.0;
    double midX = parentRect.left + halfWidth;
    double midY = parentRect.top + halfWidth;

    return {
        std::make_shared<Quad>(Rect{ parentRect.left, parentRect.top, midX            , midY }             , baseTree, parent),
        std::make_shared<Quad>(Rect{ midX           , parentRect.top, parentRect.right, midY }             , baseTree, parent),
        std::make_shared<Quad>(Rect{ parentRect.left, midY          , midX            , parentRect.bottom }, baseTree, parent),
        std::make_shared<Quad>(Rect{ midX           , midY          , parentRect.right, parentRect.bottom }, baseTree, parent),
    };
}

void QuadTree::Quad::AddEntity(const std::shared_ptr<Entity>& entity)
{
    RehomeRecursive(entity, true);
}

void QuadTree::Quad::TickRecursive(const UniverseParameters& universeParameters)
{
    if (!children_.empty()) {
        assert(entities_.empty());
        for (auto& child : children_) {
            child->TickRecursive(universeParameters);
        }
    } else {
        // Tick entities and register moved entities with their new home
        for (auto e : entities_) {
            if (e->Tick(*this, universeParameters)) {
                if (!Contains(rect_, e->GetLocation())) {
                    exitingEntities_.push_back(e);
                    RehomeRecursive(e, true);
                }
            }
        }
    }
}

void QuadTree::Quad::ResolveRecursive()
{
    if (!children_.empty()) {
        // Only leaf nodes contain child entities so we won't have anything to deal with here
        assert(enteringEntities_.empty());
        assert(exitingEntities_.empty());
        assert(entities_.empty());
        for (auto& child : children_) {
            child->ResolveRecursive();
        }

        if (AreAllChildrenLeafQuads() && RecursiveEntityCount() <= baseTree_.targetCount_ - baseTree_.leewayCount_) {
            baseTree_.requiresRebalance_ = true;
        }
    } else {
        entities_.erase(std::remove_if(std::begin(entities_), std::end(entities_), [&](auto& e)
        {
            // remove if has exitied or died
            bool exited = std::find(std::begin(exitingEntities_), std::end(exitingEntities_), e) != std::end(exitingEntities_);
            bool died = !e->Exists();
            return exited || died;
        }), std::end(entities_));
        exitingEntities_.clear();

        // insert if exists
        std::copy_if(std::begin(enteringEntities_), std::end(enteringEntities_), std::back_inserter(entities_), [](auto& e)
        {
            return e->Exists();
        });
        enteringEntities_.clear();

        if (entities_.size() >= baseTree_.targetCount_ + baseTree_.leewayCount_) {
            baseTree_.requiresRebalance_ = true;
        }
    }
}

void QuadTree::Quad::DrawRecursive(QPainter& paint, const Rect& renderArea) const
{
    if (Collides(rect_, renderArea)) {
        paint.save();
        QPen quadPen(Qt::black);
        quadPen.setCosmetic(true);
        paint.setPen(quadPen);
        paint.drawRect(QRectF(QPointF(rect_.left, rect_.top), QPointF(rect_.right, rect_.bottom)));
        paint.restore();
        if (!children_.empty()) {
            for (auto& child : children_) {
                child->DrawRecursive(paint, renderArea);
            }
        } else {
            for (auto e : entities_) {
                e->Draw(paint);
            }
        }
    }
}

void QuadTree::Quad::RehomeRecursive(const std::shared_ptr<Entity>& entity, bool delayed)
{
    if (Contains(rect_, entity->GetLocation())) {
        if (children_.empty()) {
            if (delayed) {
                enteringEntities_.push_back(entity);
            } else {
                entities_.push_back(entity);
                if (entities_.size() >= baseTree_.targetCount_ + baseTree_.leewayCount_) {
                    baseTree_.requiresRebalance_ = true;
                }
            }
        } else {
            for (auto& child : children_) {
                if (Contains(child->rect_, entity->GetLocation())) {
                    child->RehomeRecursive(entity, delayed);
                }
            }
        }
    } else if (parent_) {
        parent_->RehomeRecursive(entity, delayed);
    } else {
        baseTree_.ExpandRoot();
        RehomeRecursive(entity, delayed);
    }
}

std::shared_ptr<Entity> QuadTree::Quad::PickRecursive(const Rect& searchArea, const Point& location, bool remove)
{
    if (Collides(rect_, searchArea)) {
        if (children_.empty()) {
            std::shared_ptr<Entity> picked;
            auto iter = std::find_if(std::begin(entities_), std::end(entities_), [&](const std::shared_ptr<Entity>& entity)
            {
                return Contains(entity->GetCollide(), location);
            });

            if (iter != std::end(entities_)) {
                picked = *iter;
                if (remove) {
                    // TODO ensure this doesn't occur mid tick, risking the entity remaining in an "entering" queue elsewhere
                    entities_.erase(iter);
                    if (entities_.size() <= baseTree_.targetCount_ + baseTree_.leewayCount_) {
                        baseTree_.requiresRebalance_ = true;
                    }
                }
            }
            return picked;
        } else {
            std::shared_ptr<Entity> picked;
            for (auto& child : children_) {
                if (!picked && Collides(child->rect_, searchArea)) {
                    picked = child->PickRecursive(searchArea, location, remove);
                }
            }
            return picked;
        }
    } else if (parent_) {
        return parent_->PickRecursive(searchArea, location, remove);
    }
    return nullptr;
}

void QuadTree::Quad::ForEachRecursive(const std::function<void(const std::shared_ptr<Entity>&)>& action) const
{
    for (const auto& child : children_) {
        child->ForEachRecursive(action);
    }
    for (const auto& entity : entities_) {
        if (entity->Exists()) {
            action(entity);
        }
    }
}

uint64_t QuadTree::Quad::RecursiveEntityCount() const
{
    uint64_t count = 0;
    if (children_.empty()) {
        count = entities_.size();
    } else {
        for (auto& child : children_) {
            count += child->RecursiveEntityCount();
        }
    }
    return count;
}

const Rect& QuadTree::Quad::GetRect()
{
    return rect_;
}

void QuadTree::Quad::ForEachCollidingWith(const Point& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action) const
{
    ForEachCollidingWith<Point>(BoundingRect(collide, Entity::MAX_RADIUS), collide, action);
}

void QuadTree::Quad::ForEachCollidingWith(const Line& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action) const
{
    ForEachCollidingWith<Line>(BoundingRect(collide, Entity::MAX_RADIUS), collide, action);
}

void QuadTree::Quad::ForEachCollidingWith(const Rect& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action) const
{
    ForEachCollidingWith<Rect>(BoundingRect(collide, Entity::MAX_RADIUS), collide, action);
}

void QuadTree::Quad::ForEachCollidingWith(const Circle& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action) const
{
    ForEachCollidingWith<Circle>(BoundingRect(collide, Entity::MAX_RADIUS), collide, action);
}

void QuadTree::Quad::Rebalance(const uint64_t targetCount, uint64_t historesis)
{
    assert(enteringEntities_.empty());
    assert(exitingEntities_.empty());

    // Recurse from leaf to root
    for (auto& child : children_) {
        child->Rebalance(targetCount, historesis);
    }

    if (!children_.empty() && RecursiveEntityCount() < targetCount - historesis) {
        // Become a leaf node if children contain too few entities
        entities_ = RecursivelyCollectEntities();
        children_.clear();
    } else if (children_.empty() && RecursiveEntityCount() > targetCount + historesis) {
        // Lose leaf node status if contains too many children
        children_ = CreateChildren(rect_, baseTree_, this);
        for (auto& entity : entities_) {
            for (auto& child : children_) {
                if (Contains(child->rect_, Point{ entity->GetTransform().x, entity->GetTransform().y })) {
                    child->entities_.push_back(entity);
                    continue;
                }
            }
        }
        entities_.clear();
    }

    // Final act of rebalancing
    if (this == baseTree_.root_.get()) {
        baseTree_.ContractRoot();
    }
}

std::shared_ptr<QuadTree::Quad> QuadTree::Quad::GetNewRoot()
{
    assert(enteringEntities_.empty());
    assert(exitingEntities_.empty());

    std::shared_ptr<QuadTree::Quad> newRoot;
    unsigned emptyChildren = std::count_if(std::cbegin(children_), std::cend(children_), [](auto& child)
    {
        assert(child->enteringEntities_.empty());
        assert(child->exitingEntities_.empty());
        return child->entities_.empty() && child->children_.empty();
    });

    // if exactly one child has entities within it, make it the new root
    if (emptyChildren == 3) {
        // Find the empty child and see if the new root can go one deeper
        for (const auto& child : children_) {
            if (!child->entities_.empty() || !child->children_.empty()) {
                // If this child is not the new root, it will be deconstructed
                // But if it is the new root, its parent must be a nullptr!
                child->parent_ = nullptr;
                newRoot = child->GetNewRoot();
                if (newRoot == nullptr) {
                    // Child does not contain the new root, therefore it is the new root
                    newRoot = child;
                }
                break;
            }
        }
    }

    return newRoot;
}

bool QuadTree::Quad::AreAllChildrenLeafQuads()
{
    for (auto& child : children_) {
        if (!child->children_.empty()) {
            return false;
        }
    }
    return true;
}

std::vector<std::shared_ptr<Entity> > QuadTree::Quad::RecursivelyCollectEntities()
{
    std::vector<std::shared_ptr<Entity>> allEntities;
    if (children_.empty()) {
        allEntities.swap(entities_);
    } else {
        for (auto& child : children_) {
            std::vector<std::shared_ptr<Entity>> entities = child->RecursivelyCollectEntities();
            std::move(std::begin(entities), std::end(entities), std::back_inserter(allEntities));
        }
    }
    return allEntities;
}

