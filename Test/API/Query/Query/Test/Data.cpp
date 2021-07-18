#include <Query/Test/Data.hpp>
#include <Query/Test/DataTypes.hpp>

namespace Emergence::Query::Test
{
std::vector <Sources::Volumetric::Dimension> GetDimensions2D ()
{
    return
        {
            {-100.0f, BoundingBox::Reflection::minX, 100.0f, BoundingBox::Reflection::maxX},
            {-100.0f, BoundingBox::Reflection::minY, 100.0f, BoundingBox::Reflection::maxY}
        };
}

std::vector <Sources::Volumetric::Dimension> GetDimensions3D ()
{
    return
        {
            {-100.0f, BoundingBox::Reflection::minX, 100.0f, BoundingBox::Reflection::maxX},
            {-100.0f, BoundingBox::Reflection::minY, 100.0f, BoundingBox::Reflection::maxY},
            {-100.0f, BoundingBox::Reflection::minZ, 100.0f, BoundingBox::Reflection::maxZ},
        };
}

std::vector <Sources::Volumetric::Dimension> GetScreenRect ()
{
    return
        {
            {int16_t (-4096), ScreenRect::Reflection::minX, int16_t (4096), ScreenRect::Reflection::maxX},
            {int16_t (-4096), ScreenRect::Reflection::minY, int16_t (4096), ScreenRect::Reflection::maxY},
        };
}
} // namespace Emergence::Query::Test