#pragma once

#include <Container/Vector.hpp>

#include <Memory/UniqueString.hpp>

Emergence::Memory::UniqueString GetLevelsConfigurationPath () noexcept;

Emergence::Memory::UniqueString GetLevelsPath () noexcept;

Emergence::Memory::UniqueString GetLocalizationPath () noexcept;

Emergence::Memory::UniqueString GetObjectsPath () noexcept;

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetFontPaths () noexcept;

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetMaterialInstancePaths () noexcept;

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetMaterialPaths () noexcept;

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetShadersPaths () noexcept;

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetTexturePaths () noexcept;
