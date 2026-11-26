#version 410 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D image;
uniform vec4 spriteColor;
uniform int nightMode;
uniform vec2 screenSize;

void main()
{
    vec4 color = spriteColor * texture(image, TexCoords);
    if (nightMode == 1)
    {
        vec2 uv = gl_FragCoord.xy / screenSize;
        float distanceFromCenter = distance(uv, vec2(0.5, 0.5));
        float vignette = smoothstep(0.25, 0.78, distanceFromCenter);
        color.rgb *= vec3(0.58, 0.68, 0.95);
        color.rgb *= (1.0 - vignette * 0.48);
    }
    FragColor = color;
}
