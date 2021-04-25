#pragma once
#include "turtle.h"
#include "terrain.h"
#include <QStack>
#include <QMap>
#include <iostream>

class Terrain;
const float pi = 3.14159265358979323846;

class River {
public:
    int xPosTerr;
    int zPosTerr;
    int depth;
    int length;
    int iter;
    std::string grammer;
    QStack<Turtle> turtles;
    Turtle curTurtle;
    Terrain *terrain;

    typedef void (*Rule)(void);
    QMap<char, Rule> drawingRules;
    void expandGrammer();
    void expandWidth(int x, int z, int depth, int radius);
    void makeHalfCylinder(glm::ivec2 start, glm::ivec2 end, int r1, int r2);
    void forwardLine();
    void makeRiver();

    River(Terrain *t, int xPos, int zPos);
};
