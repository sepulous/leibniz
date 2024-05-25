#version 460 core

out vec4 FragColor;
uniform vec2 u_resolution;
uniform vec2 u_center;
uniform float u_zoom;

const int MAX_ITERATIONS = 1000;

precise int renderMandelbrot()
{
    float thing = u_resolution.x / (2 * u_resolution.y);
    vec2 uv = ((gl_FragCoord.xy / u_resolution - vec2(thing, 0.5)) / (vec2(u_zoom / (2 * thing), u_zoom) * 0.5) + (u_center - vec2(0.8, 0.08)));
    vec2 c = uv;
    vec2 z = vec2(0.0);
    float zx_squared;
    float zy_squared;
    float zx_times_zy;
    int i;
    for (i = 0; i < MAX_ITERATIONS; i++)
    {
        zx_squared = z.x * z.x;
        zy_squared = z.y * z.y;
        zx_times_zy = z.x * z.y;
        z.x = zx_squared - zy_squared + c.x;
        z.y = zx_times_zy + zx_times_zy + c.y;
        if (zx_squared + zy_squared > 4.0) break;
    }
    return i;
}

void main()
{
    int iterations = renderMandelbrot();
    float t = float(iterations) / float(MAX_ITERATIONS);
    FragColor = vec4(vec3(t), 1.0);
}

