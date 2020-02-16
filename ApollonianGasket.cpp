#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <iostream>
#include <complex>
#include <vector>
#include "rlgl.h"

using namespace std;

//Apollonian Gasket Generator by Eric J. Jenislawski
//Inspired by the article: https://mathlesstraveled.com/2016/06/10/apollonian-gaskets-and-descartes-theorem-ii/
//RayLib library by Ramon Santamaria

//Our "Apollonian circle" struct defines a circle uniquely by its signed bend = 1/radius, where bend is negative if the circle inscribes other circles
//and its bend product, defend below.
struct AppCircle {
    double bend;
    complex<double> bp; //Bend product: bend*complex representation of center (X,Y) as X+Yi
    Color color;
};


void DrawCircleLinesSmoother(int centerX, int centerY, float radius, Color color)
{
    if (rlCheckBufferLimit(2*72)) rlglDraw();

    rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        // NOTE: Circle outline is drawn pixel by pixel every degree (0 to 360)
        for (int i = 0; i < 360; i += 5)
        {
            rlVertex2f(centerX + sinf(DEG2RAD*i)*radius, centerY + cosf(DEG2RAD*i)*radius);
            rlVertex2f(centerX + sinf(DEG2RAD*(i + 5))*radius, centerY + cosf(DEG2RAD*(i + 5))*radius);
        }
    rlEnd();
}

Color colorblend(Color a, Color b, float lerp) {
    Color c;
    c.a=255;
    c.r=a.r*lerp+b.r*(1.0-lerp);
    c.r=a.g*lerp+b.g*(1.0-lerp);
    c.r=a.b*lerp+b.b*(1.0-lerp);
    return c;
}

void DrawACircle(AppCircle c) {
    int x,y;
    complex<double> bp (0.0,0.0);
    bp=c.bp/c.bend;
    x=(int)real(bp);
    y=(int)imag(bp);
    DrawCircleLinesSmoother(x,y,abs((int)1.0/c.bend),c.color);
    return;
}

void RecurseCircles(AppCircle a, AppCircle b, AppCircle c, vector<AppCircle>& circles, int rl) {
    if ( abs(a.bend>1.0) || abs(b.bend>1.0) || abs(c.bend>1.0) ) return; //If radius<1.0, bail out
    if (rl>10) return;

    rl++;

    AppCircle d, e;
    double s=a.bend+b.bend+c.bend;
    double r=2.0*pow((a.bend*b.bend+a.bend*c.bend+b.bend*c.bend),0.5);

    complex<double> cs=a.bp+b.bp+c.bp;
    complex<double> cr=2.0*pow((a.bp*b.bp+a.bp*c.bp+b.bp*c.bp),0.5);

    d.bend=s+r;
    d.bp=cs+cr;
    d.color=colorblend(RED,BLUE,1.0-((float)rl)/10.0);

    /*
    //Or calculate e this way
    e.bend=2.0*s-d.bend;
    e.bp=2.0*cs-d.bp;
    e.color=ORANGE;
    */

    circles.push_back(d);

    RecurseCircles(a, b, d, circles, rl);
    RecurseCircles(a, c, d, circles, rl);
    RecurseCircles(b, c, d, circles, rl);

    //We only need the outer enclosing circle with negative signed bend for the first level of recursion to fill the gasket
    //Removing the IF yields the same shape, but with much more redundancy, skyrocketing memory and plummeting FPS at higher recursion levels
    if (rl==1) {
        e.bend=s-r;
        e.bp=cs-cr;
        e.color=WHITE;

        circles.push_back(e);
        RecurseCircles(a, b, e, circles, rl);
        RecurseCircles(a, c, e, circles, rl);
        RecurseCircles(b, c, e, circles, rl);
    }

    return;
}

int main()
{
//Initialize Raylib
    InitWindow(1000, 900, "Apollonian Gasket");
    SetWindowPosition(600,50);

    Camera2D camera = { 0 };
    camera.target = (Vector2){ 400, 500 };
    camera.offset = (Vector2){ 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = 0.9f;

    SetTargetFPS(30);

    vector<AppCircle> circles;

    //Define our first three starter circles
    complex<double> loc (800.0,850.0);
    AppCircle circle1;
    circle1.bend=1.0/200.0;
    circle1.bp=loc*circle1.bend;
    circle1.color=RED;

    AppCircle circle2;
    loc = complex<double>(1200.0,850.0);
    circle2.bend=1.0/200.0;
    circle2.bp=loc*circle2.bend;
    circle2.color=WHITE;

    AppCircle circle3;
    loc = complex<double>(1000.0,1197.0);
    circle3.bend=1.0/200.0;
    circle3.bp=loc*circle3.bend;
    circle3.color=BLUE;

    circles.push_back(circle1);
    circles.push_back(circle2);
    circles.push_back(circle3);

    //Recurse and we're done: The beautiful math does the rest!
    RecurseCircles(circle1, circle2, circle3, circles, 0);
    cout<<"Number of circles="<<circles.size()<<endl;

    while (!WindowShouldClose()){
//Update
    //Zoom around if you'd like
    if (IsKeyDown(KEY_A)) camera.target.x-=5.0;
    if (IsKeyDown(KEY_D)) camera.target.x+=5.0;
    if (IsKeyDown(KEY_W)) camera.target.y-=5.0;
    if (IsKeyDown(KEY_S)) camera.target.y+=5.0;
    if (IsKeyDown(KEY_T)) camera.zoom*=1.1;
    if (IsKeyDown(KEY_G)) camera.zoom/=1.1;

//Draw

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);

        for (uint i=0;i<circles.size();i++) {
            DrawACircle(circles[i]);
        }

        EndMode2D();
        DrawFPS(10,10);
        EndDrawing();
    }

    return 0;
}
