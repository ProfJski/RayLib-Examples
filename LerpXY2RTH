#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

//Convert Cartesian to polar
Vector2 conv(Vector2 in) {
    Vector2 out;
    out.x=sqrt(in.x*in.x+in.y*in.y);
    out.y=atan2(in.y,in.x);
return out;
}

//Convert polar to Cartesian
Vector2 ppol(Vector2 in) {
    Vector2 out;
    out.x=in.x*cos(in.y);
    out.y=in.x*sin(in.y);
return out;
}

//Simple LERP function for vector2
Vector2 VLerp(Vector2 a, Vector2 b, float l) {
    Vector2 out;
    out.x=(1.0-l)*a.x+l*b.x;
    out.y=(1.0-l)*a.y+l*b.y;
return out;
}

int main()
{
//Initialize Raylib
    InitWindow(1000, 1000, "Title");
    SetWindowPosition(600,50);

    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0, 0 };
    camera.offset = (Vector2){ 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    SetTargetFPS(30);

    vector<Vector2> pc;  //Holds points that make up the Cartesian grid lines
    vector<Vector2> rt;  //Points of the above after transformation to polar
    vector<Vector2> fc;  //Points of the function plotted in Cartesian space
    vector<Vector2> fr;  //Points of the above tranformed to R-Theta polar

    float step=0.005;

    //Draw vertical grid lines
    for (float x=-8;x<=8;x+=.5)
    {
        for (float y=-8;y<8;y+=step){
            pc.push_back((Vector2){x,y});
        }
    }

    //Draw horizontal grid lines
    for (float y=-8;y<=8;y+=.5)
    {
        for (float x=-8;x<8;x+=step){
            pc.push_back((Vector2){x,y});
        }
    }

    //Transform all those points to polar coordinates
    for (int i=0;i<pc.size();i++) {
        rt.push_back(ppol(pc[i]));
    }

    //Point for the function we wish to plot
    for (float x=-4;x<4.0;x+=step/8.0) {
        fc.push_back({x,2.0*sin(x*2.0*PI)});
        //fc.push_back({x,sqrt(4-x*x)});
        //fc.push_back({x,-1.0*sqrt(4-x*x)});
    }

    //The above transformed into polar
    for (int i=0;i<fc.size();i++) {
        fr.push_back(ppol(fc[i]));
    }


    float time=0.0;
    Vector2 temp;

    while (!WindowShouldClose()){

//Update


    if (time<=1.0) {
        time+=0.001;
    }


//Draw

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);

        for (int i=0;i<rt.size();i++) {
            temp=VLerp(pc[i],rt[i],time);  //Simply lerp between X-Y and R-Theta points defined above
            temp=Vector2Scale(temp,50);  //Scaled everything up by 50
            temp=Vector2Add(temp,{500,500}); //And centered it on the display
            DrawPixelV(temp,RED);
        }

        for (int i=0;i<fr.size();i++) {  //Did the same for the function
            temp=VLerp(fc[i],fr[i],time);
            temp=Vector2Scale(temp,50);
            temp=Vector2Add(temp,{500,500});
            DrawPixelV(temp,YELLOW);
        }

        EndMode2D();
        DrawFPS(10,10);
        EndDrawing();
    }

    return 0;
}
//There are a jillion ways to make this better.  I just wanted to post it because it's fun to watch
