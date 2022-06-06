#include <Render/Urho3DAccessSingleton.hpp>

#include <StandardLayout/MappingRegistration.hpp>

Urho3DAccessSingleton::Reflection &Urho3DAccessSingleton::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        Emergence::StandardLayout::MappingBuilder builder;
        builder.Begin (Emergence::Memory::UniqueString {"Urho3DAccessSingleton"}, sizeof (Urho3DAccessSingleton),
                       alignof (Urho3DAccessSingleton));

        return Urho3DAccessSingleton::Reflection {
            .mapping = builder.End (),
        };
    }();

    return reflection;
}
