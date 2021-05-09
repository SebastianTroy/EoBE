#include "QuadTree.h"

namespace Tril {

QuadTreeItem::~QuadTreeItem()
{
}

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
    AddItem(*root_, item, false);
}

void QuadTree::Clear()
{
    root_->children_ = std::nullopt;
    root_->items_.clear();
    root_->entering_.clear();
}

void QuadTree::RemoveIf(const std::function<bool (const QuadTreeItem&)> predicate)
{
    bool requiresRebalance_ = false;
    ForEachNode(*root_, [&](Quad& node)
    {
        node.items_.erase(std::remove_if(std::begin(node.items_), std::end(node.items_), [&](const auto& item) -> bool
        {
            return !predicate(*item);
        }), std::end(node.items_));

        requiresRebalance_ = requiresRebalance_ || static_cast<size_t>(std::abs(static_cast<int64_t>(itemCountTarget_)) - static_cast<int64_t>(node.items_.size())) > itemCountLeeway_;
    });

    if (requiresRebalance_) {
        Rebalance();
    }
}

void QuadTree::ForEach(const std::function<void (QuadTreeItem&)>& action, const std::function<bool (const QuadTreeItem&)>& predicate)
{
    assert(!currentlyIterating_ && "Cannot non-const iterate during another non-const iteration!");

    // Guard this block because a user may try adding items mid iteration
    currentlyIterating_ = true;

    ForEachNode(*root_, [&](const Quad& node)
    {
        for (auto& item : node.items_) {
            action(*item);
        }
    });

    currentlyIterating_ = false;

    ForEachNode(*root_, [&](Quad& node)
    {
        node.items_.erase(std::remove_if(std::begin(node.items_), std::end(node.items_), [&](const auto& item) -> bool
        {
            bool removeFromTree = !predicate(*item);
            bool removeFromNode = !Contains(node.rect_, item->GetLocation());

            if (!removeFromTree && removeFromNode) {
                AddItem(node, item, true);
            }

            return removeFromTree || removeFromNode;
        }), std::end(node.items_));

        std::move(std::begin(node.entering_), std::end(node.entering_), std::back_inserter(node.items_));
        node.entering_.clear();
    });

    Rebalance();
}

void QuadTree::ForEach(const std::function<void (const QuadTreeItem&)>& action) const
{
    ForEachNode(*root_, [&](const Quad& node)
    {
        for (const auto& item : node.items_) {
            action(*item);
        }
    });
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
    ForEachNode(*root_, [&](const Quad& node)
    {
        count += node.items_.size();
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

    ForEachNode(*root_, [&](const Quad& node) -> void
    {
        // Rect isn't too small
        double minDiameter = std::min(node.rect_.right - node.rect_.left, node.rect_.bottom - node.rect_.top);
        Require(minDiameter >= minQuadDiameter_);

        // Root node has no parent
        if (&node == root_.get()) {
            Require((node.parent_ == nullptr));
        }

        if (node.children_.has_value()) {
            // No items in node containing chldren
            Require(node.items_.empty());
            Require(node.entering_.empty());

            // Having children implies at least one item stored within
            size_t count = 0;
            ForEachNode(node, [&](const Quad& node)
            {
                count += node.items_.size();
            });
            Require(count > 0);

            for (const std::shared_ptr<Quad>& child : node.children_.value()) {
                // Child points at parent
                Require(child->parent_ == &node);
            }

            // Child rects are correct
            const Rect& parentRect = node.rect_;
            double halfWidth = (parentRect.right - parentRect.left) / 2.0;
            double midX = parentRect.left + halfWidth;
            double midY = parentRect.top + halfWidth;

            Require(node.children_.value().at(0)->rect_ == Rect{ parentRect.left, parentRect.top, midX            , midY              });
            Require(node.children_.value().at(1)->rect_ == Rect{ midX           , parentRect.top, parentRect.right, midY              });
            Require(node.children_.value().at(2)->rect_ == Rect{ parentRect.left, midY          , midX            , parentRect.bottom });
            Require(node.children_.value().at(3)->rect_ == Rect{ midX           , midY          , parentRect.right, parentRect.bottom });
        } else {
            // Leaf node should only have items_ in a const context
            Require(node.entering_.empty());

            // All items should be within the bounds of the node
            for (const auto& item : node.items_) {
                Require(Contains(node.rect_, item->GetLocation()));
            }
        }
    });

    return valid;
}

void QuadTree::ForEachNode(QuadTree::Quad& node, const std::function<void (QuadTree::Quad&)>& action)
{
    action(node);
    if (node.children_.has_value()) {
        for (auto& child : node.children_.value()) {
            ForEachNode(*child, action);
        }
    }
}

void QuadTree::ForEachNode(const QuadTree::Quad& node, const std::function<void (const QuadTree::Quad&)>& action) const
{
    action(node);
    if (node.children_.has_value()) {
        for (const auto& child : node.children_.value()) {
            ForEachNode(*child, action);
        }
    }
}

void QuadTree::AddItem(QuadTree::Quad& startOfSearch, std::shared_ptr<QuadTreeItem> item, bool preventRebalance)
{
    if (currentlyIterating_) {
        NodeAt(startOfSearch, item->GetLocation()).entering_.push_back(item);
    } else {
        Quad& targetNode = NodeAt(startOfSearch, item->GetLocation());
        targetNode.items_.push_back(item);

        if (!preventRebalance) {
            double minDiameter = std::min(targetNode.rect_.right - targetNode.rect_.left, targetNode.rect_.bottom - targetNode.rect_.top);
            if (minDiameter > (2 * minQuadDiameter_) && targetNode.items_.size() > itemCountTarget_ + itemCountLeeway_) {
                Rebalance();
            }
        }
    }
}

QuadTree::Quad& QuadTree::NodeAt(QuadTree::Quad& startOfSearch, const Point& location)
{
    if (!Contains(startOfSearch.rect_, location)) {
        if (!startOfSearch.parent_) {
            ExpandRoot();
        }
        return NodeAt(*root_, location);
    } else if (startOfSearch.children_.has_value()) {
        size_t index = SubQuadIndex(startOfSearch.rect_, location);
        return NodeAt(*startOfSearch.children_.value().at(index), location);
    } else {
        return startOfSearch;
    }
}

void QuadTree::Rebalance()
{
    ForEachNode(*root_, [&](Quad& node)
    {
        size_t itemCount = RecursiveItemCount(node);
        if (node.children_.has_value() && (itemCount == 0 || itemCount < itemCountTarget_ - itemCountLeeway_)) {
            // Become a leaf node if children contain too few entities
            node.items_ = RecursiveCollectItems(node);
            node.children_ = std::nullopt;
        } else if (!node.children_.has_value() && (node.rect_.right - node.rect_.left >= minQuadDiameter_ * 2.0) && node.items_.size() > itemCountTarget_ + itemCountLeeway_) {
            // Lose leaf node status if contains too many children UNLESS the new quads would be below the minimum size!
            node.children_ = CreateChildren(node);
            std::vector<std::shared_ptr<QuadTreeItem>> itemsToRehome;
            itemsToRehome.swap(node.items_);
            for (auto& item : itemsToRehome) {
                NodeAt(node, item->GetLocation()).items_.push_back(item);
            }
        }
    });

    ContractRoot();
}

size_t QuadTree::RecursiveItemCount(const QuadTree::Quad& node)
{
    size_t count = 0;
    ForEachNode(node, [&](const Quad& node)
    {
        count += node.items_.size();
    });
    return count;
}

std::vector<std::shared_ptr<QuadTreeItem> > QuadTree::RecursiveCollectItems(QuadTree::Quad& node)
{
    std::vector<std::shared_ptr<QuadTreeItem>> collectedItems;
    ForEachNode(node, [&](Quad& node)
    {
        std::move(std::begin(node.items_), std::end(node.items_), std::back_inserter(collectedItems));
    });
    return collectedItems;
}

std::array<std::shared_ptr<QuadTree::Quad>, 4> QuadTree::CreateChildren(QuadTree::Quad& node)
{
    const Rect& parentRect = node.rect_;

    double halfWidth = (parentRect.right - parentRect.left) / 2.0;
    double midX = parentRect.left + halfWidth;
    double midY = parentRect.top + halfWidth;

    return {
        std::make_shared<Quad>(&node, Rect{ parentRect.left, parentRect.top, midX            , midY              }),
        std::make_shared<Quad>(&node, Rect{ midX           , parentRect.top, parentRect.right, midY              }),
        std::make_shared<Quad>(&node, Rect{ parentRect.left, midY          , midX            , parentRect.bottom }),
        std::make_shared<Quad>(&node, Rect{ midX           , midY          , parentRect.right, parentRect.bottom }),
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
