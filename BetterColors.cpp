#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <iostream>
#include <cstdio> //for snprintf
#include <cmath> //for fabs

using namespace std;

Color Xform_RYB2RGB(int r, int y, int b) {
    float rin=(float)r/255.0;
    float yin=(float)y/255.0;
    float bin=(float)b/255.0;

    //RYB corners to RGB values
    Vector3 CG000={0.0,0.0,0.0}; //Black
    Vector3 CG100={1.0,0.0,0.0}; //Red
    Vector3 CG010={1.0,1.0,0.0}; //Yellow = RGB Red+Green
    Vector3 CG001={0.0,0.0,1.0}; //Blue
    Vector3 CG011={0.0,1.0,0.0}; //Green
    Vector3 CG110={1.0,0.5,0.0}; //Orange = RGB full Red, half Green
    Vector3 CG101={0.75,0.0,1.0}; //Purple = RGB 3/4 Red, full Blue.  Looks better than half Red, full Blue
    Vector3 CG111={1.0,1.0,1.0}; //White

    Vector3 C00,C01,C10,C11;
    C00=Vector3Add(Vector3Scale(CG000,1.0-rin),Vector3Scale(CG100,rin));
    C01=Vector3Add(Vector3Scale(CG001,1.0-rin),Vector3Scale(CG101,rin));
    C10=Vector3Add(Vector3Scale(CG010,1.0-rin),Vector3Scale(CG110,rin));
    C11=Vector3Add(Vector3Scale(CG011,1.0-rin),Vector3Scale(CG111,rin));

    Vector3 C0,C1;
    C0=Vector3Add(Vector3Scale(C00,1.0-yin),Vector3Scale(C10,yin));
    C1=Vector3Add(Vector3Scale(C01,1.0-yin),Vector3Scale(C11,yin));

    Vector3 C;
    C=Vector3Add(Vector3Scale(C0,1.0-bin),Vector3Scale(C1,bin));

/*
    //normalize?
    float maxval=max(max(C.x,C.y),C.z);
    if (maxval<1.0) {
        C.x/=maxval;C.y/=maxval;C.z/=maxval;
    }
*/

    Color CRGB={255*C.x,255*C.y,255*C.z,255};

    return CRGB;
}

float step(float deg) {
    float out=0.0;
    while (deg<0.0) { deg+=360.0;}
    while (deg>360.0) { deg-=360.0;}

    if (deg<120.0) {
        out=1.0-deg/120.0;
    }
    else if ( (deg>120.0)&&(deg<=240.0) ) {
        out=0.0;
    }
    else if ( (deg>240) && (deg<360.0) ) {
        out=(deg-240.0)/120.0;
    }

return out;
}

Vector3 map2(float deg) {
    Vector3 out;

/*
    //Cosine based color spread around the wheel
    out.x=max(0,(int)(255*cos(DEG2RAD*(deg+00))));
    out.y=max(0,(int)(255*cos(DEG2RAD*(deg-120))));
    out.z=max(0,(int)(255*cos(DEG2RAD*(deg-240))));
*/


    //Sawtooth based color spread around the wheel
    out.x=255*step(deg);
    out.y=255*step(deg-120);
    out.z=255*step(deg-240);

return out;
}

Color brightener(Color in, float bright) {
    if (bright==0.0) {return in;}

    Color out;
    if (bright>0.0) {
        out.r=(1.0-bright)*in.r+bright*255.0;
        out.g=(1.0-bright)*in.g+bright*255.0;
        out.b=(1.0-bright)*in.b+bright*255.0;
    }

    if (bright<0.0) {
        out.r=(1.0+bright)*in.r-bright*0.0;
        out.g=(1.0+bright)*in.g-bright*0.0;
        out.b=(1.0+bright)*in.b-bright*0.0;
    }
    out.a=in.a;
return out;
}

Color saturate(Color in, float sat) {
    if (sat==0.0) {return in;}

    Color out;
    Vector3 clerp={(float)in.r/255.0,(float)in.g/255.0,(float)in.b/255.0};
    Vector3 maxsat;
    float mx=max(max(in.r,in.g),in.b);
    mx/=255.0;
    maxsat.x=clerp.x/mx;
    maxsat.y=clerp.y/mx;
    maxsat.z=clerp.z/mx;

    if (sat>0.0) {
        clerp.x=(1.0-sat)*clerp.x+sat*maxsat.x;
        clerp.y=(1.0-sat)*clerp.y+sat*maxsat.y;
        clerp.z=(1.0-sat)*clerp.z+sat*maxsat.z;
    }

    if (sat<0.0) {
        Vector3 grayc;
        float avg=(float) (in.r+in.g+in.b);
        avg/=(3.0*255.0);
        grayc={avg,avg,avg};
        clerp.x=(1.0+sat)*clerp.x-sat*grayc.x;
        clerp.y=(1.0+sat)*clerp.y-sat*grayc.y;
        clerp.z=(1.0+sat)*clerp.z-sat*grayc.z;
    }

    out={255*clerp.x,255*clerp.y,255*clerp.z,255};

return out;
}

int main()
{
//Initialize Raylib
    InitWindow(1500, 900, "Title");
    SetWindowPosition(600,50);

    Camera2D camera = { 0 };
    camera.target = (Vector2){ 0, 0 };
    camera.offset = (Vector2){ 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    SetTargetFPS(30);

    Color col=BLUE;
    Vector3 RYBCol={0,0,0};
    float degrees=0.0;
    float brightness=0.0;
    float sat=0.0;
    float picker=0.0;
    float myred=0.0,myyellow=0.0,myblue=0.0;

    float d2,d3;

    char textout[20];

    while (!WindowShouldClose()){

//Update


//Draw

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);

        for (int i=0;i<360;i+=5) {
            RYBCol=map2((float)i);
            col=Xform_RYB2RGB(RYBCol.x,RYBCol.y,RYBCol.z);
            col=saturate(col,sat);
            col=brightener(col,brightness);
            DrawCircleSector({400,350},300,i,i+5,6,col);
            //DrawCircleSector({1000,350},300,i,i+5,6,(Color){ar,ay,ab,255});
        }

        degrees=GuiSlider({250,700,360,20},NULL,NULL,degrees,0.0,359.0);
        snprintf(textout,sizeof textout,"%f",degrees);
        DrawText(textout,620,700,20,WHITE);
        DrawText("Hue:",200,700,20,WHITE);

        brightness=GuiSlider({250,750,360,20},NULL,NULL,brightness,-1.0,1.0);
        if (fabs(brightness)<0.05) {brightness=0.0;}
        snprintf(textout,sizeof textout,"%f",brightness);
        DrawText(textout,620,750,20,WHITE);
        DrawText("Tint/Shade:",120,750,20,WHITE);

        sat=GuiSlider({250,800,360,20},NULL,NULL,sat,-1.0,1.0);
        if (fabs(sat)<0.05) {sat=0.0;}
        snprintf(textout,sizeof textout,"%f",sat);
        DrawText(textout,620,800,20,WHITE);
        DrawText("Saturate:",140,800,20,WHITE);

        RYBCol=map2(degrees);
        col=Xform_RYB2RGB(RYBCol.x,RYBCol.y,RYBCol.z);
        col=saturate(col,sat);
        col=brightener(col,brightness);
        DrawRectangle(400+320*cos(DEG2RAD*(-degrees+90)),350+320*sin(DEG2RAD*(-degrees+90)),8,8,col);
        DrawRectangle(750,200,40,20,col);
        snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
        DrawText(textout,800,200,20,WHITE);

        d2=degrees+120.0;
        RYBCol=map2(d2);
        col=Xform_RYB2RGB(RYBCol.x,RYBCol.y,RYBCol.z);
        col=saturate(col,sat);
        col=brightener(col,brightness);
        DrawRectangle(400+320*cos(DEG2RAD*(-d2+90)),350+320*sin(DEG2RAD*(-d2+90)),8,8,col);
        DrawRectangle(750,250,40,20,col);
        snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
        DrawText(textout,800,250,20,WHITE);

        d3=degrees+240.0;
        RYBCol=map2(d3);
        col=Xform_RYB2RGB(RYBCol.x,RYBCol.y,RYBCol.z);
        col=saturate(col,sat);
        col=brightener(col,brightness);
        DrawRectangle(400+320*cos(DEG2RAD*(-d3+90)),350+320*sin(DEG2RAD*(-d3+90)),8,8,col);
        DrawRectangle(750,300,40,20,col);
        snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
        DrawText(textout,800,300,20,WHITE);

        picker=GuiSlider({950,50,510,20},NULL,NULL,picker,0,255);
        for (int i=0;i<255;i++) {
            for (int j=0;j<255;j++) {
                col=Xform_RYB2RGB((int)picker,i,j);
                col=saturate(col,sat);
                col=brightener(col,brightness);
                DrawRectangle(1000+i,100+j,5,5,col);
            }
        }

        myred=GuiSlider({950,400,255,20},"Red",NULL,myred,0.0,255.0);
        col=Xform_RYB2RGB(myred,0,0);
        DrawRectangle(1220,400,20,20,col);

        myyellow=GuiSlider({950,430,255,20},"Yellow",NULL,myyellow,0.0,255.0);
        col=Xform_RYB2RGB(0,myyellow,0);
        DrawRectangle(1220,430,20,20,col);

        myblue=GuiSlider({950,460,255,20},"Blue",NULL,myblue,0.0,255.0);
        col=Xform_RYB2RGB(0,0,myblue);
        DrawRectangle(1220,460,20,20,col);

        col=Xform_RYB2RGB(myred,myyellow,myblue);
        DrawRectangle(950,500,100,100,col);

        snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
        DrawText(textout,1100,500,20,WHITE);

        EndMode2D();
 //       DrawFPS(10,10);
        EndDrawing();
    }

    return 0;
}
