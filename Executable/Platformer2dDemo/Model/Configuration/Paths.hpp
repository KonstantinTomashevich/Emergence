#pragma once

#include <Container/Vector.hpp>

#include <Memory/UniqueString.hpp>

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetFontPaths () noexcept;

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetMaterialInstancePaths () noexcept;

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetMaterialPaths () noexcept;

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetShadersPaths () noexcept;

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetTexturePaths () noexcept;
