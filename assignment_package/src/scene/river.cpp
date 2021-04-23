#include "river.h"
#include "chunk.h"

River::River(Terrain *t, int xPos, int zPos) :
    xPosTerr(xPos), zPosTerr(zPos), depth(0), length(15), iter(3), grammer("FX"),
    turtles(QStack<Turtle>()), curTurtle(), terrain(t), drawingRules()
{
    for (int i = 0; i < iter; i++) {
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

float dot2(glm::vec2 v ) { return glm::dot(v,v); }
float dot2(glm::vec3 v ) { return glm::dot(v,v); }
float sdRoundCone(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r1, float r2)
{
    // sampling independent computations (only depend on shape)
    glm::vec3  ba = b - a;
    float l2 = glm::dot(ba,ba);
    float rr = r1 - r2;
    float a2 = l2 - rr*rr;
    float il2 = 1.0/l2;

    // sampling dependant computations
    glm::vec3 pa = p - a;
    float y = glm::dot(pa,ba);
    float z = y - l2;
    float x2 = dot2( pa*l2 - ba*y );
    float y2 = y*y*l2;
    float z2 = z*z*l2;

    // single square root!
    float k = glm::sign(rr)*rr*rr*x2;
    if( glm::sign(z)*a2*z2 > k ) return  sqrt(x2 + z2)        *il2 - r2;
    if( glm::sign(y)*a2*y2 < k ) return  sqrt(x2 + y2)        *il2 - r1;
                            return (sqrt(x2*a2*il2)+y*rr)*il2 - r1;
}


void River::makeHalfCylinder(glm::ivec2 start, glm::ivec2 end, int r1, int r2) {
    //1. Make an axis-aligned bounding box for our line b/t start and end
    int minX = glm::min(start.x, end.x);
    int maxX = glm::max(start.x, end.x);
    int minZ = glm::min(start.y, end.y);
    int maxZ = glm::max(start.y, end.y);
    int maxRadius = glm::max(r1, r2);
    for(int x = minX; x < maxX; ++x) {
        // Adjust Y iteration to account for [river_height, 255]
        // For Ys > midpoint of cylinder, test their SDF value at Y = midpoint,
        // but use actual Y when setting terrain to EMPTY
        for(int y = 200 - maxRadius; y < 200 + maxRadius; ++y) {
            for(int z = minZ; z < maxZ; ++z) {
                glm::vec3 p(x, y, z);
                //water
                if (y <= 200) {
                    float sdf = sdRoundCone(p, glm::vec3(start.x, y, start.y), glm::vec3(end.x, y, end.y), r1, r2);
                    if(sdf <= 0) {
                        terrain->setBlockAt(x, y, z, WATER);
                    }
                } else if (y > 200) {
                    //empty, pretend y = 200
                    float sdf = sdRoundCone(p, glm::vec3(start.x, 200, start.y), glm::vec3(end.x, 200, end.y), r1, r2);
                    if(sdf <= 0) {
                        terrain->setBlockAt(x, y, z, EMPTY);
                    }
                }
            }
        }
    }
}

//NOT USING
void River::expandWidth(int x, int z, int depth, int radius) {

    // Iterate over XYZ axes within -radius to +radius
    // If XYZ is within the actual radius of the sphere,
    // set the block to WATER

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
                            terrain->setBlockAt(xi, radius + k + 200, zj, WATER);
                        } else if (terrain->hasChunkAt(xi, zj)) {
                            terrain->setBlockAt(xi, radius + k + 200, zj, WATER);
                        }
                    } else if (terrain->hasChunkAt(xi, zj)){
                        for (int d = radius + 200; d < 255; d++) {
                            if (d <= radius * 2 + 200 && terrain->hasChunkAt(xi, zj)) {
                                terrain->setBlockAt(xi, d, zj, WATER);
                            } else {
                                if (terrain->getBlockAt(xi, d, zj) == WATER) {
                                    break;
                                }
                                if (terrain->hasChunkAt(xi, zj)) {
                                    terrain->setBlockAt(xi, d, zj, WATER);
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

    nextX = curTurtle.xPos + length * cos(glm::radians(curTurtle.rot));
    nextZ = curTurtle.zPos + length * sin(glm::radians(curTurtle.rot));
    glm::vec2 dir = glm::normalize(glm::vec2(length * cos(glm::radians(curTurtle.rot)), length * sin(glm::radians(curTurtle.rot))));

    float cylX = curTurtle.xPos + dir.x * length;
    float cylZ = curTurtle.zPos + dir.y * length;
    makeHalfCylinder(glm::ivec2(curTurtle.xPos, curTurtle.zPos),
                     glm::ivec2(cylX, cylZ),
                     2, 1);

    for (int i = 0; i < length; i++) {

        float xt = curTurtle.xPos + dir.x * i;
        float zt = curTurtle.zPos + dir.y * i;
//        makeHalfCylinder(glm::ivec2(curTurtle.xPos, curTurtle.zPos),
//                         glm::ivec2(xt, zt),
//                         1, 1);

//        expandWidth(xPosTerr + xt, zPosTerr + zt, depth, 1);
        terrain->setBlockAt(xPosTerr + xt, 200, zPosTerr + zt, WATER);
    }

    curTurtle.xPos = nextX;
    curTurtle.zPos = nextZ;
}


void River::makeRiver() {
    curTurtle = Turtle();

    for (int i = 0; i < int(grammer.length()); i++) {
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
            curTurtle.rot += 45.f; //n * pi / 180.f;
            break;
        case '-':
            curTurtle.rot -= 45.f; //n * pi / 180.f;
            break;
        case '[':
            turtles.push(curTurtle);
            //curTurtle = &turtles.top();
            break;
        case ']':
            if (!turtles.isEmpty()) {
                curTurtle = turtles.pop();
            }
            break;
        }
    }
}



















