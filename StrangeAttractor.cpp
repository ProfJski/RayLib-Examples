#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "rlgl.h"
#include <iostream>
#include <vector> //for STL vector

using namespace std;

// Strange attractor discovered by Clifford A. Pickover
// An iterative system with x=sin(a*y)-z*cos(b*x), y=z*sin(c*x)-cos(d*y), z=e*sin(x)
// Vary the top sliders to demonstrate that the shape is an attractor: various starting states don't affect the shape
// Vary the bottom sliders to change the five parameters (a,b,c,d,e) and explore a wide variety of attractor shapes!
// K Points=1000s of points to plot.  More points fill in whispy objects.
// Inspiration by Roger T. Stevens, Fractal Programming in C
// Super handy RayLib graphics library and RayGUI immediate mode GUI by Ramon Santamaria
// Code by Eric J. Jenislawski

void DrawPoint3D(Vector3 pos, Color color) {
    if (rlCheckBufferLimit(8)) rlglDraw();
    rlPushMatrix();
    rlTranslatef(pos.x,pos.y,pos.z);
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, 64);
        rlVertex3f(0.0,0.0,0.0);
        rlVertex3f(0.02,0.0,0.02);
        rlVertex3f(0.0,0.0,0.0);
        rlVertex3f(0.02,0.02,0.0);
    rlEnd();

    rlPopMatrix();

return;
}

int main()
{
//Initialize Raylib
    InitWindow(800, 800, "Pickover Attractor");
    SetWindowPosition(500,50);

    Camera camera = { 0 };
    camera.position = (Vector3){30.0, 10.0, 30.0};
    camera.target=(Vector3){0.0,0.0,0.0};
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.type = CAMERA_PERSPECTIVE;
    SetCameraMode(camera,CAMERA_FREE);
    SetTargetFPS(30);

    uint numPoints=30000;
    int npf=30;
    float ix=0.0,iy=1.0,iz=0.0;
    float a=2.24,b=0.43,c=-0.65,d=-2.43,e=1.0;
    vector<Vector3>vec_list;

    while (!WindowShouldClose()){
//Update
    //The GUI
    npf=GuiSlider({650,10,100,20},"K points",npf,30,500,true);
    numPoints=(uint)1000*npf;

    ix=GuiSlider({150,10,50,20},"X",ix,-40.0,40.0,true);
    iy=GuiSlider({250,10,50,20},"Y",iy,-40.0,40.0,true);
    iz=GuiSlider({350,10,50,20},"Z",iz,-40.0,40.0,true);

    a=GuiSlider({15,750,100,20},"A",a,-5.0,5.0,true);
    b=GuiSlider({165,750,100,20},"B",b,-5.0,5.0,true);
    c=GuiSlider({315,750,100,20},"C",c,-5.0,5.0,true);
    d=GuiSlider({465,750,100,20},"D",d,-5.0,5.0,true);
    e=GuiSlider({615,750,100,20},"E",e,-5.0,5.0,true);


    vec_list.clear();
    Vector3 pos={ix,iy,iz};
    float tempx=0.0;
    for (uint i=0;i<numPoints;i++){
        vec_list.push_back(pos);
        //The attractor equations
        tempx=sin(a*pos.y)-pos.z*cos(b*pos.x);
        pos.y=pos.z*sin(c*pos.x)-cos(d*pos.y);
        pos.z=e*sin(pos.x);
        pos.x=tempx;
    }

//Draw
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);

        for (uint i=0;i<numPoints-1;i++){
            DrawPoint3D(vec_list[i],BLUE);
        }

        UpdateCamera(&camera);
        EndMode3D();
        DrawFPS(10,10);
        EndDrawing();

    }

    return 0;
}
