#include <Configuration/Paths.hpp>

using namespace Emergence::Memory::Literals;

Emergence::Memory::UniqueString GetLevelsConfigurationPath () noexcept
{
    static const Emergence::Memory::UniqueString path {"../GameResources/Configuration/Levels.yaml"};
    return path;
}

Emergence::Memory::UniqueString GetLevelsPath () noexcept
{
    static const Emergence::Memory::UniqueString path {"../GameResources/Levels"};
    return path;
}

Emergence::Memory::UniqueString GetLocalizationPath () noexcept
{
    static const Emergence::Memory::UniqueString path {"../GameResources/Locales"};
    return path;
}

Emergence::Memory::UniqueString GetObjectsPath () noexcept
{
    static const Emergence::Memory::UniqueString path {"../GameResources/Objects"};
    return path;
}

Emergence::Memory::UniqueString GetResourceConfigRootPath () noexcept
{
    static const Emergence::Memory::UniqueString path {"../GameResources/Configuration"};
    return path;
}

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetFontPaths () noexcept
{
    static const Emergence::Container::Vector<Emergence::Memory::UniqueString> paths = [] ()
    {
        Emergence::Container::Vector<Emergence::Memory::UniqueString> vector {
            Emergence::Memory::Profiler::AllocationGroup {Emergence::Memory::Profiler::AllocationGroup::Top (),
                                                          "Paths"_us}};

        vector.emplace_back ("../GameResources/Fonts"_us);
        return vector;
    }();

    return paths;
}

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetMaterialInstancePaths () noexcept
{
    static const Emergence::Container::Vector<Emergence::Memory::UniqueString> paths = [] ()
    {
        Emergence::Container::Vector<Emergence::Memory::UniqueString> vector {
            Emergence::Memory::Profiler::AllocationGroup {Emergence::Memory::Profiler::AllocationGroup::Top (),
                                                          "Paths"_us}};

        vector.emplace_back ("../GameResources/MaterialInstances"_us);
        vector.emplace_back ("../Physics2dDebugDrawResources/MaterialInstances"_us);
        return vector;
    }();

    return paths;
}

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetMaterialPaths () noexcept
{
    static const Emergence::Container::Vector<Emergence::Memory::UniqueString> paths = [] ()
    {
        Emergence::Container::Vector<Emergence::Memory::UniqueString> vector {
            Emergence::Memory::Profiler::AllocationGroup {Emergence::Memory::Profiler::AllocationGroup::Top (),
                                                          "Paths"_us}};

        vector.emplace_back ("../GameResources/Materials"_us);
        vector.emplace_back ("../ImGUIResources/Materials"_us);
        vector.emplace_back ("../Physics2dDebugDrawResources/Materials"_us);
        vector.emplace_back ("../Render2dResources/Materials"_us);
        return vector;
    }();

    return paths;
}

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetShadersPaths () noexcept
{
    static const Emergence::Container::Vector<Emergence::Memory::UniqueString> paths = [] ()
    {
        Emergence::Container::Vector<Emergence::Memory::UniqueString> vector {
            Emergence::Memory::Profiler::AllocationGroup {Emergence::Memory::Profiler::AllocationGroup::Top (),
                                                          "Paths"_us}};

        vector.emplace_back ("../GameResources/Shaders"_us);
        vector.emplace_back ("../ImGUIResources/Shaders"_us);
        vector.emplace_back ("../Physics2dDebugDrawResources/Shaders"_us);
        vector.emplace_back ("../Render2dResources/Shaders"_us);
        return vector;
    }();

    return paths;
}

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetSpriteAnimationPaths () noexcept
{
    static const Emergence::Container::Vector<Emergence::Memory::UniqueString> paths = [] ()
    {
        Emergence::Container::Vector<Emergence::Memory::UniqueString> vector {
            Emergence::Memory::Profiler::AllocationGroup {Emergence::Memory::Profiler::AllocationGroup::Top (),
                                                          "Paths"_us}};

        vector.emplace_back ("../GameResources/Animations"_us);
        return vector;
    }();

    return paths;
}

const Emergence::Container::Vector<Emergence::Memory::UniqueString> &GetTexturePaths () noexcept
{
    static const Emergence::Container::Vector<Emergence::Memory::UniqueString> paths = [] ()
    {
        Emergence::Container::Vector<Emergence::Memory::UniqueString> vector {
            Emergence::Memory::Profiler::AllocationGroup {Emergence::Memory::Profiler::AllocationGroup::Top (),
                                                          "Paths"_us}};

        vector.emplace_back ("../GameResources/Textures"_us);
        return vector;
    }();

    return paths;
}
