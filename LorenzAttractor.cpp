#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <iostream>
#include <vector> //for STL vector

using namespace std;

//The Lorenz equations
float dxdt(float x, float y){
    return 10.0*(y-x);
}

float dydt(float x, float y, float z) {
    return -1*x*z+28.0*x-y;
}

float dzdt(float x, float y, float z) {
    return x*y-(8.0/3.0)*z;
}

//Fourth order Runge-Kutta discrete integration
Vector3 rk4(Vector3 p, float dt) {
    float k0,k1,k2,k3;
    Vector3 np=p;
    k0=dt*dxdt(p.x,p.y);
    k1=dt*dxdt(p.x+k0/2.0,p.y);
    k2=dt*dxdt(p.x+k1/2.0,p.y);
    k3=dt*dxdt(p.x+k2,p.y);
    np.x=p.x+k0/6.0+k1/3.0+k2/3.0+k3/6.0;

    k0=dt*dydt(p.x,p.y,p.z);
    k1=dt*dydt(p.x,p.y+k0/2.0,p.z);
    k2=dt*dydt(p.x,p.y+k1/2.0,p.z);
    k3=dt*dydt(p.x,p.y+k2,p.z);
    np.y=p.y+k0/6.0+k1/3.0+k2/3.0+k3/6.0;

    k0=dt*dzdt(p.x,p.y,p.z);
    k1=dt*dzdt(p.x,p.y,p.z+k0/2.0);
    k2=dt*dzdt(p.x,p.y,p.z+k1/2.0);
    k3=dt*dzdt(p.x,p.y,p.z+k2);
    np.z=p.z+k0/6.0+k1/3.0+k2/3.0+k3/6.0;

    return np;
}

int main()
{
//Initialize Raylib
    InitWindow(800, 800, "Lorenz Attractor");
    SetWindowPosition(500,50);

    Camera camera = { 0 };
    camera.position = (Vector3){50.0, 50.0, 50.0};
    camera.target=(Vector3){0.0,0.0,0.0};
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    SetCameraMode(camera,CAMERA_FREE);
    SetTargetFPS(30);

    uint numPoints=8000;
    float ix=0.0,iy=1.0,iz=0.0;

    vector<Vector3>vec_list;

    while (!WindowShouldClose()){
//Update
    ix=GuiSlider({150,10,50,30},"X",ix,-40.0,40.0,true);
    iy=GuiSlider({250,10,50,30},"Y",iy,-40.0,40.0,true);
    iz=GuiSlider({350,10,50,30},"Z",iz,0.0,40.0,true);

    vec_list.clear();
    Vector3 pos={ix,iy,iz};
    for (uint i=0;i<numPoints;i++){
        vec_list.push_back(pos);
        pos=rk4(pos,0.01);
    }

//Draw
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        //DrawGrid(100, 1.0f);

        for (uint i=0;i<numPoints-1;i++){
            DrawLine3D(vec_list[i],vec_list[i+1],BLUE);
        }

        UpdateCamera(&camera);
        EndMode3D();
        DrawFPS(10,10);
        EndDrawing();
    }

    return 0;
}
