#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <iostream>

//Water model by Eric J. Jenislawski
//RayLib graphics by Ramon Santamaria

using namespace std;

struct matter {
    float height=0.0;
    float vel=0.0;
};

//The colorizer function adds a simple reflection to the water surface.  It is badly written, but works fast enough.
//It computes the normal of the water surface, then the angle of reflection from the camera to the water surface
//Then takes the dot product of the reflected beam with the yhat vector, which points straight up but can be whatever you want as long as it is normalized
//This routine could doubtless be optimized -- is a quick kludge for a WIP
Color colorizer(matter* matArray, int xsize, int zsize, int posx, int posz, Vector3 eye) {
    Color blue=BLUE;
    Vector3 blueparts=(Vector3){blue.r,blue.g,blue.b};
    Vector3 whiteparts=(Vector3){255,255,255};
    Vector3 faderesults=(Vector3){0,0,0};
    Vector3 xvector, zvector, normal, pos;
    pos=(Vector3){posx,matArray[posx+xsize*posz].height,posz}; //position of water in voxel, just adding height for y coordinate
    Vector3 yhat=(Vector3){0.0,1.0,0.0}; // where our light source is
    float fade;
    if ((posx==0)||(posz==0)||(posx>=xsize-1)||(posz>=zsize-1)) {  // Being lazy, not calculating reflection for voxels at border with a special routine
        return BLUE;
    }
    //Below I approximate the slope of the water surface by averaging the heights of the nearest neighbors in the x and z axes.
    //Then I use that to calculate surface normal.  This is quick and dirty.  There are several more accurate ways to take a discrete derivative
    //This algorithm tends to introduce some pixelation at high frequency locations.  Basically we want gradient of height field here.
    //Low order finite difference approximation would be better here
    xvector=(Vector3){2,matArray[(posx+1)+xsize*posz].height-matArray[(posx-1)+xsize*posz].height,0}; //calculate dy/dx - slope along x axis
    zvector=(Vector3){0,matArray[posx+xsize*(posz+1)].height-matArray[posx+xsize*(posz-1)].height,2}; //calculate dz/dx - slope along z axis
    normal=Vector3Scale(Vector3Normalize(Vector3CrossProduct(xvector,zvector)),-1.0); // cross product gives us normal vector.  Scaled by -1.0 to point up.
    fade=Vector3DotProduct(Vector3Reflect(normal,Vector3Normalize(Vector3Subtract(pos,eye))),yhat); // Dot product of normal with reflection = how aligned reflection is with light source
    fade=abs(fade); // If I paid better attention to signs, maybe I wouldn't have to do abs() but it works
    faderesults=Vector3Lerp(blueparts,whiteparts,fade); //Blend our default view with white according to dot product result to give us reflection
    blue=(Color){faderesults.x,faderesults.y,faderesults.z,128}; //Return blended result, keeping transparency of 128
    return blue;

}

//Update height of voxel according to velocity
void update_height(matter* matArray, int xsize, int zsize) {
    for (int i=0;i<xsize;i++) {
        for (int j=0;j<zsize;j++) {
            matArray[i+xsize*j].height+=matArray[i+xsize*j].vel;
        }
    }
return;
}

//Apply acceleration (ie change to velocity) according to difference in height, using a Hooke's law force and damping factor
//We examine all 8 nearest neighbors when available, averaging them, to approximate gradient
//Funky ternary operators in for-loops deal with boundaries.  Tempdiv is a bit kludgy--just counts how many neighbors we are averaging
//This routine could probably be faster by hard-coding boundary calculations and eliminating tempdiv.
//As stands, boundaries contribute nothing, like a solid surface bounding a pool of water.
//If boundaries are fixed to zero value and counted in average, you can simulate a membrane like the surface of a drum.
void update_vel(matter* matArray, int xsize, int zsize) {
    float temp, tempdiv;
    for (int i=0;i<xsize;i++) {
        for (int j=0;j<zsize;j++) {
            temp=0.0;
            tempdiv=0.0;
            for (int a=((i==0)?0:-1);a<((i==xsize-1)?0:2);a++) {
                for (int b=((j==0)?0:-1);b<((j==zsize-1)?0:2);b++) {
                    temp+=(matArray[(i+a)+xsize*(j+b)].height-matArray[i+xsize*j].height);
                    tempdiv+=1.0;
                }
            }
        matArray[i+xsize*j].vel+=0.5*temp/tempdiv; //hooke's law
        matArray[i+xsize*j].vel*=0.99; //damping
        }
    }
return;
}


int main()
{
//Initialize Raylib
    InitWindow(800, 800, "Water");
    SetWindowPosition(500,50);

    Camera camera = { 0 };
    camera.position = (Vector3){-10.0, 10.0, -10.0};
    camera.target=(Vector3){25.0,0.0,25.0};
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.type = CAMERA_PERSPECTIVE;
    SetCameraMode(camera,CAMERA_FREE);
    SetTargetFPS(60);
    UpdateCamera(&camera);

    int xsize=100, zsize=100;
    matter matArray[xsize*zsize];

    while (!WindowShouldClose()){
//Update

    if (IsKeyDown(KEY_A)) {  //Makes two drops to simulate a dipole
        matArray[20+xsize*20].height+=5.0;
        matArray[80+xsize*20].height+=5.0;
    }
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {  // Lets you put drops wherever you want
        RayHitInfo result;
        result=GetCollisionRayGround(GetMouseRay(GetMousePosition(), camera),1.0);
        //cout<<"Hit at X:"<<result.position.x<<"Y"<<result.position.y<<"Z"<<result.position.z<<endl;  //Checks mouse hit is accurate, seems to be working OK
        if ((result.position.x>0)&&(result.position.x<xsize)&&(result.position.z>0)&&(result.position.z<zsize)){  // Discard mouse clicks out-of-bounds or else SEGFAULT on array access
            matArray[(int)(result.position.x)+xsize*(int)(result.position.z)].height+=5.0;
        }

    }

    //All the physics is in these two lines.  d2y/dt = GRADIENT(y)
    update_height(matArray,xsize,zsize);
    update_vel(matArray,xsize,zsize);

//Draw

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        DrawGrid(100, 1.0f);
        DrawGizmo({0,0,0});

        for (int i=1;i<(xsize-1);i++) {
            for (int j=1;j<(zsize-1);j++) {
                //If you don't want the reflection comment out the line below and uncomment the two lines beneath it to draw plain blue transparent cubes with blue line borders which help visibility
                DrawCube((Vector3){i,matArray[i+xsize*j].height,j},1.0,1.0,1.0,colorizer(matArray,xsize,zsize,i,j, camera.position));
                //DrawCube((Vector3){i,matArray[i+xsize*j].height,j},1.0,1.0,1.0,(Color){0,121,241,128});
                //DrawCubeWires((Vector3){i,matArray[i+xsize*j].height,j},1.0,1.0,1.0,BLUE);
            }
        }

        UpdateCamera(&camera);
        EndMode3D();
        DrawFPS(10,10);
        EndDrawing();
    }

    return 0;
}
