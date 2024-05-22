#version 460 core

out vec4 FragColor;
uniform vec2 u_resolution;
uniform vec2 u_center;
uniform float u_zoom;

void main()
{
    vec2 uv = (gl_FragCoord.xy / u_resolution - 0.5) * u_zoom + u_center;
    vec2 c = uv;
    vec2 z = vec2(0.0);
    int maxIterations = 10000;
    int i;
    for (i = 0; i < maxIterations; i++)
    {
        z = vec2(z.x * z.x - z.y * z.y + c.x, 2.0 * z.x * z.y + c.y);
        if (dot(z, z) > 4.0) break;
    }
    float t = float(i) / float(maxIterations);
    FragColor = vec4(vec3(t), 1.0);
}

