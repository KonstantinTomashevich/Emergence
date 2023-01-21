$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

uniform vec4 hasColorTexture;
SAMPLER2D(colorTexture, 0);

void main()
{
    vec4 color = v_color0;
    if (hasColorTexture.x > 0.5)
    {
        vec4 texel = texture2D(colorTexture, v_texcoord0);
        color = texel * color;
    }

    gl_FragColor = color;
}
