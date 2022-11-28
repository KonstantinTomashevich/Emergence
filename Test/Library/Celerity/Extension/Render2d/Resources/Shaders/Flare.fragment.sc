$input v_texcoord0

#include <bgfx_shader.sh>

uniform vec4 flareRadius_flareStrength_flarePower;
uniform vec4 color;

void main()
{
    float flareRadius = flareRadius_flareStrength_flarePower.x;
    float flareStrength = flareRadius_flareStrength_flarePower.y;
    float flarePower = flareRadius_flareStrength_flarePower.z;

    float distanceToLight = distance(v_texcoord0, vec2(0.5, 0.5));
    float lighting = 0.0;

    if (distanceToLight < flareRadius)
    {
        lighting = flareStrength * (1.0 - pow(distanceToLight / flareRadius, flarePower));
    }

    gl_FragColor = vec4(color.xyz, lighting);
}
