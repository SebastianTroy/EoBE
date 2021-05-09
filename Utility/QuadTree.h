#ifndef QUADTREE_H
#define QUADTREE_H

#include "Shape.h"

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <cmath>

namespace Tril {

class QuadTreeItem {
public:
    virtual ~QuadTreeItem();
    virtual const Point& GetLocation() const = 0;
};

class QuadTree {
public:
    QuadTree(const Rect& startArea, size_t itemCountTarget, size_t itemCountLeeway, double minQuadDiameter);

    void Insert(std::shared_ptr<QuadTreeItem> item);

    void Clear();
    void RemoveIf(const std::function<bool(const QuadTreeItem& item)> predicate);

    /**
     * Performs an action for each item, as this is not const it assumes any
     * item may have moved, so it will check each item location and move and
     * rebalance the tree accordingly. It is also possible to supply a check to
     * determine whether any given item should be removed from the container.
     * This is supplied so that the cntainer can be iterated and modified in one
     * loop, rather than requiring an additional call to RemofeIf(predicate)
     * immediately after.
     *
     * @param action The action will be called for each item contained within
     * the quad tree in an indeterminate order.
     *
     * @param predicate If this returns false for an item, it will be removed
     * from the quad tree, by default will return true for all items.
     */
    void ForEach(const std::function<void(QuadTreeItem& item)>& action, const std::function<bool(const QuadTreeItem&)>& predicate = [](const QuadTreeItem&){ return true; });
    void ForEach(const std::function<void(const QuadTreeItem& item)>& action) const;

    template <typename T>
    void ForEach(const std::function<void(T& item)>& action, const std::function<bool(const T&)>& predicate = [](const T&){ return true; })
    {
        ForEach([&](QuadTreeItem& item)
        {
            if (T* castItem = dynamic_cast<T*>(&item)) {
                action(*castItem);
            }
        },
        [&](const QuadTreeItem& item) -> bool
        {
            const T* castItem = dynamic_cast<const T*>(&item);
            return castItem && predicate(*castItem);
        });
    }
    template <typename T>
    void ForEach(const std::function<void(const QuadTreeItem& item)>& action) const
    {
        ForEach([&](QuadTreeItem& item)
        {
            if (const T* castItem = dynamic_cast<const T*>(&item)) {
                action(*castItem);
            }
        });
    }

    void SetItemCountTaregt(unsigned target);
    void SetItemCountLeeway(unsigned leeway);

    unsigned GetItemCountTaregt() const;
    unsigned GetItemCountLeeway() const;
    size_t Size() const;

    /**
     * @brief Validate Used primarily for testing this container.
     */
    bool Validate() const;

private:
    struct Quad {
        Quad* parent_;
        std::optional<std::array<std::shared_ptr<Quad>, 4>> children_;

        Rect rect_;
        std::vector<std::shared_ptr<QuadTreeItem>> items_;
        std::vector<std::shared_ptr<QuadTreeItem>> entering_;

        Quad(Quad* parent, Rect rect);
    };

    std::shared_ptr<Quad> root_;
    uint64_t rootExpandedCount_;
    size_t itemCountTarget_;
    size_t itemCountLeeway_;
    double minQuadDiameter_;
    bool currentlyIterating_;

    template <typename Collide>
    void ForEachNode(Quad& node, Collide collide, const std::function<void(Quad& node)>& action)
    {
        if (node.children_.has_value()) {
            for (auto& child : node.children_.value()) {
                if (Collides(node.rect_, collide)) {
                    ForEachNode(*child, collide, action);
                }
            }
        }
        if (Collides(node.rect_, collide)) {
            action(node);
        }
    }
    template <typename Collide>
    void ForEachNode(const Quad& node, Collide collide, const std::function<void(const Quad& node)>& action) const
    {
        if (node.children_.has_value()) {
            for (const auto& child : node.children_.value()) {
                if (Collides(node.rect_, collide)) {
                    ForEachNode(*child, collide, action);
                }
            }
        }
        if (Collides(node.rect_, collide)) {
            action(node);
        }
    }
    void ForEachNode(Quad& node, const std::function<void(Quad& node)>& action);
    void ForEachNode(const Quad& node, const std::function<void(const Quad& node)>& action) const;

    void AddItem(Quad& startOfSearch, std::shared_ptr<QuadTreeItem> item, bool preventRebalance);
    Quad& NodeAt(Quad& startOfSearch, const Point& location);
    void Rebalance();
    size_t RecursiveItemCount(const Quad& node);
    std::vector<std::shared_ptr<QuadTreeItem>> RecursiveCollectItems(Quad& node);
    std::array<std::shared_ptr<Quad>, 4> CreateChildren(Quad& node);

    void ExpandRoot();
    void ContractRoot();

    size_t SubQuadIndex(const Rect& rect, const Point& p) const;
};

} // namespace Tril

#endif // QUADTREE_H
