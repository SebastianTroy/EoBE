#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "Shape.h"
#include "JsonHelpers.h"

#include <nlohmann/json.hpp>
#include <fmt/format.h>

/**
 * @brief Placeholder for a proper matrix based coordinate system
 */
struct Transform {
public:
    double x;
    double y;
    double rotation;

    Transform operator+(const Transform& other) const
    {
        return { x + other.x, y + other.y, rotation + other.rotation };
    }

    static nlohmann::json Serialise(const Transform& transform);
    static std::optional<Transform> Deserialise(const nlohmann::json& transform);

private:
    static const inline std::string KEY_X = "x";
    static const inline std::string KEY_Y = "y";
    static const inline std::string KEY_Rotation = "Rotation";
};

template<>
struct fmt::formatter<Transform> : fmt::formatter<double>
{
    template <typename FormatContext>
    auto format(const Transform& transform, FormatContext& context)
    {
        auto&& out= context.out();
        format_to(out, "(x=");
        fmt::formatter<double>::format(transform.x, context);
        format_to(out, ", y=");
        fmt::formatter<double>::format(transform.y, context);
        format_to(out, ", rotation=");
        fmt::formatter<double>::format(transform.rotation, context);
        return format_to(out, ")");
    }
};


#endif // TRANSFORM_H
