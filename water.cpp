#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "rlgl.h"
#include <iostream>

using namespace std;

//Ripple Tank Simulator by Eric J. Jenislawski
//RayLib graphics by Ramon Santamaria

struct matter {
    Vector3 gradient=(Vector3){0.0,0.0,0.0};
    Vector3 normal=(Vector3){0.0,0.0,0.0};
    float height=0.0;
    float vel=0.0;
    bool boundary=false;
};

struct sphere {
    Vector3 location=(Vector3){100.0,0.0,100.0};
    Vector3 velocity=(Vector3){0.0,0.0,0.0};
    Color color=GREEN;
};

Color colorizer(matter* matArray, int xsize, int zsize, int posx, int posz, Vector3 eye) {
    Color blue=BLUE,light=RAYWHITE;
    Vector3 blueparts=(Vector3){blue.r,blue.g,blue.b};
    Vector3 lightparts=(Vector3){light.r,light.g,light.b};
    Vector3 faderesults=(Vector3){0,0,0};
    Vector3 pos=(Vector3){posx,matArray[posx+posz*xsize].height,posz};
    Vector3 lightsource=(Vector3){0.0,100.0,0.0};
    Vector3 l_to_p=Vector3Subtract(lightsource,pos);
    float fade=0.0;

    if (matArray[posx+posz*xsize].boundary) {  // Draw boundaries in solid white
        return WHITE;
    }
    if ((posx==0)||(posz==0)||(posx>=xsize-1)||(posz>=zsize-1)) {  // Being lazy, not calculating reflection for voxels at border with a special routine
        return (Color){0,121,241,128};
    }

    fade=Vector3DotProduct(Vector3Reflect(Vector3Normalize(Vector3Subtract(pos,eye)),matArray[posx+posz*xsize].normal),Vector3Normalize(l_to_p));
    fade=(fade>0)?((fade>1)?1.0:fade):0.0; // clamp
    faderesults=Vector3Lerp(blueparts,lightparts,fade); //Blend our default view with white according to dot product result to give us reflection
    blue=(Color){faderesults.x,faderesults.y,faderesults.z,128}; //Return blended result, keeping transparency of 128
    return blue;

}


//Update height of voxel according to velocity
void update_height(matter* matArray, int xsize, int zsize) {
    for (int i=0;i<xsize;i++) {
        for (int j=0;j<zsize;j++) {
            if (!matArray[i+xsize*j].boundary) {
                matArray[i+xsize*j].height+=matArray[i+xsize*j].vel;
            }
        }
    }
return;
}


void update_vel(matter* matArray, int xsize, int zsize) {
    float temp, tempdiv;
    for (int i=0;i<xsize;i++) {
        for (int j=0;j<zsize;j++) {
            temp=0.0;
            tempdiv=0.0;
            for (int a=((i==0)?0:-1);a<((i==xsize-1)?0:2);a++) {
                for (int b=((j==0)?0:-1);b<((j==zsize-1)?0:2);b++) {
                    if (!matArray[(i+a)+xsize*(j+b)].boundary) {
                        temp+=(matArray[(i+a)+xsize*(j+b)].height-matArray[i+xsize*j].height);
                        tempdiv+=1.0;
                    }
                    else {
                        tempdiv+=1.0;
                    }
                }
            }
        matArray[i+xsize*j].vel+=0.5*temp/tempdiv; //Hooke's law
        matArray[i+xsize*j].vel*=0.995; //damping
        }
    }
return;
}

void calc_gradient_and_normal(matter* matArray, int xsize, int zsize) {
    float dx, dz;

    for (int i=1;i<xsize-1;i++) {
        for (int j=1;j<zsize-1;j++) {
            dx=(matArray[(i+1)+j*xsize].height-matArray[(i-1)+j*xsize].height)/2.0;
            dz=(matArray[i+(j+1)*xsize].height-matArray[i+(j-1)*xsize].height)/2.0;
            matArray[i+j*xsize].gradient=(Vector3){dx,0.0,dz};
            matArray[i+j*xsize].normal=Vector3Normalize(Vector3CrossProduct((Vector3){0.0,dz,1.0},(Vector3){1.0,dx,0.0}));
        }
    }
return;
}

void update_spheres(sphere* spheres, matter* matArray, int xsize, int zsize, int num_spheres) {
    int xpos, zpos;
    float depth, subvol, temp;
    Vector3 temploc=(Vector3){0.0,0.0,0.0}, tempvel=(Vector3){0.0,0.0,0.0};
    for (int s=0;s<num_spheres;s++) {
        temploc=(Vector3){0.0,0.0,0.0};
        tempvel=(Vector3){0.0,0.0,0.0};
        xpos=(int)spheres[s].location.x;
        zpos=(int)spheres[s].location.z;
        depth=matArray[xpos+zpos*xsize].height-spheres[s].location.y;
        //volume of sphere underwater if it is z of 2r submerged: pi*(-z^3/3+r*z^2) --> vol * mass of water displaced = bouyant force upward by Archimedes' principle
        if (depth<0) { //ball is higher than water, so falls straight down
            spheres[s].velocity.y-=0.9; // taking -.9 as accel grav
        }
        else { //otherwise it is touching or partially/wholly submerged
            if (depth>2.0) {depth=2.0;} // presuming sphere of radius one, if depth > 2.0, it is wholly submerged
            subvol=PI*(-1.0*depth*depth*depth/3.0+1.0*depth*depth); // 1.0 factor=radius of sphere;
            spheres[s].velocity.x-=5.0*matArray[xpos+zpos*xsize].gradient.x; // 5.0 arbitrary scale factor till it "looks right"
            spheres[s].velocity.z-=5.0*matArray[xpos+zpos*xsize].gradient.z;
            spheres[s].velocity.y+=subvol-0.9; //taking density of water here as 1.0, -0.9 because gravity still weighing it down too;
            spheres[s].velocity.y+=matArray[xpos+zpos*xsize].vel; //if submerged, water velocity upward will also transfer to sphere
            spheres[s].velocity=Vector3Add(spheres[s].velocity,Vector3Scale(spheres[s].velocity,-0.01*Vector3Length(spheres[s].velocity)));  //viscosity drag
        }

    temploc=Vector3Add(spheres[s].location,Vector3Scale(spheres[s].velocity,0.03));  // temporarily assign position before checking for collision with boundary
    if (matArray[(int)(temploc.x)+(int)(temploc.z)*xsize].boundary) {
        tempvel=spheres[s].velocity;
        temp=tempvel.x;  // Do a simple, kludgy 90-degree reflection using negative reciprocal of incident velocity
        tempvel.x=-tempvel.z;
        tempvel.z=temp;
        spheres[s].velocity=tempvel;
        spheres[s].location=Vector3Add(spheres[s].location,Vector3Scale(spheres[s].velocity,0.03));
    }
    else {
        spheres[s].location=temploc;
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
    SetTargetFPS(100);
    UpdateCamera(&camera);

    int xsize=200, zsize=200;
    matter matArray[xsize*zsize];
    int num_spheres=20;
    sphere spheres[20];
    Vector3 loc=(Vector3){0.0,0.0,0.0};
    bool paused=false;

    for (int i=0;i<xsize;i++) {
        for (int j=0;j<zsize;j++) {
            matArray[i+j*xsize].height=0.0;
            matArray[i+j*xsize].vel=0.0;
            //if ( abs( ((i-140)*(i-140)+(j-140)*(j-140)) -10000)<210) {
            //if ( (abs(i-100)<2)||(abs(i-140)<2)||(abs(j-20)<2)||(abs(j-220)<2)) {
            if ((i<2)||(i>xsize-2)||(j<2)||(j>zsize-2)) {
            //if (false) {
                matArray[i+j*xsize].boundary=true;
            }
            else {
                matArray[i+j*xsize].boundary=false;
            }

        }
    }


//Line boundary
    for (int i=0;i<xsize;i++) {
            matArray[i+40*xsize].boundary=true;
    }

/*  // Solid block boundary
    for (int i=90;i<111;i++) {
        for (int j=90;j<111;j++) {
            matArray[i+j*xsize].boundary=true;
        }
    }
*/

    for (int i=0,posx=0,posz=0;i<num_spheres;i++) {
        posx=rand()%xsize;
        posz=rand()%zsize;
        spheres[i].location=(Vector3){posx,1.0,posz};
        if (i%2==0) {spheres[i].color=GREEN;} else {spheres[i].color=RED;}
    }


    while (!WindowShouldClose()){
//Update
//Get Input

    if (IsKeyDown(KEY_A)) {  //Makes two drops to simulate a dipole
        matArray[50+xsize*150].height+=5.0;
        matArray[150+xsize*150].height+=5.0;
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {  // Lets you put drops wherever you want
        RayHitInfo result;
        result=GetCollisionRayGround(GetMouseRay(GetMousePosition(), camera),1.0);
        //cout<<"Hit at X:"<<result.position.x<<"Y"<<result.position.y<<"Z"<<result.position.z<<endl;  //Checks mouse hit is accurate, seems to be working OK
        if ((result.position.x>0)&&(result.position.x<xsize)&&(result.position.z>0)&&(result.position.z<zsize)){  // Discard mouse clicks out-of-bounds or else SEGFAULT on array access
            matArray[(int)(result.position.x)+xsize*(int)(result.position.z)].height+=20.0;
        }

    }

    if (IsKeyPressed(KEY_R)) {  // Reset the system = water is calm
        for (int i=0;i<xsize*zsize;i++) {
            matArray[i].vel=0.0;
            matArray[i].height=0.0;
        }
    }

    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {  // Right click lets you put boundaries wherever you want, hold E to erase
        RayHitInfo result;
        result=GetCollisionRayGround(GetMouseRay(GetMousePosition(), camera),1.0);
        //cout<<"Hit at X:"<<result.position.x<<"Y"<<result.position.y<<"Z"<<result.position.z<<endl;  //Checks mouse hit is accurate, seems to be working OK
        if ((result.position.x>0)&&(result.position.x<xsize)&&(result.position.z>0)&&(result.position.z<zsize)){  // Discard mouse clicks out-of-bounds or else SEGFAULT on array access
            matArray[(int)(result.position.x)+xsize*(int)(result.position.z)].height=0.0;
            matArray[(int)(result.position.x)+xsize*(int)(result.position.z)].vel=0.0;
            if (IsKeyDown(KEY_X)) {
                for (int i=0;i<xsize;i++) {
                    matArray[i+(int)(result.position.z)*xsize].boundary=!IsKeyDown(KEY_E);
                }
            }
            else if (IsKeyDown(KEY_Z)) {
                for (int j=0;j<zsize;j++) {
                    matArray[(int)(result.position.x)+j*xsize].boundary=!IsKeyDown(KEY_E);
                }
            }
            else {
                matArray[(int)(result.position.x)+xsize*(int)(result.position.z)].boundary=!IsKeyDown(KEY_E);
            }
        }
    }

    if (IsKeyPressed(KEY_P)) {  // Pauses the system
        paused=!paused;
    }

//Update math
    if (!paused){
        update_height(matArray,xsize,zsize);
        update_vel(matArray,xsize,zsize);
        calc_gradient_and_normal(matArray,xsize,zsize);
        update_spheres(spheres,matArray,xsize,zsize,num_spheres);
    }





//Draw

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(camera);
        rlDisableDepthTest();
        //DrawGrid(100, 1.0f);
        DrawGizmo({0,0,0});

        for (int i=0;i<xsize;i++) {
            for (int j=0;j<zsize;j++) {
                loc=(Vector3){i,matArray[i+xsize*j].height,j};
                DrawCube(loc,1.0,1.0,1.0,colorizer(matArray,xsize,zsize,i,j, camera.position));
                //DrawLine3D(loc,Vector3Add(loc,matArray[i+xsize*j].gradient),ORANGE);  // Uncomment to draw gradient
                //DrawLine3D(loc,Vector3Add(loc,matArray[i+xsize*j].normal),GREEN);  // Uncomment to draw normal to surface
            }
        }

        for (int s=0;s<num_spheres;s++) {
            DrawSphere(spheres[s].location,1.0,spheres[s].color);
        }

        UpdateCamera(&camera);
        EndMode3D();
        DrawFPS(10,10);
        EndDrawing();
    }

    return 0;
}
