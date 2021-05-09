#include <QuadTree.h>
#include <Random.h>

#include <catch2/catch.hpp>

using namespace Tril;

namespace {

class TestType : public QuadTreeItem {
public:
    Point location_;

    TestType(const Point& location)
        : location_(location)
    {
    }

    virtual const Point& GetLocation() const override
    {
        return location_;
    }
};

inline auto PointComparator = [](const Point& a, const Point& b)
{
    return a.x < b.x || (a.x == b.x && a.y < b.y);
};

}

TEST_CASE("QuadTree", "[container]")
{
    Random::Seed(42);

    SECTION("Empty Tree")
    {
        Rect area{ 0, 0, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        double minQuadSize = 1.0;
        QuadTree tree(area, targetCount, countLeeway, minQuadSize);

        REQUIRE(tree.Validate());
    }

    SECTION("One item - in bounds")
    {
        Rect area{ 0, 0, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        double minQuadSize = 1.0;
        QuadTree tree(area, targetCount, countLeeway, minQuadSize);

        Point testLoc{ 5.0, 5.0 };
        std::shared_ptr<QuadTreeItem> testItem = std::make_shared<TestType>(testLoc);

        tree.Insert(testItem);

        REQUIRE(tree.Validate());
        REQUIRE(tree.Size() == 1);
        REQUIRE(testItem->GetLocation() == testLoc);

        size_t count = 0;
        tree.ForEach([&](const QuadTreeItem& item)
        {
            ++count;
            REQUIRE(item.GetLocation() == testLoc);
        });

        REQUIRE(count == 1);
    }

    SECTION("Multiple items - in bounds, unique locations")
    {
        Rect area{ 0, 0, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        double minQuadSize = 1.0;
        QuadTree tree(area, targetCount, countLeeway, minQuadSize);

        size_t count = 25;
        std::vector<Point> testLocations;

        for (size_t i = 0; i < count; ++i) {
            testLocations.push_back(Random::PointIn(area));
            tree.Insert(std::make_shared<TestType>(testLocations.back()));
            REQUIRE(tree.Validate());
        }

        REQUIRE(tree.Size() == count);

        std::vector<Point> itemLocations;

        size_t itemCount = 0;
        tree.ForEach([&](const QuadTreeItem& item)
        {
            ++itemCount;
            itemLocations.push_back(item.GetLocation());
        });

        std::sort(std::begin(testLocations), std::end(testLocations), PointComparator);
        std::sort(std::begin(itemLocations), std::end(itemLocations), PointComparator);

        REQUIRE(itemLocations == testLocations);
        REQUIRE(itemCount == count);
    }

    SECTION("Multiple items - in bounds, same location")
    {
        Rect area{ 0, 0, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        double minQuadSize = 1.0;
        QuadTree tree(area, targetCount, countLeeway, minQuadSize);

        size_t count = 25;
        Point testLoc = Random::PointIn(area);

        for (size_t i = 0; i < count; ++i) {
            tree.Insert(std::make_shared<TestType>(testLoc));
        }

        REQUIRE(tree.Validate());
        REQUIRE(tree.Size() == count);

        size_t itemCount = 0;
        tree.ForEach([&](const QuadTreeItem& item)
        {
            ++itemCount;
            REQUIRE(item.GetLocation() == testLoc);
        });

        REQUIRE(itemCount == count);
    }

    SECTION("Clear")
    {
        Rect area{ 0, 0, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        double minQuadSize = 1.0;
        QuadTree tree(area, targetCount, countLeeway, minQuadSize);

        REQUIRE(tree.Validate());
        REQUIRE(tree.Size() == 0);
        tree.Clear();
        REQUIRE(tree.Validate());
        REQUIRE(tree.Size() == 0);

        for (int i = 0; i < 25; ++i) {
            tree.Insert(std::make_shared<TestType>(Random::PointIn(area)));
            REQUIRE(tree.Validate());
            REQUIRE(tree.Size() == 1);
            tree.Clear();
            REQUIRE(tree.Validate());
            REQUIRE(tree.Size() == 0);
        }
    }

    SECTION("Items out of bounds")
    {
        Rect area{ 0, 0, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        double minQuadSize = 1.0;

        SECTION("Single item - oob")
        {
            auto outOfBoundsPoints = { Point{ -1, -1 }, Point{ -1, 1 }, Point{ 1, -1 }, // around top left
                                       Point{ 11, -1 }, Point{ 11, 1 }, Point{ 9, -1 }, // around top right
                                       Point{ 11, 11 }, Point{ 11, 9 }, Point{ 9, 11 }, // around bottom right
                                       Point{ -1, 11 }, Point{ -1, 9 }, Point{ 1, 11 }, // around bottom left
                                       Point{ -100, 11 }, Point{ -1, 100 }, Point{ 100, 110 }, // way out of bounds
                                     };
            for (const auto& testLoc : outOfBoundsPoints) {
                QuadTree tree(area, targetCount, countLeeway, minQuadSize);
                tree.Insert(std::make_shared<TestType>(testLoc));
                REQUIRE(tree.Validate());
            }
        }

        SECTION("Multiple items - one oob")
        {
            auto outOfBoundsPoints = { Point{ -1, -1 }, Point{ -1, 1 }, Point{ 1, -1 }, // around top left
                                       Point{ 11, -1 }, Point{ 11, 1 }, Point{ 9, -1 }, // around top right
                                       Point{ 11, 11 }, Point{ 11, 9 }, Point{ 9, 11 }, // around bottom right
                                       Point{ -1, 11 }, Point{ -1, 9 }, Point{ 1, 11 }, // around bottom left
                                       Point{ -100, 11 }, Point{ -1, 100 }, Point{ 100, 110 }, // way out of bounds
                                     };
            for (const auto& testLoc : outOfBoundsPoints) {
                QuadTree tree(area, targetCount, countLeeway, minQuadSize);
                for (int i = 0; i < 25; ++i) {
                    tree.Insert(std::make_shared<TestType>(Random::PointIn(area)));
                }
                REQUIRE(tree.Validate());

                tree.Insert(std::make_shared<TestType>(testLoc));
                REQUIRE(tree.Validate());
            }
        }
    }

    SECTION("Removing items")
    {
        const Rect area{ 0, 0, 10, 10 };
        const Rect topArea{ 0, 0, 10, 5 };
        const Rect bottomArea{ 0, 5, 10, 10 };
        size_t targetCount = 1;
        size_t countLeeway = 0;
        const double minQuadSize = 1.0;
        const size_t itemCount = 50;
        QuadTree tree(area, targetCount, countLeeway, minQuadSize);

        auto removeTopItemsPredicate = [=](const QuadTreeItem& item)
        {
            return Contains(bottomArea, item.GetLocation());
        };

        for (size_t i = 0; i < itemCount / 2; ++i) {
            tree.Insert(std::make_shared<TestType>(Random::PointIn(topArea)));
            tree.Insert(std::make_shared<TestType>(Random::PointIn(bottomArea)));
        }

        SECTION("RemoveIf")
        {
            tree.RemoveIf(removeTopItemsPredicate);

            tree.ForEach([bottomArea](const QuadTreeItem& item)
            {
                REQUIRE(Contains(bottomArea, item.GetLocation()));
            });

            REQUIRE(tree.Size() == itemCount / 2);
            REQUIRE(tree.Validate());
        }

        SECTION("ForEach predicate")
        {
            tree.ForEach([](QuadTreeItem& /*item*/)
            {
                // Do nothing
            }, removeTopItemsPredicate);

            tree.ForEach([bottomArea](const QuadTreeItem& item)
            {
                REQUIRE(Contains(bottomArea, item.GetLocation()));
            });

            REQUIRE(tree.Size() == itemCount / 2);
            REQUIRE(tree.Validate());
        }
    }

    SECTION("Moving items")
    {
        const Rect area{ 0, 0, 10, 10 };
        const double minQuadSize = 1.0;
        const size_t itemCount = 50;
        std::vector<std::pair<size_t, size_t>> targetAndLeewayCombinations{
            { 1, 0 },
            { 5, 0 },
            { 5, 5 },
            { 0, itemCount },
            { itemCount, 0 },
            { itemCount, itemCount },
            { 1, 7 }, // make sure it does something sensible!
            { 0, 0 }, // make sure it does something sensible!
            { 0, 7 }, // make sure it does something sensible!
        };
        for (const auto& [ targetCount, countLeeway ] : targetAndLeewayCombinations) {
            QuadTree tree(area, targetCount, countLeeway, minQuadSize);

            for (size_t i = 0; i < itemCount; ++i) {
                tree.Insert(std::make_shared<TestType>(Random::PointIn(area)));
            }

            tree.ForEach<TestType>([=](TestType& item)
            {
                item.location_ = Random::PointIn(area);
            });

            REQUIRE(tree.Validate());
            REQUIRE(tree.Size() == itemCount);
        }
    }

    SECTION("Full use-case test")
    {
        const Rect startArea{ 0, 0, 10, 10 };
        const Rect movementArea{ -100, -100, 100, 100 };
        const double minQuadSize = 1.0;
        const size_t itemCount = 100;
        std::vector<std::pair<size_t, size_t>> targetAndLeewayCombinations{
            { 1, 0 },
            { 5, 0 },
            { 5, 5 },
            { 0, itemCount },
            { itemCount, 0 },
            { itemCount, itemCount },
            { 1, 7 }, // make sure it does something sensible!
            { 0, 0 }, // make sure it does something sensible!
            { 0, 7 }, // make sure it does something sensible!
        };
        for (const auto& [ targetCount, countLeeway ] : targetAndLeewayCombinations) {
            QuadTree tree(startArea, targetCount, countLeeway, minQuadSize);

            // Now go through a few a few test loops
            for (int i = 0; i < 100; ++i) {
                // Top up the tree, as we may have removed a few items
                size_t itemsToAdd = itemCount - tree.Size();
                for (size_t i = 0; i < itemsToAdd; ++i) {
                    tree.Insert(std::make_shared<TestType>(Random::PointIn(startArea)));
                }

                tree.ForEach<TestType>([=](TestType& item)
                {
                    item.location_ = Random::PointIn(movementArea);
                },
                [](const TestType& /*item*/) -> bool
                {
                    return Random::Boolean();
                });

                REQUIRE(tree.Validate());
                REQUIRE(tree.Size() != itemCount);
            }
        }
    }

    SECTION("Add items mid iteration")
    {
        const Rect area{ 0, 0, 10, 10 };
        const size_t targetCount = 1;
        const size_t countLeeway = 0;
        const double minQuadSize = 1.0;
        const size_t itemCount = 25;
        QuadTree tree(area, targetCount, countLeeway, minQuadSize);

        for (size_t i = 0; i < itemCount; ++i) {
            tree.Insert(std::make_shared<TestType>(Random::PointIn(area)));
        }

        tree.ForEach([&](auto& /*item*/)
        {
            tree.Insert(std::make_shared<TestType>(Random::PointIn(area)));
        });

        REQUIRE(tree.Validate());
        REQUIRE(tree.Size() == itemCount * 2);
    }
}
