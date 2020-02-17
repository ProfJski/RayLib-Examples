#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <iostream>

//First Draft at better colors: Adjusted HSV spectrum and RYB to RGB conversion
//By Prof. Jski, who is probably re-inventing the wheel here, pun intended.

using namespace std;

float mapper(float input) {
    while (input>360) {input-=360;}
    while (input<0) {input+=360;}

    float output=0.0;
    if ( (input>0) && (input<=60.0) ) {  //(0,60) to (0,45)
        output=0.75*input;
        return output;
    }
    if ( (input>60.0) && (input<=160.0) ) {  // (80,160) to (45,70)
        output=45.0+0.25*(input-60.0);
        return output;
    }
    if ( (input>160.0) && (input<=240.0) ) { //(160,240) to (70,230)
        output=70+2.0*(input-160.0);
        return output;
    }
    if ( (input>200.0) && (input<=360.0) ) { // (240,360) to (230,350)
        output=230+1.0*(input-240.0);
    return output;
}
return 0.0;
}

Color ryb2rgb(int r, int y, int b) {
    Color out;

    //Normalize on the way in -- my addition
    int normfactor=max(max(r,y),b);
    if (normfactor<255) {
        float n=255.0/(float)normfactor;
        r*=n;y*=n;b*=n;
    }

//    cout<<"Inputs. R="<<r<<" Y="<<y<<" B="<<b<<endl;
	int w=min(min(r,y),b); //w = whiteness
//	cout<<"whiteness="<<w<<endl;
    r-=w; y-=w; b-=w;

    int my=max(max(r,y),b);
//    cout<<"My="<<my<<endl;

    int g=min(y,b); //remove green from yellow and blue
    y-=g;
    b-=g;

    b*=2; g*=2;
    r+=y; g+=y;

    int mg=max(max(r,g),b);
//    cout<<"Mg="<<mg<<endl;
    if (mg>0) {
        float n=(float)my/(float)mg;
        r*=n; g*=n; b*=n;
    }

    r+=w; g+=w; b+=w;

//    cout<<"Output R="<<r<<" G="<<g<<" B="<<b<<endl;
    out.r=r; out.g=g; out.b=b; out.a=255;
    return out;
}



int main()
{
//Initialize Raylib
    InitWindow(1500, 800, "Title");
    SetWindowPosition(600,50);

    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0, 0 };
    camera.offset = (Vector2){ 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    SetTargetFPS(30);

    Color c=BLUE;
    Vector3 HSV=(Vector3){0,1,0};
    float degrees=0.0;

    int ar, ay, ab;

    while (!WindowShouldClose()){

//Update


//Draw

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);

/*
        c=GuiColorPicker({100,100,300,300},c);
        DrawRectangle(500,100,100,100,c);
*/

        degrees=GuiSlider({500,50,360,20},NULL,NULL,degrees,0.0,359.0);

        DrawText("Green-Dominated HSV wheel",40,10,20,GREEN);
        DrawText("Poor yellow range & color opposition",40,30,20,GREEN);
        for (int i=0;i<360;i+=5) {
            HSV=(Vector3){i,1.0,1.0};
            DrawCircleSector({250,300},200,i,i+5,6,ColorFromHSV(HSV));
            DrawRectangle(30+i,550,5,20,ColorFromHSV(HSV));
        }

        DrawText("Rebalanced HSV: primaries at 120 degrees",500,10,20,BLUE);
        for (int i=0;i<360;i+=5) {
            HSV=(Vector3){mapper(i),1.0,1.0};
            DrawCircleSector({700,300},200,i,i+5,6,ColorFromHSV(HSV));
            DrawRectangle(500+i,550,5,20,ColorFromHSV(HSV));
        }
        HSV=(Vector3){mapper(degrees),1.0,1.0};
        DrawRectangle(700+220*cos(DEG2RAD*(-degrees+90)),300+220*sin(DEG2RAD*(-degrees+90)),8,8,ColorFromHSV(HSV));
        DrawRectangle(950,100,40,20,ColorFromHSV(HSV));

        degrees+=120;
        HSV=(Vector3){mapper(degrees),1.0,1.0};
        DrawRectangle(700+220*cos(DEG2RAD*(-degrees+90)),300+220*sin(DEG2RAD*(-degrees+90)),5,5,ColorFromHSV(HSV));
        DrawRectangle(950,150,40,20,ColorFromHSV(HSV));

        degrees+=120;
        HSV=(Vector3){mapper(degrees),1.0,1.0};
        DrawRectangle(700+220*cos(DEG2RAD*(-degrees+90)),300+220*sin(DEG2RAD*(-degrees+90)),5,5,ColorFromHSV(HSV));
        DrawRectangle(950,200,40,20,ColorFromHSV(HSV));

/*
        for (int i=0;i<360;i+=5) {
            ar=max(0,(int)(255*cos(1.0*DEG2RAD*(i+00))));
            ay=max(0,(int)(255*cos(1.0*DEG2RAD*(i-120))));
            ab=max(0,(int)(255*cos(1.0*DEG2RAD*(i-240))));
            DrawCircleSector({650,250},200,i,i+5,6,ryb2rgb(ar,ay,ab));
//            DrawRectangle(10+i,500,5,20,ColorFromHSV(HSV));
        }
*/
        EndMode2D();
 //       DrawFPS(10,10);
        EndDrawing();
    }

    return 0;
}
