#include <iostream>
#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <vector>

// Modulo multiplication group explorer.  Cf. Mathologer video: https://www.youtube.com/watch?v=6ZrO90AI0c8&t=1238s
// See also https://en.wikipedia.org/wiki/Lie_group and https://en.wikipedia.org/wiki/Abelian_group
// By Eric J. Jenislawski. https://www.github.com/ProfJski
// Raylib and RayGUI libraries by Ramon Santamaria. https://github.com/raysan5
//
// What does the program do?  You pick a modulus (Cmod), a multiplier (Cmult), and a starting number (Cstart).
// Numbers are graphed on the circumference of the circle, from zero to Cmod-1.
// In each iteration, the current value (starting with Cstart) is multiplied by the multiplier (Cmult).  The digital root of the product is found.  This result becomes the new current value.
// A line is drawn between the old and new values, and the program iterates until a value is repeated (thus making a loop on the graph).  The algorithm then ends.
// Different combinations of modulus and multiplier result in interesting graphs.  Different starting numbers may result in different loops.
// The "scan values" (Press G) function will display the number of unique digital roots for the first 400 moduli for a given multiplier, or the first 400 multipliers for a given modulus.  (Your choice, click the button twice to change.)
// Higher values on the "scan values" display will indicate where more elaborate patterns are found.  Mouse over a line on the graph to see its value.  Left-click to set the value on the slider.


using namespace std;

unsigned int DigRoot(unsigned int num, unsigned int Mmod) {
    unsigned int root=0;
    while (num>0) {
        root+=num%Mmod;
        num/=Mmod;
    }
    while (root>=Mmod) root=DigRoot(root,Mmod);
return root;
};

Vector2 DR2Vec(Vector2 Ccenter, float radius, unsigned int DR, unsigned int Mmod) {
    Vector2 ret=(Vector2){Ccenter.x+radius*cos(DR*2*PI/Mmod),Ccenter.y+radius*sin(DR*2*PI/Mmod)};
return ret;
}

Color colorize(Vector2 vstart, Vector2 vend, float radius) {
    float dist=Vector2Length(Vector2Subtract(vend,vstart))/radius;
return (Color){dist*255,0,(1-dist)*255,128};
}

Color colorize2(unsigned int i,unsigned int s) {
    float dist=(float)i/(float)s;
return (Color){dist*255,0,(1-dist)*255,128};
}

unsigned int GetMapPoints(Vector2 Ccenter, float Cradius,unsigned int Cmod, unsigned int Cmult, unsigned int Cstart, vector<Vector2>& points){
    unsigned int num=Cstart;
    unsigned int DR=DigRoot(num,Cmod);

    Vector2 prevPoint=DR2Vec(Ccenter,Cradius,DR,Cmod);
    Vector2 nextPoint={0,0};
    vector<unsigned int>DRSeen;
    DRSeen.reserve(Cmod);
    points.clear();

    DRSeen.push_back(DR);
    points.push_back(prevPoint);
    bool hasLooped = false;

    while (!hasLooped) {
        num=num*Cmult;
        DR=DigRoot(num,Cmod);
        if (num>DR) num=DR;
        nextPoint=DR2Vec(Ccenter,Cradius,DR,Cmod);
        //cout<<"Num="<<num<<" DR="<<DR<<" Size="<<DRSeen.size()<<endl;
        points.push_back(nextPoint);
        prevPoint=nextPoint;
        for (unsigned int i=0;i<DRSeen.size();i++) {
            if (DRSeen[i]==DR) hasLooped=true;
        }
        DRSeen.push_back(DR);
        //The final point in <vector>points and <vector>DR is the first point to repeat an earlier value, so a loop will be drawn on the graph
    }
return points.size()-1; //Returns number of unique digital roots
}

//The same function as above but only returns the number of unique digital roots and doesn't calculate vectors for graph lines.  It is used for "scan values" graph because it's faster.
unsigned int GetNUDRs(unsigned int Cmod, unsigned int Cmult, unsigned int Cstart) {
    unsigned int num=Cstart;
    unsigned int DR=DigRoot(num,Cmod);

    vector<unsigned int>DRSeen;
    DRSeen.push_back(DR);
    bool hasLooped = false;

    while (!hasLooped) {
        num=num*Cmult;
        DR=DigRoot(num,Cmod);
        if (num>DR) num=DR;
        for (unsigned int i=0;i<DRSeen.size();i++) {
            if (DRSeen[i]==DR) hasLooped=true;
        }
        DRSeen.push_back(DR);
    }
return DRSeen.size()-1; //Returns the number of unique digital roots
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1400;
    const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "Modulus Multiplication Explorer as seen in Mathologer 'Vortex Math' video");

    SetTargetFPS(30);

    //Upper limits for GUI slider bars.  Can be increased greatly.
    unsigned int CMOD_MAX=2000; //Max Modulus
    unsigned int CMULT_MAX=200; //Max multiplier
    unsigned int CSTART_MAX=200; //Max starting number
    //Initial values
    unsigned int Cmod = 100; //Initial modulus
    unsigned int Cmult = 2; //Initial multiplier
    unsigned int Cstart = 1; //Initial value of iteration sequence

    vector<Vector2> points;

    //GUI parameters
    char mytext[32];
    char gToggleGroupText[100]="One color;By length;By iteration";
    char gHelpText[400]="Keys:\n<O> and <P>: Decrease / Increase modulus by one\n<K> and <L>: Decrease / Increase multiplier by one\n<M> and <,>: Decrease / Increase starting number by one\n<Z> amd <X>: Camera zoom in / out\n<A><D><W><S>: Move camera\n \n<G> graphs unique digital roots by modulus or multiplier.\nMouse over the graph to find where high values (dense mappings) occur.\n \nPress <F1> to close this window";


    const int gGuiLeftEdge = 860;
    Rectangle gBackdrop = (Rectangle) {gGuiLeftEdge-20,0,screenWidth,screenHeight};
    Rectangle gRec_Cmod = (Rectangle) {gGuiLeftEdge,40,500,20};
    Rectangle gRec_Cmult = (Rectangle) {gGuiLeftEdge,140,500,20};
    Rectangle gRec_Cstart = (Rectangle) {gGuiLeftEdge,240,500,20};
    Vector2 gFPS_location = (Vector2) {gGuiLeftEdge, screenHeight-40};
    Rectangle gRec_ToggleGroup = (Rectangle) {gFPS_location.x+100,gFPS_location.y,140,20};

    Rectangle gRec_NUDRGraph = (Rectangle) {900,400,404,402};
    Rectangle gRec_graphMode = (Rectangle) {900,805,200,20};

    float Cradius=400;
    Vector2 Ccenter={gGuiLeftEdge/2,screenHeight/2};

    unsigned int colorizerMode=0;
    bool recalculate=true;
    bool scanValues=false;
    bool scanRecalc=true;
    vector<unsigned int> DRsPerM; //For scanning number of unique digital roots across a range of moduli for a given multiplier or vice versa
    unsigned int DRMaxVal=0;
    int graphMode=0;
    bool showHelp=false;

    //--------------------------------------------------------------------------------------

    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_O)) {Cmod=(Cmod<3)?2:--Cmod; recalculate=true;}
        if (IsKeyPressed(KEY_P)) {Cmod++; recalculate=true;}
        if (IsKeyPressed(KEY_K)) {Cmult=(Cmult<3)?2:--Cmult; recalculate=true;}
        if (IsKeyPressed(KEY_L)) {Cmult++; recalculate=true;}
        if (IsKeyPressed(KEY_M)) {Cstart=(Cstart<2)?1:--Cstart; recalculate=true;}
        if (IsKeyPressed(KEY_COMMA)) {Cstart++; recalculate=true;}
        if (IsKeyPressed(KEY_Z)) {Cradius+=10.0; recalculate=true;}
        if (IsKeyPressed(KEY_X)) {Cradius-=10.0; recalculate=true;}
        if (IsKeyPressed(KEY_A)) {Ccenter.x-=10.0; recalculate=true;}
        if (IsKeyPressed(KEY_D)) {Ccenter.x+=10.0; recalculate=true;}
        if (IsKeyPressed(KEY_W)) {Ccenter.y+=10.0; recalculate=true;}
        if (IsKeyPressed(KEY_S)) {Ccenter.y-=10.0; recalculate=true;}
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {recalculate=true; scanRecalc=true;}  //Catches if we changed parameters using the GUI in the last frame
        if (recalculate) scanRecalc=true;
        if (IsKeyPressed(KEY_G)) {
                if (scanValues) {
                    scanValues=false;
                    scanRecalc=false;
                }
                else {
                    scanValues=true;
                    scanRecalc=true;
                }
        }
        if (IsKeyPressed(KEY_F1)) showHelp=!showHelp;

        if (recalculate) {  //Only recompute points when we have changed values with mouse or keys.  Helps preserve responsiveness on busy maps.
            GetMapPoints(Ccenter,Cradius,Cmod,Cmult,Cstart,points);
            recalculate=false;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(BLACK);

        //Draw Background circle
        DrawCircleV(Ccenter,Cradius,Fade(DARKGRAY,0.2));

        //Draw all the lines, colorized as selected
        for (unsigned int i=0;i<points.size()-1;i++) {
            switch (colorizerMode) {
                case 0: DrawLineV(points[i],points[i+1],(Color){255,0,0,192}); break;
                case 1: DrawLineV(points[i],points[i+1],colorize(points[i],points[i+1],Cradius)); break;
                case 2: DrawLineV(points[i],points[i+1],colorize2(i,points.size())); break;
                default: DrawLineV(points[i],points[i+1],RED); break;
            }
        }

        //Draw the GUI
        DrawRectangleRec(gBackdrop,BLACK);

        DrawText("Modulus:",gRec_Cmod.x,gRec_Cmod.y-24,20,GREEN);
        Cmod=GuiScrollBar(gRec_Cmod,Cmod,2,CMOD_MAX);
        sprintf(mytext,"%u",Cmod);
        DrawText(mytext,gRec_Cmod.x,gRec_Cmod.y+gRec_Cmod.height+4,20,WHITE);

        DrawText("Multiplier:",gRec_Cmult.x,gRec_Cmult.y-24,20,GREEN);
        Cmult=GuiScrollBar(gRec_Cmult,Cmult,1,CMULT_MAX);
        sprintf(mytext,"%u",Cmult);
        DrawText(mytext,gRec_Cmult.x,gRec_Cmult.y+gRec_Cmult.height+4,20,WHITE);

        DrawText("Starting value:",gRec_Cstart.x,gRec_Cstart.y-24,20,GREEN);
        Cstart=GuiScrollBar(gRec_Cstart,Cstart,1,CSTART_MAX);
        sprintf(mytext,"%u",Cstart);
        DrawText(mytext,gRec_Cstart.x,gRec_Cstart.y+gRec_Cstart.height+4,20,WHITE);

        sprintf(mytext,"Unique digital roots: %u",points.size()-1);
        DrawText(mytext,gRec_Cstart.x,gRec_Cstart.y+gRec_Cstart.height+30,20,WHITE);

        DrawFPS(gFPS_location.x,gFPS_location.y);
        colorizerMode=GuiToggleGroup(gRec_ToggleGroup,gToggleGroupText,colorizerMode);

        if (scanValues) {
                graphMode=GuiToggleGroup(gRec_graphMode,"By Modulus;By Multiplier",graphMode);
                DrawText("Click twice to change",gRec_graphMode.x,gRec_graphMode.y+gRec_graphMode.height+4,10,GREEN);
                }

        //Draw the graph of unique digital roots
        if (scanValues) {
            if (scanRecalc) {
                scanRecalc=false;
                unsigned int NUDRs = 0;
                DRsPerM.clear();
                DRMaxVal=0;
                for (unsigned int i=2;i<402;i++) {
                    if (graphMode==0) NUDRs=GetNUDRs(i,Cmult,Cstart);
                    else NUDRs=GetNUDRs(Cmod,i,Cstart);
                    DRsPerM.push_back(NUDRs);
                    if (NUDRs>DRMaxVal) DRMaxVal=NUDRs;
                }
            }
            DrawRectangleLinesEx(gRec_NUDRGraph,1,WHITE);
            DrawLine(gRec_NUDRGraph.x,gRec_NUDRGraph.y+gRec_NUDRGraph.height,gRec_NUDRGraph.x+gRec_NUDRGraph.width,gRec_NUDRGraph.y,LIGHTGRAY);
            for (unsigned int i=0;i<400;i++) {
                if (graphMode==0) DrawLine(901+i,800,901+i,800-DRsPerM[i],(i+2==Cmod)?RED:YELLOW);
                else if (DRMaxVal<400) {
                    DrawLine(901+i,800,901+i,800-DRsPerM[i],YELLOW);
                }
                else {
                    DrawLine(901+i,800,901+i,800-((float)DRsPerM[i]*400.0/DRMaxVal),YELLOW); //If we are graphing by multiplier at high Moduli, graph height can be as high as Mod-2, so we need to scale output to fit the graph
                }
            }

            Vector2 MousePos=GetMousePosition();
            if (CheckCollisionPointRec(MousePos,gRec_NUDRGraph)) {
                unsigned int idx=(unsigned int)(MousePos.x-901);
                sprintf(mytext,"Mod: %i, #%i",idx+2,DRsPerM[idx]);
                DrawRectangle(MousePos.x+5,MousePos.y-30,70,14,Fade(GRAY,0.8));
                DrawText(mytext,MousePos.x+5,MousePos.y-30,10,WHITE);
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    if (graphMode==0) Cmod=idx+2;
                    else Cmult=idx+2;
                }
            }
        }  //End Scan values graph

        if (showHelp) {
            Rectangle gShow_Help = (Rectangle) {300,200,800,400};
            DrawRectangleRec(gShow_Help,Fade(BLACK,0.8));
            DrawRectangleLinesEx(gShow_Help,2,DARKGRAY);
            DrawText(gHelpText,gShow_Help.x+10,gShow_Help.y+10,20,WHITE);
        }
        else {
            DrawText("Press F1 for help",10,screenHeight-20,14,GREEN);
        }



        EndDrawing();

        //-----------------------------

    }

    // De-Initialization
    //--------------------------------------------------------------------------------------

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
