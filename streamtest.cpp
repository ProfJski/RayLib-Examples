#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <iostream>

using namespace std;

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

    float frequency=160.0;
    float samplingrate=11050.0;
    int SPU=4000; //Samples Per Update
    int rawsize=samplingrate*10; // Ten seconds worth at samplingrate
    int readpos=0; // reader

    short* raw = new short[rawsize];
    short* buff = new short[SPU];


    //fill raw with sound data
    for (uint i=0;i<rawsize;i++) {
        raw[i]=(short)16000*sin((float)i*2*PI*frequency/samplingrate);  // 2*PI*frequency/sample rate
        //cout<<"i="<<i<<" raw[i]="<<raw[i]<<endl;
    }

    //fill buff with first part of raw
    for (int i=0;i<SPU;i++) {
        buff[i]=raw[i];
    }
    readpos+=SPU;

    InitAudioDevice();
    if (IsAudioDeviceReady()) {
        cout<<"Audio ready"<<endl;
    }
    else {
        cout<<"Audio not initialized"<<endl;
    }


    AudioStream myaudio = InitAudioStream((uint)samplingrate,16,1);
    UpdateAudioStream(myaudio,buff,SPU);
    PlayAudioStream(myaudio);

    while (!WindowShouldClose()){
//Update

        if (IsAudioStreamProcessed(myaudio)) {
            cout<<"reloading - ";

            if (readpos>rawsize) { //this should never happen
                cout<<"Impossible"<<endl;
                readpos=0;
            }
            if (readpos+SPU>rawsize) { //we have to loop around on raw
                cout<<"looping";
                int tail=rawsize-readpos;
                cout<<" tail="<<tail;
                int j=0;
                for (int i=readpos;i<rawsize;i++) {
                    buff[j]=raw[i];
                    j++;
                }
                int firstpart = SPU-tail;
                cout<<" firstpart="<<firstpart;
                readpos=0;
                for (int i=readpos;i<firstpart;i++) {
                    buff[j]=raw[i];
                    j++;
                }
                readpos+=firstpart;
                cout<<" next readpos="<<readpos<<endl;   //" Buff 0="<<buff[0]<<" Buff 1="<<buff[1]<<" Buff end="<<buff[SPU-1]<<endl;
            }
            else if (readpos+SPU<=rawsize) {
                cout<<"regular. Readpos in="<<readpos;
                int j=0;
                for (int i=readpos;i<readpos+SPU;i++) {
                    buff[j]=raw[i];
                    j++;
                }
                readpos+=SPU;
                cout<<" readpos out="<<readpos<<endl;    //" Buff 0="<<buff[0]<<" Buff 1="<<buff[1]<<" Buff end="<<buff[SPU-1]<<endl;
            }
            else {
                cout<<"WTF"<<endl;
                readpos=0;
            }

            UpdateAudioStream(myaudio,buff,SPU);
            //PlayAudioStream(myaudio);

            }
            else {
                cout<<",";
            }

//Draw

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);

        float xratio=rawsize/1500;
        for (int i=0;i<rawsize;i++) {
                DrawPixel(i/xratio, 450+300*raw[i]/32000, RED);
        }
        DrawLine(readpos/xratio,0,readpos/xratio,800,GREEN);

        EndMode2D();
        DrawFPS(10,10);
        EndDrawing();
    }


    delete [] raw;
    delete [] buff;
    CloseAudioDevice();
    return 0;
}
