#version 460 core

out vec4 FragColor;
uniform vec2 u_resolution;
uniform vec2 u_center;
uniform float u_zoom;

const int MAX_ITERATIONS = 10000;

precise int renderMandelbrot()
{
    vec2 uv = (gl_FragCoord.xy / u_resolution - 0.5) * u_zoom + u_center;
    vec2 c = uv;
    vec2 z = vec2(0.0);
    int i;
    for (i = 0; i < MAX_ITERATIONS; i++)
    {
        z = vec2(z.x * z.x - z.y * z.y + c.x, 2.0 * z.x * z.y + c.y);
        if (z.x*z.x + z.y*z.y > 4.0) break;
    }
    return i;
}

void main()
{
    int iterations = renderMandelbrot();
    float t = float(iterations) / float(MAX_ITERATIONS);
    FragColor = vec4(vec3(t), 1.0);
}

