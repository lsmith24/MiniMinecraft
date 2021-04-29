//#version 150

//uniform mat4 u_ViewProj;

//uniform ivec2 u_Dimensions; // Screen dimensions

//uniform vec3 u_Eye; // Camera pos

//uniform int u_Time;

//out vec4 outColor; //needs to be vec4

//const float PI = 3.14159265359;
//const float TWO_PI = 6.28318530718;

//// Sunset palette
//const vec3 sunset[5] = vec3[](vec3(255, 229, 119) / 255.0,
//                               vec3(254, 192, 81) / 255.0,
//                               vec3(255, 137, 103) / 255.0,
//                               vec3(253, 96, 81) / 255.0,
//                               vec3(57, 32, 51) / 255.0);
//// Dusk palette
//const vec3 dusk[5] = vec3[](vec3(144, 96, 144) / 255.0,
//                            vec3(96, 72, 120) / 255.0,
//                            vec3(72, 48, 120) / 255.0,
//                            vec3(48, 24, 96) / 255.0,
//                            vec3(0, 24, 72) / 255.0);

//const vec3 sunColor = vec3(255, 255, 190) / 255.0;
//const vec3 moonColor = vec3(210, 220, 230) / 255.0; //adding a moon color grayish blue
//const vec3 cloudColor = sunset[3];

////converts coords to polar
//vec2 sphereToUV(vec3 p) {
//    float phi = atan(p.z, p.x);
//    if(phi < 0) {
//        phi += TWO_PI;
//    }
//    float theta = acos(p.y);
//    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
//}

////linearly interpolates between different stages of the sky
////to blend colors nicely across sky
//vec3 uvToSunset(vec2 uv) {
//    if(uv.y < 0.5) {
//        return sunset[0];
//    }
//    else if(uv.y < 0.55) {
//        return mix(sunset[0], sunset[1], (uv.y - 0.5) / 0.05);
//    }
//    else if(uv.y < 0.6) {
//        return mix(sunset[1], sunset[2], (uv.y - 0.55) / 0.05);
//    }
//    else if(uv.y < 0.65) {
//        return mix(sunset[2], sunset[3], (uv.y - 0.6) / 0.05);
//    }
//    else if(uv.y < 0.75) {
//        return mix(sunset[3], sunset[4], (uv.y - 0.65) / 0.1);
//    }
//    return sunset[4];
//}

//vec3 uvToDusk(vec2 uv) {
//    if(uv.y < 0.5) {
//        return dusk[0];
//    }
//    else if(uv.y < 0.55) {
//        return mix(dusk[0], dusk[1], (uv.y - 0.5) / 0.05);
//    }
//    else if(uv.y < 0.6) {
//        return mix(dusk[1], dusk[2], (uv.y - 0.55) / 0.05);
//    }
//    else if(uv.y < 0.65) {
//        return mix(dusk[2], dusk[3], (uv.y - 0.6) / 0.05);
//    }
//    else if(uv.y < 0.75) {
//        return mix(dusk[3], dusk[4], (uv.y - 0.65) / 0.1);
//    }
//    return dusk[4];
//}

//vec2 random2( vec2 p ) {
//    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
//}

//vec3 random3( vec3 p ) {
//    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
//                          dot(p,vec3(269.5, 183.3, 765.54)),
//                          dot(p, vec3(420.69, 631.2,109.21))))
//                 *43758.5453);
//}

//float WorleyNoise3D(vec3 p)
//{
//    // Tile the space
//    vec3 pointInt = floor(p);
//    vec3 pointFract = fract(p);

//    float minDist = 1.0; // Minimum distance initialized to max.

//    // Search all neighboring cells and this cell for their point
//    for(int z = -1; z <= 1; z++)
//    {
//        for(int y = -1; y <= 1; y++)
//        {
//            for(int x = -1; x <= 1; x++)
//            {
//                vec3 neighbor = vec3(float(x), float(y), float(z));

//                // Random point inside current neighboring cell
//                vec3 point = random3(pointInt + neighbor);

//                // Animate the point
//                point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

//                // Compute the distance b/t the point and the fragment
//                // Store the min dist thus far
//                vec3 diff = neighbor + point - pointFract;
//                float dist = length(diff);
//                minDist = min(minDist, dist);
//            }
//        }
//    }
//    return minDist;
//}

////already have a function that does this for terrain
//float WorleyNoise(vec2 uv)
//{
//    // Tile the space
//    vec2 uvInt = floor(uv);
//    vec2 uvFract = fract(uv);

//    float minDist = 1.0; // Minimum distance initialized to max.

//    // Search all neighboring cells and this cell for their point
//    for(int y = -1; y <= 1; y++)
//    {
//        for(int x = -1; x <= 1; x++)
//        {
//            vec2 neighbor = vec2(float(x), float(y));

//            // Random point inside current neighboring cell
//            vec2 point = random2(uvInt + neighbor);

//            // Animate the point
//            point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

//            // Compute the distance b/t the point and the fragment
//            // Store the min dist thus far
//            vec2 diff = neighbor + point - uvFract;
//            float dist = length(diff);
//            minDist = min(minDist, dist);
//        }
//    }
//    return minDist;
//}

//float worleyFBM(vec3 uv) {
//    float sum = 0;
//    float freq = 4;
//    float amp = 0.5;
//    for(int i = 0; i < 8; i++) {
//        sum += WorleyNoise3D(uv * freq) * amp;
//        freq *= 2;
//        amp *= 0.5;
//    }
//    return sum;
//}

////want to look like sun is rising and setting
//vec3 rotateSun(vec3 pt, float angle) {
//    float s = sin(-1 * angle);
//    float c = cos(-1 * angle);
//    return vec3(pt.x, c * pt.y - s * pt.z, s * pt.y + c * pt.z);
//}


//void main()
//{
//    vec2 ndc = (gl_FragCoord.xy / vec2(u_Dimensions)) * 2.0 - 1.0; // -1 to 1 NDC
//    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
//    p *= 1000.0; // Times far clip plane value
//    p = u_ViewProj * p; // Convert from unhomogenized screen to world

//    vec3 rayDir = normalize(p.xyz - u_Eye);
//    vec2 uv = sphereToUV(rayDir); //covert ray

 //   float time = float(u_Time);

    // Add a glowing sun in the sky
//    vec3 sunDir = normalize(rotateSun(vec3(0, 0.1, 1.0), u_Time * 0.005)); //sun pos dependent on time
  //  vec3 sunDir = normalize(vec3(cos(u_Time), sin(u_Time), cos(u_Time)));
  //  vec2 sunUV = sphereToUV(sunDir);
  //  float sunSize = 4.0;
 //   float moonSize = 2.0;
  //  float angle = acos(dot(rayDir, sunDir)) * 180.0 / PI;
  //  float moonAngle = (PI - angle) * 180.0 / PI; //angle between moon and sun is static
//    vec3 tempColor;
//    vec3 skyColor = vec3(157, 204, 252) / 255.0;
//    vec3 nightColor = vec3(43, 76, 110) / 255.0;
//    vec3 offset = random3(rayDir); //for some noise in the sky

//    outColor = vec4(skyColor, 1);
//    return;

//    if (moonAngle < moonSize) {
//        tempColor = moonColor;
//    } else if (sunUV.y < 0.4 || sunDir.y < -0.9) {
//        tempColor = nightColor;
//    } else if (sunUV.y > 0.6) {
//        tempColor = skyColor;
//    } else {
//        float a = abs(sunUV.x - uv.x);
//        if (a > 0.5) {
//            a = 1.0 - a;
//        }
//        a = smoothstep(0.0, 1.0, a / 0.5);
//        tempColor = mix(uvToSunset(uv), uvToDusk(uv), a);

//        //interpolate btwn sunrise and sunset
//        if (sunUV.y < 0.5) {
//            float a2 = smoothstep(0.0, 1.0, (sunUV.y - 0.4) / 0.1);
//            tempColor = mix(nightColor, skyColor, a2);
//        } else {
//            float a2 = smoothstep(0.0, 1.0, (sunUV.y - 0.5) / 0.1);
//            tempColor = mix(tempColor, skyColor, a2);
//        }
//    }

//    // If the angle between our ray dir and vector to center of sun
//    // is less than the threshold, then we're looking at the sun
//    if(angle < sunSize) {
//        // Full center of sun
//        if(angle < sunSize / 2.0) {
//            tempColor = sunColor;
//        }
//        // Corona of sun, mix with sky color
//        else {
//            float a = smoothstep(0.0, 1.0, 2.0 * angle / sunSize - 1.0);
//            tempColor = mix(sunColor, tempColor, a);
//        }
//    }

//    //add a bit of noise to sky
//    offset *= 2.0;
//    offset -= vec3(1.0);
//    outColor = vec4(tempColor + offset * 0.1, 1);
//}

#version 150

uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
                            // from our CPU, but it's named u_ViewProj so we don't
                            // have to bother rewriting our ShaderProgram class

uniform ivec2 u_Dimensions; // Screen dimensions

uniform vec3 u_Eye; // Camera pos

uniform float u_Time;

out vec4 outColor;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

// Sunset palette
const vec3 sunset[5] = vec3[](vec3(255, 229, 119) / 255.0,
                               vec3(254, 192, 81) / 255.0,
                               vec3(255, 137, 103) / 255.0,
                               vec3(253, 96, 81) / 255.0,
                               vec3(57, 32, 51) / 255.0);
// Dusk palette
const vec3 dusk[5] = vec3[](vec3(144, 96, 144) / 255.0,
                            vec3(96, 72, 120) / 255.0,
                            vec3(72, 48, 120) / 255.0,
                            vec3(48, 24, 96) / 255.0,
                            vec3(0, 24, 72) / 255.0);

const vec3 sunColor = vec3(255, 255, 190) / 255.0;
const vec3 moonColor = vec3(210, 220, 230) / 255.0;
const vec3 cloudColor = sunset[3];

vec2 sphereToUV(vec3 p) {
    float phi = atan(p.z, p.x);
    if(phi < 0) {
        phi += TWO_PI;
    }
    float theta = acos(p.y);
    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
}

vec3 rotateSun(vec3 pt, float angle) {
    float s = sin(-1 * angle);
    float c = cos(-1 * angle);
    return vec3(pt.x, c * pt.y - s * pt.z, s * pt.y + c * pt.z);
}

vec3 uvToSunset(vec2 uv) {
    if(uv.y < 0.5) {
        return sunset[0];
    }
    else if(uv.y < 0.55) {
        return mix(sunset[0], sunset[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(sunset[1], sunset[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(sunset[2], sunset[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(sunset[3], sunset[4], (uv.y - 0.65) / 0.1);
    }
    return sunset[4];
}

vec3 uvToDusk(vec2 uv) {
    if(uv.y < 0.5) {
        return dusk[0];
    }
    else if(uv.y < 0.55) {
        return mix(dusk[0], dusk[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(dusk[1], dusk[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(dusk[2], dusk[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(dusk[3], dusk[4], (uv.y - 0.65) / 0.1);
    }
    return dusk[4];
}

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
                          dot(p,vec3(269.5, 183.3, 765.54)),
                          dot(p, vec3(420.69, 631.2,109.21))))
                 *43758.5453);
}

float WorleyNoise3D(vec3 p)
{
    // Tile the space
    vec3 pointInt = floor(p);
    vec3 pointFract = fract(p);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int z = -1; z <= 1; z++)
    {
        for(int y = -1; y <= 1; y++)
        {
            for(int x = -1; x <= 1; x++)
            {
                vec3 neighbor = vec3(float(x), float(y), float(z));

                // Random point inside current neighboring cell
                vec3 point = random3(pointInt + neighbor);

                // Animate the point
                point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

                // Compute the distance b/t the point and the fragment
                // Store the min dist thus far
                vec3 diff = neighbor + point - pointFract;
                float dist = length(diff);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
}

float WorleyNoise(vec2 uv)
{
    // Tile the space
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
            vec2 neighbor = vec2(float(x), float(y));

            // Random point inside current neighboring cell
            vec2 point = random2(uvInt + neighbor);

            // Animate the point
            point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

float worleyFBM(vec3 uv) {
    float sum = 0;
    float freq = 4;
    float amp = 0.5;
    for(int i = 0; i < 8; i++) {
        sum += WorleyNoise3D(uv * freq) * amp;
        freq *= 2;
        amp *= 0.5;
    }
    return sum;
}


void main()
{
    vec2 ndc = (gl_FragCoord.xy / vec2(u_Dimensions)) * 2.0 - 1.0; // -1 to 1 NDC

//    outColor = vec3(ndc * 0.5 + 0.5, 1);

    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
    p *= 1000.0; // Times far clip plane value
    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world

    vec3 rayDir = normalize(p.xyz - u_Eye);
    vec2 uv = sphereToUV(rayDir);
    vec2 offset = vec2(0.0);

    // Compute a gradient from the bottom of the sky-sphere to the top
    vec3 sunsetColor = uvToSunset(uv + offset * 0.1);
    vec3 duskColor = uvToDusk(uv + offset * 0.1);

    outColor = vec4(sunsetColor, 1.0);


    // Add a glowing sun in the sky
    //vec3 sunDir = normalize(vec3(cos(u_Time), sin(u_Time), cos(u_Time)));
    vec3 sunDir = normalize(rotateSun(vec3(0, 0.1, 1.0), u_Time * 0.005));
    float sunSize = 30;
    float moonSize = 10;
    float angle = acos(dot(rayDir, sunDir)) * 360.0 / PI;
    float moonAngle = (PI - angle) * 360.0 / PI;


    // If the angle between our ray dir and vector to center of sun
    // is less than the threshold, then we're looking at the sun
    if(angle < sunSize) {
        // Full center of sun
        if(angle < 7.5) {
            outColor = vec4(sunColor, 1.0);
        }
        // Corona of sun, mix with sky color
        else {
            outColor = vec4(mix(sunColor, sunsetColor, (angle - 7.5) / 22.5), 1.0);
        }
    }
    // Otherwise our ray is looking into just the sky
    else {
        float raySunDot = dot(rayDir, sunDir);
#define SUNSET_THRESHOLD 0.75
#define DUSK_THRESHOLD -0.1
        if(raySunDot > SUNSET_THRESHOLD) {
            // Do nothing, sky is already correct color
        }
        // Any dot product between 0.75 and -0.1 is a LERP b/t sunset and dusk color
        else if(raySunDot > DUSK_THRESHOLD) {
            float t = (raySunDot - SUNSET_THRESHOLD) / (DUSK_THRESHOLD - SUNSET_THRESHOLD);
            outColor = vec4(mix(outColor.xyz, duskColor, t), 1);
        }
        // Any dot product <= -0.1 are pure dusk color
        else {
            outColor = vec4(duskColor, 1);
        }
    }

}

