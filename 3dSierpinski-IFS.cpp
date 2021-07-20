#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "rlgl.h"
#include <iostream>
#include <cstdlib> //for RAND
#include <vector> //for STL vector

using namespace std;
//Draws a 3D Sierpinski Triangle as an Iterated Function System
//Draws a 3D Sierpinski Cube less nicely if you comment out the first routine and uncomment the second

void DrawPoint3D(Vector3 pos, Color color) {
    if (rlCheckBufferLimit(8)) rlglDraw();
    rlPushMatrix();
    rlTranslatef(pos.x,pos.y,pos.z);
    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        rlVertex3f(0.0,0.0,0.0);
        rlVertex3f(0.1,0.0,0.1);
        rlVertex3f(0.0,0.0,0.0);
        rlVertex3f(0.1,0.1,0.0);
    rlEnd();
    rlPopMatrix();
return;
}

Vector3 affine_transform(int num) {
    int random_number;
    int counter=0;
    float rx, ry, rz;
    rx = (float)rand()/RAND_MAX;
    ry = (float)rand()/RAND_MAX;
    rz = (float)rand()/RAND_MAX;
    Vector3 position=(Vector3){rx,ry,rz};


    while (counter<num){
        random_number=rand()%4;
        position=Vector3Scale(position,0.5);
        if (random_number==0) {        }
        if (random_number==1) position=Vector3Add(position,(Vector3){0.5,0.0,0.0});
        if (random_number==2) position=Vector3Add(position,(Vector3){0.0,0.5,0.0});
        if (random_number==3) position=Vector3Add(position,(Vector3){0.0,0.0,0.5});

    counter++;
    }

    /*
    //Change to this routine for a box.  Needs more points to fill
    while (counter<num){
        random_number=rand()%3;
        position=Vector3Scale(position,0.33333);
        if (random_number==0) {
            int random_number2=rand()%4;
            if (random_number2==0) position=Vector3Add(position,(Vector3){0.0,0.0,0.3333});
            if (random_number2==1) position=Vector3Add(position,(Vector3){0.0,0.66666,0.3333});
            if (random_number2==2) position=Vector3Add(position,(Vector3){0.66666,0.0,0.3333});
            if (random_number2==3) position=Vector3Add(position,(Vector3){0.66666,0.66666,0.3333});
            }
        else {
            int random_number2=rand()%8;
            float z=(random_number==1)?0.0:0.66666;
            if (random_number2==0) position=Vector3Add(position,(Vector3){0.0,0.0,z});
            if (random_number2==1) position=Vector3Add(position,(Vector3){0.0,0.33333,z});
            if (random_number2==2) position=Vector3Add(position,(Vector3){0.0,0.66666,z});
            if (random_number2==3) position=Vector3Add(position,(Vector3){0.33333,0.0,z});
            if (random_number2==4) position=Vector3Add(position,(Vector3){0.33333,0.66666,z});
            if (random_number2==5) position=Vector3Add(position,(Vector3){0.66666,0.0,z});
            if (random_number2==6) position=Vector3Add(position,(Vector3){0.66666,0.33333,z});
            if (random_number2==7) position=Vector3Add(position,(Vector3){0.66666,0.66666,z});
        }

    counter++;
    }
    */
    return position;
}


int main()
{
//Initialize Raylib
    InitWindow(800, 800, "Sierpinski Triangles");
    SetWindowPosition(500,50);

    Camera camera = { 0 };
    camera.position = (Vector3){100.0, 50.0, 100.0};
    camera.target=(Vector3){0.0,0.0,0.0};
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    SetCameraMode(camera,CAMERA_FREE);
    SetTargetFPS(60);

    uint numPoints=1000000; //number of points to plot
    int afn=20; //number of affine transforms to apply to each random initial point
    float scale=100.0;
    float cubeScale=0.1;
    Vector3 offset={50,0,50};

    vector<Vector3>vec_list;

    for (uint i=0;i<numPoints;i++){
        vec_list.push_back(Vector3Subtract(Vector3Scale(affine_transform(afn),scale),offset));
    }


    while (!WindowShouldClose()){
//Update

//Draw

        BeginDrawing();
        ClearBackground(BLACK);
        //cubeScale=GuiSlider({70,50,50,20},"Cube size",cubeScale,0.05,1,true);
        BeginMode3D(camera);
        DrawGrid(100, 1.0f);

        for (uint i=0;i<numPoints;i++){
            //DrawCube(vec_list[i],cubeScale,cubeScale,cubeScale,BLUE);
            DrawPoint3D(vec_list[i],RED);
        }

        UpdateCamera(&camera);
        EndMode3D();
        //DrawFPS(10,10);
        EndDrawing();

    }

    return 0;
}
