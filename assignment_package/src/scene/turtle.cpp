#include "turtle.h"

Turtle::Turtle() :
    xPos(32), zPos(1), rot(0)
{}

void Turtle::setComps(int x, int z, float r) {
    this->xPos = x;
    this->zPos = z;
    this->rot = r;
}
