$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(screenTexture,  0);

void main()
{
    vec4 textureColor = texture2D(screenTexture, v_texcoord0);
    gl_FragColor = vec4(textureColor.x, 0.0, 0.0, 1.0);
}
