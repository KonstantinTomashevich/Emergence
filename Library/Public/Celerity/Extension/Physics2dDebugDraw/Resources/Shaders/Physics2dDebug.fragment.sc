$input v_texcoord0

#include <bgfx_shader.sh>

uniform vec4 color;

void main()
{
    gl_FragColor = color;
}
