#include "QuadTree.h"

namespace Tril {

///
/// QuadTreeItem
///

QuadTreeItem::~QuadTreeItem()
{
}

///
/// QuadTreeIterator
///

QuadTreeIterator::QuadTreeIterator(std::function<void (std::shared_ptr<QuadTreeItem>)>&& action)
    : itemAction_(std::move(action))
    , quadFilter_([](const Rect&){ return true; })
    , itemFilter_([](const QuadTreeItem&){ return true; })
    , removeItemPredicate_([](const QuadTreeItem&){ return false; })
{
}

QuadTreeIterator& QuadTreeIterator::SetQuadFilter(std::function<bool (const Rect&)>&& filter)
{
    quadFilter_ = std::move(filter);
    return *this;
}

QuadTreeIterator& QuadTreeIterator::SetQuadFilter(const Rect& r)
{
    SetQuadFilter([=](const Rect& quadArea)
    {
        return Collides(r, quadArea);
    });
    return *this;
}

QuadTreeIterator& QuadTreeIterator::SetItemFilter(std::function<bool (const QuadTreeItem&)>&& filter)
{
    itemFilter_ = std::move(filter);
    return *this;
}

QuadTreeIterator& QuadTreeIterator::SetItemFilter(const Point& p)
{
    SetItemFilter([=](const QuadTreeItem& item)
    {
        return Collides(p, item.GetCollide());
    });
    return *this;
}

QuadTreeIterator& QuadTreeIterator::SetItemFilter(const Line& l)
{
    SetItemFilter([=](const QuadTreeItem& item)
    {
        return Collides(l, item.GetCollide());
    });
    return *this;
}

QuadTreeIterator& QuadTreeIterator::SetItemFilter(const Circle& c)
{
    SetItemFilter([=](const QuadTreeItem& item)
    {
        return Collides(c, item.GetCollide());
    });
    return *this;
}

QuadTreeIterator& QuadTreeIterator::SetItemFilter(const Rect& r)
{
    SetItemFilter([=](const QuadTreeItem& item)
    {
        return Collides(r, item.GetCollide());
    });
    return *this;
}

QuadTreeIterator& QuadTreeIterator::SetRemoveItemPredicate(std::function<bool (const QuadTreeItem&)>&& removeItemPredicate)
{
    removeItemPredicate_ = std::move(removeItemPredicate);
    return *this;
}

///
/// ConstQuadTreeIterator
///

ConstQuadTreeIterator::ConstQuadTreeIterator(std::function<void (const QuadTreeItem&)>&& action)
    : itemAction_(std::move(action))
    , quadFilter_([](const Rect&){ return true; })
    , itemFilter_([](const QuadTreeItem&){ return true; })
{
}

ConstQuadTreeIterator& ConstQuadTreeIterator::SetQuadFilter(std::function<bool (const Rect&)>&& filter)
{
    quadFilter_ = std::move(filter);
    return *this;
}

ConstQuadTreeIterator& ConstQuadTreeIterator::SetQuadFilter(const Rect& r)
{
    SetQuadFilter([=](const Rect& quadArea)
    {
        return Collides(r, quadArea);
    });
    return *this;
}

ConstQuadTreeIterator& ConstQuadTreeIterator::SetItemFilter(std::function<bool (const QuadTreeItem&)>&& filter)
{
    itemFilter_ = std::move(filter);
    return *this;
}

ConstQuadTreeIterator& ConstQuadTreeIterator::SetItemFilter(const Point& p)
{
    SetItemFilter([=](const QuadTreeItem& item)
    {
        return Collides(p, item.GetCollide());
    });
    return *this;
}

ConstQuadTreeIterator& ConstQuadTreeIterator::SetItemFilter(const Line& l)
{
    SetItemFilter([=](const QuadTreeItem& item)
    {
        return Collides(l, item.GetCollide());
    });
    return *this;
}

ConstQuadTreeIterator& ConstQuadTreeIterator::SetItemFilter(const Circle& c)
{
    SetItemFilter([=](const QuadTreeItem& item)
    {
        return Collides(c, item.GetCollide());
    });
    return *this;
}

ConstQuadTreeIterator& ConstQuadTreeIterator::SetItemFilter(const Rect& r)
{
    SetItemFilter([=](const QuadTreeItem& item)
    {
        return Collides(r, item.GetCollide());
    });
    return *this;
}

///
/// QuadTree
///

QuadTree::QuadTree(const Rect& startArea, size_t itemCountTarget, size_t itemCountLeeway, double minQuadDiameter)
    : root_(std::make_shared<Quad>(nullptr, startArea))
    , rootExpandedCount_(0)
    , itemCountTarget_(std::max(itemCountTarget, size_t{1}))
    , itemCountLeeway_(std::min(itemCountTarget, itemCountLeeway))
    , minQuadDiameter_(minQuadDiameter)
    , currentlyIterating_(false)
{
}

void QuadTree::Insert(std::shared_ptr<QuadTreeItem> item)
{
    assert(!currentlyIterating_);
    AddItem(*root_, item, false);
}

void QuadTree::Clear()
{
    assert(!currentlyIterating_);
    root_->children_ = std::nullopt;
    root_->items_.clear();
    root_->entering_.clear();
}

void QuadTree::RemoveIf(const std::function<bool(const QuadTreeItem&)>& predicate)
{
    assert(!currentlyIterating_);
    bool requiresRebalance_ = false;
    ForEachQuad(*root_, [&](Quad& quad)
    {
        quad.items_.erase(std::remove_if(std::begin(quad.items_), std::end(quad.items_), [&](const auto& item) -> bool
        {
            return predicate(*item);
        }), std::end(quad.items_));

        requiresRebalance_ = requiresRebalance_ || static_cast<size_t>(std::abs(static_cast<int64_t>(itemCountTarget_)) - static_cast<int64_t>(quad.items_.size())) > itemCountLeeway_;
    });

    if (requiresRebalance_) {
        Rebalance();
    }
}

void QuadTree::ForEachQuad(const std::function<void (const Rect&)>& action) const
{
    ForEachQuad(*root_, [&](const Quad& quad)
    {
        action(quad.rect_);
    });
}

void QuadTree::ForEachItem(const ConstQuadTreeIterator& iter) const
{
    ForEachQuad(*root_, [&](const Quad& quad)
    {
        for (const auto& item : quad.items_) {
            if (iter.itemFilter_(*item)) {
                iter.itemAction_(*item);
            }
        }
    }, iter.quadFilter_);
}

void QuadTree::ForEachItem(const QuadTreeIterator& iter)
{
    bool wasIteratingAlready = currentlyIterating_;
    currentlyIterating_ = true;

        ForEachQuad(*root_, [&](const Quad& quad)
        {
            for (auto& item : quad.items_) {
                if (iter.itemFilter_(*item)) {
                    iter.itemAction_(item);
                }
            }
        }, iter.quadFilter_);


    // Let the very first non-const iteration deal with all of the re-balancing
    if (!wasIteratingAlready) {
        currentlyIterating_ = false;

        ForEachQuad(*root_, [&](Quad& quad)
        {
            quad.items_.erase(std::remove_if(std::begin(quad.items_), std::end(quad.items_), [&](const auto& item) -> bool
            {
                bool removeFromTree = iter.removeItemPredicate_(*item);
                bool removeFromQuad = !Contains(quad.rect_, item->GetLocation());

                if (!removeFromTree && removeFromQuad) {
                    AddItem(quad, item, true);
                }

                return removeFromTree || removeFromQuad;
            }), std::end(quad.items_));

            std::move(std::begin(quad.entering_), std::end(quad.entering_), std::back_inserter(quad.items_));
            quad.entering_.clear();
        });

        Rebalance();
    }
}

void QuadTree::SetItemCountTaregt(unsigned target)
{
    itemCountTarget_ = target;
}

void QuadTree::SetItemCountLeeway(unsigned leeway)
{
    itemCountLeeway_ = leeway;
}

unsigned QuadTree::GetItemCountTaregt() const
{
    return itemCountTarget_;
}

unsigned QuadTree::GetItemCountLeeway() const
{
    return itemCountLeeway_;
}

size_t QuadTree::Size() const
{
    size_t count = 0;
    ForEachQuad(*root_, [&](const Quad& quad)
    {
        count += quad.items_.size();
    });
    return count;
}

bool QuadTree::Validate() const
{
    bool valid = true;

    // For easy breakpoint setting for debugging!
    auto Require = [&](bool val)
    {
        if (!val) {
            valid = false;
        }
    };

    // The following will not work if we are non-const looping, but we may want
    // to validate mid const loop, or make sure the const version is being called
    Require(!currentlyIterating_);

    ForEachQuad(*root_, [&](const Quad& quad) -> void
    {
        // Rect isn't too small
        double minDiameter = std::min(quad.rect_.right - quad.rect_.left, quad.rect_.bottom - quad.rect_.top);
        Require(minDiameter >= minQuadDiameter_);

        // Root quad has no parent
        if (&quad == root_.get()) {
            Require((quad.parent_ == nullptr));
        }

        if (quad.children_.has_value()) {
            // No items in quad containing chldren
            Require(quad.items_.empty());
            Require(quad.entering_.empty());

            // Having children implies at least one item stored within
            size_t count = 0;
            ForEachQuad(quad, [&](const Quad& quad)
            {
                count += quad.items_.size();
            });
            Require(count > 0);

            for (const std::shared_ptr<Quad>& child : quad.children_.value()) {
                // Child points at parent
                Require(child->parent_ == &quad);
            }

            // Child rects are correct
            const Rect& parentRect = quad.rect_;
            double halfWidth = (parentRect.right - parentRect.left) / 2.0;
            double midX = parentRect.left + halfWidth;
            double midY = parentRect.top + halfWidth;

            Require(quad.children_.value().at(0)->rect_ == Rect{ parentRect.left, parentRect.top, midX            , midY              });
            Require(quad.children_.value().at(1)->rect_ == Rect{ midX           , parentRect.top, parentRect.right, midY              });
            Require(quad.children_.value().at(2)->rect_ == Rect{ parentRect.left, midY          , midX            , parentRect.bottom });
            Require(quad.children_.value().at(3)->rect_ == Rect{ midX           , midY          , parentRect.right, parentRect.bottom });
        } else {
            // Leaf quad should only have items_ in a const context
            Require(quad.entering_.empty());

            // All items should be within the bounds of the quad
            for (const auto& item : quad.items_) {
                Require(Contains(quad.rect_, item->GetLocation()));
            }
        }
    });

    return valid;
}

void QuadTree::ForEachQuad(QuadTree::Quad& quad, const std::function<void (QuadTree::Quad&)>& action)
{
    ForEachQuad(quad, action, [](auto){ return true; });
}

void QuadTree::ForEachQuad(QuadTree::Quad& quad, const std::function<void (QuadTree::Quad&)>& action, const std::function<bool(const Rect& area)>& filter)
{
    action(quad);
    if (quad.children_.has_value()) {
        for (auto& child : quad.children_.value()) {
            if (filter(child->rect_)) {
                ForEachQuad(*child, action, filter);
            }
        }
    }
}

void QuadTree::ForEachQuad(const QuadTree::Quad& quad, const std::function<void (const QuadTree::Quad&)>& action) const
{
    ForEachQuad(quad, action, [](auto){ return true; });
}

void QuadTree::ForEachQuad(const QuadTree::Quad& quad, const std::function<void (const QuadTree::Quad&)>& action, const std::function<bool(const Rect&)>& filter) const
{
    action(quad);
    if (quad.children_.has_value()) {
        for (const auto& child : quad.children_.value()) {
            if (filter(child->rect_)) {
                ForEachQuad(*child, action, filter);
            }
        }
    }
}

void QuadTree::AddItem(QuadTree::Quad& startOfSearch, std::shared_ptr<QuadTreeItem> item, bool preventRebalance)
{
    if (currentlyIterating_) {
        QuadAt(startOfSearch, item->GetLocation()).entering_.push_back(item);
    } else {
        Quad& targetQuad = QuadAt(startOfSearch, item->GetLocation());
        targetQuad.items_.push_back(item);

        if (!preventRebalance) {
            double minDiameter = std::min(targetQuad.rect_.right - targetQuad.rect_.left, targetQuad.rect_.bottom - targetQuad.rect_.top);
            if (minDiameter > (2 * minQuadDiameter_) && targetQuad.items_.size() > itemCountTarget_ + itemCountLeeway_) {
                Rebalance();
            }
        }
    }
}

QuadTree::Quad& QuadTree::QuadAt(QuadTree::Quad& startOfSearch, const Point& location)
{
    if (!Contains(startOfSearch.rect_, location)) {
        if (!startOfSearch.parent_) {
            ExpandRoot();
        }
        return QuadAt(*root_, location);
    } else if (startOfSearch.children_.has_value()) {
        size_t index = SubQuadIndex(startOfSearch.rect_, location);
        return QuadAt(*startOfSearch.children_.value().at(index), location);
    } else {
        return startOfSearch;
    }
}

void QuadTree::Rebalance()
{
    ForEachQuad(*root_, [&](Quad& quad)
    {
        size_t itemCount = RecursiveItemCount(quad);
        if (quad.children_.has_value() && (itemCount == 0 || itemCount < itemCountTarget_ - itemCountLeeway_)) {
            // Become a leaf quad if children contain too few entities
            quad.items_ = RecursiveCollectItems(quad);
            quad.children_ = std::nullopt;
        } else if (!quad.children_.has_value() && (quad.rect_.right - quad.rect_.left >= minQuadDiameter_ * 2.0) && quad.items_.size() > itemCountTarget_ + itemCountLeeway_) {
            // Lose leaf quad status if contains too many children UNLESS the new quads would be below the minimum size!
            quad.children_ = CreateChildren(quad);
            std::vector<std::shared_ptr<QuadTreeItem>> itemsToRehome;
            itemsToRehome.swap(quad.items_);
            for (auto& item : itemsToRehome) {
                QuadAt(quad, item->GetLocation()).items_.push_back(item);
            }
        }
    });

    ContractRoot();
}

size_t QuadTree::RecursiveItemCount(const QuadTree::Quad& quad)
{
    size_t count = 0;
    ForEachQuad(quad, [&](const Quad& quad)
    {
        count += quad.items_.size();
    });
    return count;
}

std::vector<std::shared_ptr<QuadTreeItem> > QuadTree::RecursiveCollectItems(QuadTree::Quad& quad)
{
    std::vector<std::shared_ptr<QuadTreeItem>> collectedItems;
    ForEachQuad(quad, [&](Quad& quad)
    {
        std::move(std::begin(quad.items_), std::end(quad.items_), std::back_inserter(collectedItems));
    });
    return collectedItems;
}

std::array<std::shared_ptr<QuadTree::Quad>, 4> QuadTree::CreateChildren(QuadTree::Quad& quad)
{
    const Rect& parentRect = quad.rect_;

    double halfWidth = (parentRect.right - parentRect.left) / 2.0;
    double midX = parentRect.left + halfWidth;
    double midY = parentRect.top + halfWidth;

    return {
        std::make_shared<Quad>(&quad, Rect{ parentRect.left, parentRect.top, midX            , midY              }),
        std::make_shared<Quad>(&quad, Rect{ midX           , parentRect.top, parentRect.right, midY              }),
        std::make_shared<Quad>(&quad, Rect{ parentRect.left, midY          , midX            , parentRect.bottom }),
        std::make_shared<Quad>(&quad, Rect{ midX           , midY          , parentRect.right, parentRect.bottom }),
    };
}

void QuadTree::ExpandRoot()
{
    bool expandOutwards = rootExpandedCount_++ % 2 == 0;
    const Rect& oldRootRect = root_->rect_;
    double width = oldRootRect.right - oldRootRect.left;
    double height = oldRootRect.bottom - oldRootRect.top;
    Rect newRootRect{
        oldRootRect.left - (expandOutwards ? 0.0 : width),
        oldRootRect.top - (expandOutwards ? 0.0 : height),
        oldRootRect.right + (expandOutwards ? width : 0.0),
        oldRootRect.bottom + (expandOutwards ? height : 0.0)
    };

    std::shared_ptr<Quad> newRoot = std::make_shared<Quad>(nullptr, newRootRect);

    if (RecursiveItemCount(*root_) >= itemCountTarget_ - itemCountLeeway_) {
        // Keep children
        const size_t index = expandOutwards ? 0 : 3;
        newRoot->children_ = CreateChildren(*newRoot);
        newRoot->children_->at(index).swap(root_);
        newRoot->children_->at(index)->parent_ = newRoot.get();
    } else {
        // Discard children
        auto newRoot = std::make_shared<Quad>(nullptr, newRootRect);
        newRoot->items_ = RecursiveCollectItems(*root_);
    }

    root_.swap(newRoot);
}

void QuadTree::ContractRoot()
{
    if (root_->children_.has_value()) {
        unsigned count = 0;
        std::shared_ptr<Quad> quadWithItems;
        for (auto& child : root_->children_.value()) {
            if (child->items_.size() > 0 || child->children_.has_value()) {
                ++count;
                quadWithItems = child;
            }
        }
        if (count == 1) {
            root_ = quadWithItems;
            root_->parent_ = nullptr;
            --rootExpandedCount_;
        }
    }
}

size_t QuadTree::SubQuadIndex(const Rect& rect, const Point& p) const
{
    //  ___
    // |0|1| Sub-Quad indices
    // |2|3|
    //  ---
    // Add one if in the right half (avoids branching)
    size_t lr = ((p.x - rect.left) / (rect.right - rect.left)) + 0.5;
    // Add two if in the bottom half (avoids branching)
    size_t tb = static_cast<size_t>(((p.y - rect.top) / (rect.bottom - rect.top)) + 0.5) * 2;
    return lr + tb;
}

QuadTree::Quad::Quad(QuadTree::Quad* parent, Rect rect)
    : parent_(parent)
    , children_(std::nullopt)
    , rect_(rect)
    , items_{}
    , entering_{}
{
}

} // namespace Tril
