$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(colorTexture,  0);
uniform vec4 darknessFactor_lightFactor_lightRadius_lightDistance;
uniform vec4 angle;

void main()
{
    float darknessFactor = darknessFactor_lightFactor_lightRadius_lightDistance.x;
    float lightFactor = darknessFactor_lightFactor_lightRadius_lightDistance.y;
    float lightRadius = darknessFactor_lightFactor_lightRadius_lightDistance.z;
    float lightDistance = darknessFactor_lightFactor_lightRadius_lightDistance.w;

    vec2 lightPosition = vec2(0.5 + sin(angle.x) * lightDistance, 0.5 - cos(angle.x) * lightDistance);
    float distanceToLight = distance(v_texcoord0, lightPosition);
    float lighting = 0.0;

    if (distanceToLight < lightRadius)
    {
        lighting = 1.0 - pow(distanceToLight / lightRadius, 3.0);
    }

    float lightingFactor = darknessFactor + (lightFactor - darknessFactor) * lighting;
    vec4 sourceTextureColor = texture2D(colorTexture, v_texcoord0);
    vec3 lightedDiffuse = sourceTextureColor.rgb * lightingFactor;

    gl_FragColor = vec4(lightedDiffuse, sourceTextureColor.a);
}
