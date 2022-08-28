#pragma once

#include <Asset/Object/Object.hpp>

#include <StandardLayout/PatchBuilder.hpp>

namespace Emergence::Asset::Object::Test
{
void CheckChangelistEquality (const Container::Vector<StandardLayout::Patch> &_first,
                              const Container::Vector<StandardLayout::Patch> &_second) noexcept;

template <typename Type>
StandardLayout::Patch MakePatch (const Type &_changed) noexcept
{
    static const Type initial {};
    return StandardLayout::PatchBuilder::FromDifference (Type::Reflect ().mapping, &_changed, &initial);
}
} // namespace Emergence::Asset::Object::Test
