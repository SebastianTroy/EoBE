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
    virtual const Circle& GetCollide() const = 0;
};

/**
 * @brief Not really an iterator so much as a convinience class encapsulating
 * various iteration options and associated helpers.
 */
class QuadTreeIterator {
public:
    QuadTreeIterator(std::function<void(std::shared_ptr<QuadTreeItem> item)>&& action);

    template <typename T>
    static QuadTreeIterator Create(const std::function<void(std::shared_ptr<T> item)>& action)
    {
        return QuadTreeIterator([&](std::shared_ptr<QuadTreeItem> item)
        {
            if (std::shared_ptr<T> castItem = std::dynamic_pointer_cast<T>(item)) {
                action(castItem);
            }
        });
    }

    QuadTreeIterator& SetQuadFilter(std::function<bool(const Rect& area)>&& filter);
    QuadTreeIterator& SetQuadFilter(const Rect& r);
    QuadTreeIterator& SetItemFilter(std::function<bool(const QuadTreeItem& item)>&& filter);
    QuadTreeIterator& SetItemFilter(const Point& p);
    QuadTreeIterator& SetItemFilter(const Line& l);
    QuadTreeIterator& SetItemFilter(const Circle& c);
    QuadTreeIterator& SetItemFilter(const Rect& r);
    QuadTreeIterator& SetRemoveItemPredicate(std::function<bool(const QuadTreeItem& item)>&& removeItemPredicate);

    template <typename T>
    QuadTreeIterator& SetRemoveItemPredicate(std::function<bool(const T& item)>&& removeItemPredicate)
    {
        SetRemoveItemPredicate([removeItemPredicate = std::move(removeItemPredicate)](const QuadTreeItem& item)
        {
            if (const T* castItem = dynamic_cast<const T*>(&item)) {
                return removeItemPredicate(*castItem);
            } else {
                return false;
            }
        });
        return *this;
    }

private:
    friend class QuadTree;

    std::function<void(std::shared_ptr<QuadTreeItem> item)> itemAction_;
    std::function<bool(const Rect& area)> quadFilter_;
    std::function<bool(const QuadTreeItem& item)> itemFilter_;
    std::function<bool(const QuadTreeItem& item)> removeItemPredicate_;
};

/**
 * @brief Not really an iterator so much as a convinience class encapsulating
 * various iteration options and associated helpers.
 */
class ConstQuadTreeIterator {
public:
    ConstQuadTreeIterator(std::function<void(const QuadTreeItem& item)>&& action);

    template <typename T>
    static ConstQuadTreeIterator Create(const std::function<void(const T& item)>& action)
    {
        return ConstQuadTreeIterator([&](const QuadTreeItem& item)
        {
            if (const T* castItem = dynamic_cast<const T*>(&item)) {
                action(*castItem);
            }
        });
    }

    ConstQuadTreeIterator& SetQuadFilter(std::function<bool(const Rect& area)>&& filter);
    ConstQuadTreeIterator& SetQuadFilter(const Rect& r);
    ConstQuadTreeIterator& SetItemFilter(std::function<bool(const QuadTreeItem& item)>&& filter);
    ConstQuadTreeIterator& SetItemFilter(const Point& p);
    ConstQuadTreeIterator& SetItemFilter(const Line& l);
    ConstQuadTreeIterator& SetItemFilter(const Circle& c);
    ConstQuadTreeIterator& SetItemFilter(const Rect& r);


    template <typename T>
    ConstQuadTreeIterator& SetItemFilter(std::function<bool(const T& item)>&& filter)
    {
        SetItemFilter([filter = std::move(filter)](const QuadTreeItem& item)
        {
            if (const T* castItem = dynamic_cast<const T*>(item)) {
                return filter(*castItem);
            } else {
                return false;
            }
        });
        return *this;
    }

private:
    friend class QuadTree;

    std::function<void(const QuadTreeItem& item)> itemAction_;
    std::function<bool(const Rect& area)> quadFilter_;
    std::function<bool(const QuadTreeItem& item)> itemFilter_;
};

class QuadTree {
public:
    QuadTree(const Rect& startArea, size_t itemCountTarget, size_t itemCountLeeway, double minQuadDiameter);

    void Insert(std::shared_ptr<QuadTreeItem> item);
    void Clear();
    void RemoveIf(const std::function<bool(const QuadTreeItem& item)>& predicate);


    void ForEachQuad(const std::function<void(const Rect& area)>& action) const;

    /**
     * @brief Allows an action to be undertaken for each item in turn, however
     * not all items are included, only those contained within quads for which
     * quadFilter(quadArea) returns true.
     * @param action Performed for each item in the specified quads, in an
     * unspecified order.
     * @param quadFilter Each quad is tested based on this predicate, failed
     * quads will be skipped, as will their children.
     */
    void ForEachItem(const ConstQuadTreeIterator& iter) const;

    /**
     * @brief ForEachItem Allows an action to be performed for each item that is
     * within a quad that passes the requirements of quadFilter. As this is non-
     * const, item locations may change during this call, so a rebalance will
     * need to be performed after the call. Only one rebalance will occur, even
     * if this is called mid iteration (i.e. during an item's action). The
     * removeItemPredicate allows for the removal of unwanted items, it is
     * equivalent to calling RemoveIf with the same predicate.
     * @param iter This helper encapsulates a number of components, the action
     * to be performed for each item, an optional Quad filter that can be used
     * to cull quads for efficiency, an optional item filter that can be used to
     * select which items to apply the action to, and a removeItemPredicate,
     * which is equivalent to calling RemoveIf with the same predicate, but
     * wrapped up in a single pass.
     */
    void ForEachItem(const QuadTreeIterator& iter);

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

    void ForEachQuad(Quad& quad, const std::function<void(Quad& quad)>& action);
    void ForEachQuad(Quad& quad, const std::function<void(Quad& quad)>& action, const std::function<bool(const Rect&)>& filter);
    void ForEachQuad(const Quad& quad, const std::function<void(const Quad& quad)>& action) const;
    void ForEachQuad(const Quad& quad, const std::function<void(const Quad& quad)>& action, const std::function<bool(const Rect&)>& filter) const;

    void AddItem(Quad& startOfSearch, std::shared_ptr<QuadTreeItem> item, bool preventRebalance);
    Quad& QuadAt(Quad& startOfSearch, const Point& location);
    void Rebalance();
    size_t RecursiveItemCount(const Quad& quad);
    std::vector<std::shared_ptr<QuadTreeItem>> RecursiveCollectItems(Quad& quad);
    std::array<std::shared_ptr<Quad>, 4> CreateChildren(Quad& quad);

    void ExpandRoot();
    void ContractRoot();

    size_t SubQuadIndex(const Rect& rect, const Point& p) const;
};

} // namespace Tril

#endif // QUADTREE_H
