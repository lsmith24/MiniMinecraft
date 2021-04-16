#include "river.h"
#include "chunk.h"

River::River(Terrain *t, int xPos, int zPos) :
    xPosTerr(xPos), zPosTerr(zPos), depth(0), length(10), iter(3), grammer("FX"),
    turtles(QStack<Turtle>()), curTurtle(nullptr), terrain(t), drawingRules()
{
    for (int i = 0; i > iter; i++) {
        expandGrammer();
    }
}


void River::expandGrammer() {
    std::string str = "";
    for (int i = 0; i < int(grammer.length()); i++) {
        //expand string using lecture example
        if (grammer[i] == 'X') {
            double random = double(rand()) / RAND_MAX;
            if (random > 0.5) {
                str.append("[+FX][FX]-FX");
            } else {
                str.append("[+FX]-FX");
            }
        } else if (grammer[i] == 'F') {
            str.append("FF");
        } else {
            str.push_back(grammer[i]);
        }
    }
    this->grammer = str;
}

void River::expandWidth(int x, int z, int depth, int radius) {

    for (int i = -1 * radius; i <= radius; i++) {
        for (int j = -1 * radius; j <= radius; j++) {
            for (int k = -1 * radius; k <= radius; k++) {
                int xi = x + i;
                int zj = z + j;
                int terX = xPosTerr + 64;
                int terZ = zPosTerr + 64;

               // std::cout << "xi: " << xi << " zj: " << zj << " r + k + 150: " << radius + k + 150 << std::endl;
                if (xPosTerr <= xi && xi < terX && zPosTerr <= zj && zj < terZ) {
                    if ((i*i + j*j + k*k) <= (radius * radius)) {
                        if (k <= -1 * depth && terrain->hasChunkAt(xi, zj)) {
                            terrain->setBlockAt(xi, radius + k + 132, zj, WATER);
                        } else if (terrain->hasChunkAt(xi, zj)) {
                            terrain->setBlockAt(xi, radius + k + 132, zj, EMPTY);
                        }
                    } else if (terrain->hasChunkAt(xi, zj)){
                        for (int d = radius + 132; d < 255; d++) {
                            if (d <= radius * 2 + 132 && terrain->hasChunkAt(xi, zj)) {
                                terrain->setBlockAt(xi, d, zj, EMPTY);
                            } else {
                                if (terrain->getBlockAt(xi, d, zj) == EMPTY) {
                                    break;
                                }
                                if (terrain->hasChunkAt(xi, zj)) {
                                    terrain->setBlockAt(xi, d, zj, EMPTY);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}


void River::forwardLine() {
    float nextX;
    float nextZ;
    int xRot;
    int zRot;
    int rLength = std::max(int(length * depth), 15);
    double r = double(rand()) / RAND_MAX;

    //want random positive or negative not actual random number
    if (r >= 0.5) {
        r = 1;
    } else {
        r = -1;
    }

    for (int i = 0; i < rLength; i++) {
        float step = i * 2 * pi / rLength;
        float os = r * sin(step) * depth; //os -> offset

        nextX = curTurtle->xPos + os;
        nextZ = curTurtle->zPos + i;
        xRot = int(cos(curTurtle->rot) * (nextX - curTurtle->xPos) - sin(curTurtle->rot) * (nextZ - curTurtle->zPos) + (curTurtle->xPos));
        zRot = int(sin(curTurtle->rot) * (nextZ - curTurtle->xPos) + cos(curTurtle->rot) * (nextZ - curTurtle->zPos) + (curTurtle->zPos));
        //xRot = int(sin(curTurtle->rot) * 0.748327 + cos(curTurtle->rot));
        //zRot = int(sin(curTurtle->rot) * 0.98423 + cos(curTurtle->rot));
        expandWidth(xPosTerr + xRot, zPosTerr + zRot, depth, depth + 4);
        curTurtle->xPos = xRot;
        curTurtle->zPos = zRot;
    }

}


void River::makeRiver() {
    Turtle turtle = Turtle();
    turtles.push(turtle);
    curTurtle = &(turtles.top());

    for (int i = 0; i < int(grammer.length()); i++) {

        float n = 30 + rand() % 21;
        Turtle t = Turtle();
        t.setComps(curTurtle->xPos, curTurtle->zPos, curTurtle->rot);

        switch(grammer[i]) {
        case 'X':
            break;
        case 'F':
            depth++;
            if (i == int(grammer.length() - 1) || grammer[i + 1] != 'F') {
                forwardLine();
                depth = 0;
            }
            break;
        case '+':
            curTurtle->rot += n * pi / 180.f;
            break;
        case '-':
            curTurtle->rot -= n * pi / 180.f;
            break;
        case '[':
            turtles.push(t);
            curTurtle = &turtles.top();
            break;
        case ']':
            if (!turtles.isEmpty()) {
                turtles.pop();
            }

            curTurtle = &turtles.top();
            break;
        }
    }
}



















