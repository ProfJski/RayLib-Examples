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

    //Balanced corners for better distribution of colors around hue circle and relative intensity of pure colors
    Vector3 CG000={0.0,0.0,0.0}; //Black
    Vector3 CG100={1.0,0.0,0.0}; //Red
    Vector3 CG010={0.9,0.9,0.0}; //Yellow = RGB Red+Green.  Still a bit high, but helps Yellow compete against Green.  Lower gives murky yellows.
    Vector3 CG001={0.0,0.36,1.0}; //Blue: Green boost of 0.36 helps eliminate flatness of spectrum around pure Blue
    Vector3 CG011={0.0,0.75,0.3}; //Green: A less intense green than {0,1,0}, which tends to dominate
    Vector3 CG110={1.0,0.6,0.0}; //Orange = RGB full Red, 60% Green
    Vector3 CG101={0.6,0.0,1.0}; //Purple = 60% Red, full Blue
    Vector3 CG111={1.0,1.0,1.0}; //White


/*
    //Unbalanced corners
    Vector3 CG000={0.0,0.0,0.0}; //Black
    Vector3 CG100={1.0,0.0,0.0}; //Red
    Vector3 CG010={1.0,1.0,0.0}; //Yellow = RGB Red+Green
    Vector3 CG001={0.0,0.0,1.0}; //Blue
    Vector3 CG011={0.0,1.0,0.0}; //Green
    Vector3 CG110={1.0,0.5,0.0}; //Orange = RGB full Red, Half Green
    Vector3 CG101={0.5,0.0,1.0}; //Purple = Half Red, full Blue
    Vector3 CG111={1.0,1.0,1.0}; //White
*/

    //Trilinear interpolation between RYB and RGB
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

    Color CRGB={255*C.x,255*C.y,255*C.z,255};

    return CRGB;
}

Vector3 col2vec(Color c) {
    Vector3 out=(Vector3){(float)c.r/255.0,(float)c.g/255.0,(float)c.b/255.0};
    return out;
}

Color ColorMix(Color a, Color b, float blend) {
    Color out;
    out.r=sqrt((1.0-blend)*(a.r*a.r)+blend*(b.r*b.r));
    out.g=sqrt((1.0-blend)*(a.g*a.g)+blend*(b.g*b.g));
    out.b=sqrt((1.0-blend)*(a.b*a.b)+blend*(b.b*b.b));
    out.a=(1.0-blend)*a.a+blend*b.a;

return out;
}

Color ColorMixLin(Color a, Color b, float blend) {
    Color out;
    out.r=(1.0-blend)*a.r+blend*b.r;
    out.g=(1.0-blend)*a.g+blend*b.g;
    out.b=(1.0-blend)*a.b+blend*b.b;
    out.a=(1.0-blend)*a.a+blend*b.a;

return out;
}

Color ColorInv(Color in) {
    Color out={255-in.r,255-in.g,255-in.b,255};
return out;
}

//Full color from 300 degrees to 60 degrees.  No color on [120,240].  Smoothstep transition in between.
float step2(float deg) {
    float out=0.0;
    float sc=0.0;
    while (deg<0.0) { deg+=360.0;}
    while (deg>360.0) { deg-=360.0;}

    if (deg<=60.0) {
        out=1.0;
    }
    else if ( (deg>60.0)&&(deg<=120.0) ) {
        sc=(deg-60.0)/60.0;
        out=1.0-2.0*sc/sqrt(1.0+3.0*sc*sc);
    }
    else if ( (deg>120.0) && (deg<=240.0) ) {
        out=0.0;
    }
    else if ( (deg>240.0) && (deg<=300.0) ) {
        sc=(deg-240.0)/60.0;
        out=2.0*sc/sqrt(1.0+3.0*sc*sc);
    }
    else if ( (deg>300.0) && (deg<=360.0) ) {
        out=1.0;
    }

return out;
}

Color map2(float deg) {
    Vector3 out;
    Color output;

    //Function based color spread around the wheel
    out.x=255*step2(deg);
    out.y=255*step2(deg-120);
    out.z=255*step2(deg-240);

    output=Xform_RYB2RGB(out.x,out.y,out.z);

return output;
}

Color brightener(Color in, float bright) {
    if (bright==0.0) {return in;}

    Color out;
    if (bright>0.0) {
        out=ColorMix(in,WHITE,bright);
    }

    if (bright<0.0) {
        out=ColorMix(in,BLACK,-1.0*bright);
    }
return out;
}

Color saturate(Color in, float sat) {
    if (sat==0.0) {return in;}
    if ((in.r==0)&&(in.g==0)&&(in.b==0)) {return in;}  //Prevents division by zero trying to saturate black

    Color out;
    Vector3 clerp={(float)in.r/255.0,(float)in.g/255.0,(float)in.b/255.0};

    if (sat>0.0) {
        Vector3 maxsat;
        float mx=max(max(in.r,in.g),in.b);
        mx/=255.0;
        maxsat.x=clerp.x/mx;
        maxsat.y=clerp.y/mx;
        maxsat.z=clerp.z/mx;
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

Color ColorProduct(Color a, Color b) {
    Color out;
    out.r=(a.r*b.r)/255;
    out.g=(a.g*b.g)/255;
    out.b=(a.b*b.b)/255;
    out.a=255;

return out;
}

Color ColorProductSq(Color a, Color b) {
    Color out;
    out.r=255-sqrt( ( (255-a.r)*(255-a.r)+(255-b.r)*(255-b.r) )/2.0 );
    out.g=255-sqrt( ( (255-a.g)*(255-a.g)+(255-b.g)*(255-b.g) )/2.0 );
    out.b=255-sqrt( ( (255-a.b)*(255-a.b)+(255-b.b)*(255-b.b) )/2.0 );
    out.a=255;

return out;
}

Color ColorScale(Color in, float scale) {
    Color out;
    out.r=(float)in.r*scale;
    out.g=(float)in.g*scale;
    out.b=(float)in.b*scale;
    out.a=255;
return out;
}

float ColorDistance(Color a, Color b) {
    float out=(float)((a.r-b.r)*(a.r-b.r)+(a.g-b.g)*(a.g-b.g)+(a.b-b.b)*(a.b-b.b));
    out=sqrt(out)/(sqrt(3.0)*255); //scale to 0-1
return out;
}

Color ColorMixSub(Color a, Color b, float blend) {
    Color out;
    Color c,d,f;

    c=ColorInv(a);
    d=ColorInv(b);

    f.r=max(0,255-c.r-d.r);
    f.g=max(0,255-c.g-d.g);
    f.b=max(0,255-c.b-d.b);

    float cd=ColorDistance(a,b);
    cd=4.0*blend*(1.0-blend)*cd;
    out=ColorMixLin(ColorMixLin(a,b,blend),f,cd);

    out.a=255;
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
    float myred2=0.0,myyellow2=0.0,myblue2=0.0;
    float blendfactor=0.0;
    float d2,d3,d4,d5;
    char textout[20];

    Vector2 CircleCenter={300,250};
    Vector2 MousePosition;

    Color mix_a, mix_b;

    while (!WindowShouldClose()){

//Update


//Draw

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);

        for (int i=0;i<360;i+=5) {
            col=map2((float)i);
            col=saturate(col,sat);
            col=brightener(col,brightness);
            DrawCircleSector(CircleCenter,200,i,i+5,6,col);
            //DrawCircleSector({1000,350},200,i,i+5,6,(Color){RYBCol.x,RYBCol.y,RYBCol.z,255});
        }

        degrees=GuiSlider({150,500,360,20},NULL,NULL,degrees,0.0,359.0);
        snprintf(textout,sizeof textout,"%f",degrees);
        DrawText(textout,520,500,20,WHITE);
        DrawText("Hue:",100,500,20,WHITE);

        brightness=GuiSlider({150,550,360,20},NULL,NULL,brightness,-1.0,1.0);
        if (fabs(brightness)<0.05) {brightness=0.0;}
        snprintf(textout,sizeof textout,"%f",brightness);
        DrawText(textout,520,550,20,WHITE);
        DrawText("Tint/Shade:",20,550,20,WHITE);

        sat=GuiSlider({150,600,360,20},NULL,NULL,sat,-1.0,1.0);
        if (fabs(sat)<0.05) {sat=0.0;}
        snprintf(textout,sizeof textout,"%f",sat);
        DrawText(textout,520,600,20,WHITE);
        DrawText("Saturate:",40,600,20,WHITE);

        col=map2(degrees);
        col=saturate(col,sat);
        col=brightener(col,brightness);
        DrawRectangle(CircleCenter.x+220*cos(DEG2RAD*(-degrees+90)),CircleCenter.y+220*sin(DEG2RAD*(-degrees+90)),10,10,col);
        DrawRectangle(550,200,40,20,col);
        snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
        DrawText(textout,600,200,20,WHITE);

        d2=degrees+150.0;
        col=map2(d2);
        col=saturate(col,sat);
        col=brightener(col,brightness);
        DrawRectangle(CircleCenter.x+220*cos(DEG2RAD*(-d2+90)),CircleCenter.y+220*sin(DEG2RAD*(-d2+90)),8,8,col);
        DrawRectangle(550,250,40,20,col);
        snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
        DrawText(textout,600,250,20,WHITE);

        d3=degrees+210.0;
        col=map2(d3);
        col=saturate(col,sat);
        col=brightener(col,brightness);
        DrawRectangle(CircleCenter.x+220*cos(DEG2RAD*(-d3+90)),CircleCenter.y+220*sin(DEG2RAD*(-d3+90)),8,8,col);
        DrawRectangle(550,300,40,20,col);
        snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
        DrawText(textout,600,300,20,WHITE);

/*
        d4=degrees+45.0;
        col=map2(d4);
        col=saturate(col,sat);
        col=brightener(col,brightness);
        DrawRectangle(CircleCenter.x+220*cos(DEG2RAD*(-d4+90)),CircleCenter.y+220*sin(DEG2RAD*(-d4+90)),8,8,col);
        DrawRectangle(550,350,40,20,col);
        snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
        DrawText(textout,600,350,20,WHITE);

        d5=degrees+60.0;
        col=map2(d5);
        col=saturate(col,sat);
        col=brightener(col,brightness);
        DrawRectangle(CircleCenter.x+220*cos(DEG2RAD*(-d5+90)),CircleCenter.y+220*sin(DEG2RAD*(-d5+90)),8,8,col);
        DrawRectangle(550,400,40,20,col);
        snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
        DrawText(textout,600,400,20,WHITE);
*/
/*
        picker=GuiSlider({950,50,510,20},NULL,NULL,picker,0,255);
        for (int i=0;i<255;i++) {
            for (int j=0;j<255;j++) {
                col=Xform_RYB2RGB((int)picker,i,j);
                col=saturate(col,sat);
                col=brightener(col,brightness);
                DrawRectangle(1000+i,100+j,5,5,col);
            }
        }
*/


//Palettizer
        col=map2(degrees);
        Color PCol;

        for (float i=-0.75;i<1.0;i+=0.35) {
            for (float j=-0.75;j<0.9;j+=0.35) {
                PCol=saturate(col,i);
                PCol=brightener(PCol,j);
                DrawRectangle(1000+i*300,200+j*120,20,20,PCol);
            }
        }

        col=map2(d2);

        for (float i=-0.75;i<1.0;i+=0.35) {
            for (float j=-0.75;j<0.9;j+=0.35) {
                PCol=saturate(col,i);
                PCol=brightener(PCol,j);
                DrawRectangle(1020+i*300,200+j*120,20,20,PCol);
            }
        }

        col=map2(d3);

        for (float i=-0.75;i<1.0;i+=0.35) {
            for (float j=-0.75;j<1.0;j+=0.35) {
                PCol=saturate(col,i);
                PCol=brightener(PCol,j);
                DrawRectangle(1040+i*300,200+j*120,20,20,PCol);
            }
        }



//Custom blender
        myred=GuiSlider({100,750,255,20},"Red",NULL,myred,0.0,255.0);
        col=Xform_RYB2RGB(myred,0,0);
        DrawRectangle(360,750,20,20,col);

        myyellow=GuiSlider({100,780,255,20},"Yellow",NULL,myyellow,0.0,255.0);
        col=Xform_RYB2RGB(0,myyellow,0);
        DrawRectangle(360,780,20,20,col);

        myblue=GuiSlider({100,810,255,20},"Blue",NULL,myblue,0.0,255.0);
        col=Xform_RYB2RGB(0,0,myblue);
        DrawRectangle(360,810,20,20,col);

        col=Xform_RYB2RGB(myred,myyellow,myblue);
        DrawRectangle(400,750,100,100,col);

        snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
        DrawText(textout,520,750,20,WHITE);
        mix_a=col;


        myred2=GuiSlider({700,750,255,20},"Red",NULL,myred2,0.0,255.0);
        col=Xform_RYB2RGB(myred2,0,0);
        DrawRectangle(960,750,20,20,col);

        myyellow2=GuiSlider({700,780,255,20},"Yellow",NULL,myyellow2,0.0,255.0);
        col=Xform_RYB2RGB(0,myyellow2,0);
        DrawRectangle(960,780,20,20,col);

        myblue2=GuiSlider({700,810,255,20},"Blue",NULL,myblue2,0.0,255.0);
        col=Xform_RYB2RGB(0,0,myblue2);
        DrawRectangle(960,810,20,20,col);

        col=Xform_RYB2RGB(myred2,myyellow2,myblue2);
        DrawRectangle(1000,750,100,100,col);

        snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
        DrawText(textout,1120,750,20,WHITE);

        mix_b=col;

/*
        col=ColorMix(mix_a,mix_b,blendfactor);
        DrawRectangle(1000,500,50,50,col);
        snprintf(textout,sizeof textout,"%f",blendfactor);
        blendfactor=GuiSlider({950,450,255,20},"Blend",textout,blendfactor,0.0,1.0);
        snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
        DrawText(textout,1070,500,20,WHITE);

        col=ColorMixLin(mix_a,mix_b,blendfactor);
        DrawRectangle(1000,600,50,50,col);
        snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
        DrawText(textout,1070,600,20,WHITE);
*/

        for (float i=0.0;i<1.1;i+=0.1) {
            col=ColorMix(mix_a,mix_b,i);
            DrawRectangle(750+(600*i),460,60,50,col);
            snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
            DrawText(textout,750+(600*i),510,10,WHITE);

            col=ColorMixLin(mix_a,mix_b,i);
            DrawRectangle(750+(600*i),530,60,50,col);
            snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
            DrawText(textout,750+(600*i),580,10,WHITE);


            col=ColorMixSub(mix_a,mix_b,i);
            DrawRectangle(750+(600*i),600,60,50,col);
            snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
            DrawText(textout,750+(600*i),650,10,WHITE);

            /*
            col=ColorInv(col);
            DrawRectangle(750+(600*i),600,60,50,col);
            snprintf(textout,sizeof textout,"%i %i %i",col.r,col.g,col.b);
            DrawText(textout,750+(600*i),650,10,WHITE);
            */

        }


        EndMode2D();
 //       DrawFPS(10,10);
        EndDrawing();
    }

    return 0;
}
