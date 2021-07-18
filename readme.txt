-------------------------
Milestone I
-------------------------

Efficient Terrain Rendering and Chunking, Ryan Kenney

The terrain drawing function in MyGL ultimately draws every terrain generation zone in m_generatedTerrain through the Terrain draw function. At each tick, all terrain generation zones that are not around the player are deleted, and if there are too few, then new ones are created by creating or loading all of its chunks. To create the VBO data for a chunk, every block is checked and every adjacent empty block adds a face to the VBO. In order to only load one VBO, the position one was chosen; I attempted to create a separate vertex handle for referencing all info, but it ultimately caused the program to crash (although this was likely due to another problem fixed later). As such, the interleaved draw function references the data stored in the position VBO for all attributes.

Procedural Terrain, Lindsay Smith
In order to generate the terrain I first follow the descriptions for worley noise and perlin noise in the lecture slides. In the Terrain class I added functions that generated values using the noise functions. Then I used those values to determine heights for the grass biome and the mountain biomes each. The grass biome uses mainly worley noise, and the moutain biome uses mainly perlin noise. I then linearly interpolated those values with another perlin noise value in order to smooth between biomes. Depending on the height of the block I changed which type of block it should be as well. The grass biome has stone then dirt then grass. The mountains are all stone except for snow on the top. I set this by checking the height of the block being drawn. I also added an offset of the chunk x and chunk z to the createGenericChunk function so that the scene would properly render.

Player Physics, Brandon Reid
To check for player movement I have created boolean member variables for the Player class that flip to true if the corresponding key is pressed and false otherwise. Additionally, for camera rotation, I created member variables for the Camera class that store the current rotation and I use this to help calculate the new Camera axis. For creating a destroying and block, I used a grid march function that place/destroys the closest block the player is looking at. In flight mode the user can translate their local axis however, including clipping through terrain. But in non-flight mode the user cannot clip through terrain and will collide with any blocks in it’s hotbox. 
