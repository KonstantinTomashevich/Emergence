#pragma once

#include <Resource/Object/Object.hpp>

#include <StandardLayout/PatchBuilder.hpp>

namespace Emergence::Resource::Object::Test
{
void CheckChangelistEquality (const Container::Vector<ObjectComponent> &_first,
                              const Container::Vector<ObjectComponent> &_second) noexcept;

template <typename Type>
ObjectComponent MakeComponentPatch (const Type &_changed) noexcept
{
    static const Type initial {};
    return {StandardLayout::PatchBuilder::FromDifference (Type::Reflect ().mapping, &_changed, &initial)};
}
} // namespace Emergence::Resource::Object::Test
