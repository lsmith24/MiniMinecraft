#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this),
      m_terrain(this), m_player(glm::vec3(48.f, 140.f, 48.f), m_terrain)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.create();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    m_terrain.CreateTestScene();

    // Test goblin
    entities.push_back(mkU<Goblin>(Goblin(this, m_terrain, glm::vec3(49.5f, 150.0f, 20.5f))));
    entities.push_back(mkU<Goblin>(Goblin(this, m_terrain, glm::vec3(44.5f, 150.0f, 18.5f))));
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);

    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    m_terrain.expandChunks(m_player); // Checks if more chunks need to be loaded
    m_terrain.updateChunks(); // Move thread generated chunks to terrain
    m_terrain.updateVBOs();
    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    long long currframe = QDateTime::currentMSecsSinceEpoch();
    m_player.tick(currframe - lastFrame, m_inputs);
    for(const uPtr<Entity> &e : entities) {
        e->tick(currframe - lastFrame, m_inputs);
    }
    lastFrame = currframe;
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

void MyGL::paintRecursive(EntityNode* n, glm::mat4 transformation) {
    transformation = transformation * n->transformationMatrix();
    if (n->draw) {
        m_progLambert.setModelMatrix(transformation);
        m_progLambert.draw(n->square);
    }

    // Recursively repeat for this node's children
    for(const uPtr<EntityNode> &new_n : n->children) {
        paintRecursive(new_n.get(), transformation);
    }
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    renderTerrain();

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progFlat.draw(m_worldAxes);
    glEnable(GL_DEPTH_TEST);

    // Draw entities (test)
    for(const uPtr<Entity>& e: entities) {
        // Static cast for now
        Goblin* g = static_cast<Goblin*>(e.get());
        EntityNode* root = g->getRoot();
        // Only draw if they are within a certain xz distance (64 blocks for now)
        if (abs(m_player.mcr_position.x - root->t_x) < 64 && abs(m_player.mcr_position.z - root->t_z) < 64) {
            paintRecursive(root, glm::mat4());
        }
        m_progLambert.setModelMatrix(glm::mat4());
    }
}

// Renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain() {
    // Get the zone that the player is currently in
    int player_x = static_cast<int>(glm::floor(m_player.mcr_position[0] / 64.f) * 64);
    int player_z = static_cast<int>(glm::floor(m_player.mcr_position[2] / 64.f) * 64);
    // Just draw it all at once; 3x3 terrain generation zone around the player
    m_terrain.draw(player_x - 64, player_x + 128, player_z - 64, player_z + 128, &m_progLambert);
}


void MyGL::keyPressEvent(QKeyEvent *e) {
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = true;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = true;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = true;
    } else if (e->key() == Qt::Key_F) {
        m_inputs.fPressed = true;
    } else if (e->key() == Qt::Key_P) {
        // Create "pathing play pen"; a container with walls
        for (int x = 0; x < 16; ++x) {
            for(int y = 0; y < 256; ++y) {
                m_terrain.setBlockAt(x, y, 0, STONE);
                m_terrain.setBlockAt(x, y, 15, STONE);
                m_terrain.setBlockAt(0, y, x, STONE);
                m_terrain.setBlockAt(15, y, x, STONE);
            }
        }
        // Create walls within the pen
        for (int x = 0; x < 16; ++x) {
            for(int y = 0; y < 135; ++y) {
                m_terrain.setBlockAt(x + 2, y, x, STONE);
            }
        }
        // Create the "goal" that goblin will move towards
        m_terrain.setBlockAt(6, 129, 1, SNOW);

        m_terrain.getChunkAt(0, 0)->destroy();
        m_terrain.getChunkAt(0, 0)->create();

        // Make the goblin and path to test
        entities.push_back(mkU<Goblin>(Goblin(this, m_terrain, glm::vec3(2.0f, 150.0f, 2.0f))));
        Goblin* g = static_cast<Goblin*>(entities.back().get());
        g->findPath(6, 129, 1);
    } else if (e->key() == Qt::Key_O) {
        // Attracts nearby goblins (< 10 blocks)
        for(const uPtr<Entity>& e: entities) {
            // Static cast for now
            Goblin* g = static_cast<Goblin*>(e.get());
            bool res = g->findPath(int(m_player.mcr_position.x), int(m_player.mcr_position.y), int(m_player.mcr_position.z));
            if(!res) {
                std::cout << "Could not find path" << std::endl;
            }
        }
    } else if (e->key()  == Qt::Key_U) {
        // Add a goblin where the player is standing
        entities.push_back(mkU<Goblin>(Goblin(this, m_terrain, m_player.mcr_position)));
    }

}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = false;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = false;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = false;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = false;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = false;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = false;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
    } else if (e->key() == Qt::Key_F) {
        m_inputs.fPressed = false;
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    m_inputs.mouseX = e->x();
    m_inputs.mouseY = e->y();
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::RightButton) {
        m_inputs.rightClick = true;
    } else if (e->button() == Qt::LeftButton) {
        m_inputs.leftClick = true;
    }
}

void MyGL::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::RightButton) {
        m_inputs.rightClick = false;
    } else if (e->button() == Qt::LeftButton) {
        m_inputs.leftClick = false;
    }
}
