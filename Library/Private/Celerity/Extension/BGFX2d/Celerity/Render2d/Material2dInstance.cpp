#define _CRT_SECURE_NO_WARNINGS

#include <cassert>

#include <Celerity/Render2d/Material2d.hpp>
#include <Celerity/Render2d/Material2dInstance.hpp>

#include <Log/Log.hpp>

#include <StandardLayout/MappingRegistration.hpp>

namespace Emergence::Celerity
{
void Material2dInstance::InitForMaterial (const Material2d *_material) noexcept
{
    materialResourceId = _material->resourceId;
    const std::size_t uniformsSize =
        _material->uniforms.empty () ?
            0u :
            _material->uniforms.back ().offset + GetUniformSize (_material->uniforms.back ().type);
    uniforms.resize (uniformsSize, 0u);
}

void Material2dInstance::SetUniformById (const Material2d *_material,
                                         Memory::UniqueString _id,
                                         const void *_source) noexcept
{
    assert (_material->resourceId == materialResourceId);
    for (std::size_t index = 0u; index < _material->uniforms.size (); ++index)
    {
        const Uniform2d &uniform = _material->uniforms[index];
        if (uniform.id == _id)
        {
            SetUniformByIndex (_material, index, _source);
            return;
        }
    }

    EMERGENCE_LOG (ERROR, "Material2dInstance: Material \"", materialResourceId, "\", requested for instance \"", resourceId,
                   "\", has no uniform called \"", _id, "\"!");
}

void Material2dInstance::SetUniformByIndex (const Material2d *_material,
                                            std::size_t _index,
                                            const void *_source) noexcept
{
    assert (_material->resourceId == materialResourceId);
    assert (_index < _material->uniforms.size ());
    const Uniform2d &uniform = _material->uniforms[_index];
    assert (uniforms.size () >= uniform.offset + GetUniformSize (uniform.type));
    memcpy (uniforms.data () + uniform.offset, _source, GetUniformSize (uniform.type));
}

const Material2dInstance::Reflection &Material2dInstance::Reflect () noexcept
{
    static Reflection reflection = [] ()
    {
        EMERGENCE_MAPPING_REGISTRATION_BEGIN (Material2dInstance);
        EMERGENCE_MAPPING_REGISTER_REGULAR (resourceId);
        EMERGENCE_MAPPING_REGISTER_REGULAR (materialResourceId);
        EMERGENCE_MAPPING_REGISTRATION_END ();
    }();

    return reflection;
}
} // namespace Emergence::Celerity
