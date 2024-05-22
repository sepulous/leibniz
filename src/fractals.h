#ifndef FRACTALS_H
#define FRACTALS_H

#include <unordered_map>

enum class Fractal
{
    MANDELBROT
};

const std::unordered_map<Fractal, const char*> FRACTALS = {
    {Fractal::MANDELBROT, "Mandelbrot"}
};

void renderMandelbrot();

#endif
