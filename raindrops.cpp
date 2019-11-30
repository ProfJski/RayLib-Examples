#include "raylib.h"
#include "raymath.h" //for vector math
#include <iostream>
#include <cstdio>
#include <list>
#include <random> //for random engine
#include <cmath> //for cube root
#include <algorithm> //for remove_if
#include <iterator> //for next method to get it2

using namespace std;

//Raindrops on a glass pane generator by Eric J. Jenislawski
//Fun with RayLib by Ramon Santamaria
//Warning: runs slowly once the drops build up.

default_random_engine generator;
uniform_real_distribution<float> dist1(-199,199); //one in from edge at -200,200 so drops don't overhang glass
uniform_real_distribution<float> dist2(1.0,1.3);

bool CollisionCheck(Vector3 A, Vector3 B, float RadA, float RadB){
    if (abs(A.x-B.x)>(RadA+RadB)){
        //cout<<"Abs:"<<abs(A.x-B.x)<<"vs RadA+B"<<(RadA+RadB)<<endl;
        return false;
    }
    else {
     return Vector3DotProduct(Vector3Subtract(B,A),Vector3Subtract(B,A))<(RadA+RadB)*(RadA+RadB);
    }

}

class raindrop{
    public:
        Vector3 position;
        float radius;
        Vector3 velocity;
        float mass;
        bool coll_flag=false;

        raindrop(){

            position=(Vector3){dist1(generator),dist1(generator),0.0};
            radius=dist2(generator);
            mass=4.0*PI*radius*radius*radius/6.0;
            velocity={0.0,(mass<10.0)?0.0:mass/-20.0,0.0};
            coll_flag=false;
        }

        void movedrop(){
            if (position.y<-200) {velocity=(Vector3){0.0,-10.0,0.0};}
            position=Vector3Add(position,velocity);
        }

};

bool Cull (const raindrop & p) {return (p.coll_flag==true);}

bool Offscreen (const raindrop & p) {return (p.position.y<-250);}

class dropcontroller{

    public:
        list<raindrop> drops;

    dropcontroller(){
        drops.push_back(raindrop());
    }

    void add_drop(){
        drops.push_back(raindrop());
        drops.push_back(raindrop());
        drops.push_back(raindrop());
    }

    void move_drops(){
        for (list<raindrop>::iterator it=drops.begin();it!=drops.end();++it){
            if (it->velocity.y!=0.0) {
                it->movedrop();
                coll_check(); //Check velocity before invoking, otherwise it crawls.
            }
        }

    }

    void coll_check(){
        Vector3 temp_pos={0.0,0.0,0.0};
        float temp_mass=0.0;
        //cout<<"size of list="<<drops.size()<<endl;
        for (list<raindrop>::iterator it=drops.begin();it!=prev(drops.end(),2);++it){
            for (list<raindrop>::iterator it2=next(it,1);it2!=prev(drops.end(),1);++it2) {
                //Could I optimize this to avoid square root in CheckCollSpheres by filtering first for it.x +/- radius since drops fall down?
                //if (CheckCollisionSpheres(it->position,it->radius,it2->position,it2->radius)){
                if (CollisionCheck(it->position,it2->position,it->radius,it2->radius)){
                    it->coll_flag=true;
                    it2->coll_flag=true;
                    temp_pos=Vector3Add(temp_pos,Vector3Multiply(it2->position,it2->mass)); //Add the mass and position from it2 here so it's only those which collide with it
                    temp_mass+=it2->mass;
                    //cout<<"Component: rad="<<it2->radius<<" mass="<<it2->mass<<" X="<<it2->position.x<<" Y="<<it2->position.y<<endl;
                    //cout<<"collision"<<endl;
                }
            }
            if (it->coll_flag){
                temp_pos=Vector3Add(temp_pos,Vector3Multiply(it->position,it->mass)); //Now add it's contribution since we are done checking it against other drops
                temp_mass+=it->mass;
                //cout<<"Component (it): rad="<<it->radius<<" mass="<<it->mass<<" X="<<it->position.x<<" Y="<<it->position.y<<endl;
                //modify it rather than create a new drop
                it->position=Vector3Divide(temp_pos,temp_mass);
                it->mass=temp_mass;
                it->radius=cbrt(6.0*it->mass/(4.0*PI));
                it->coll_flag=false;
                it->velocity=(Vector3){0.0,(it->mass<10.0)?0.0:it->mass/-20.0,0.0};
                //cout<<"New Drop Rad="<<it->radius<<" mass="<<it->mass<<" Pos x="<<it->position.x<<" Pos y="<<it->position.y<<" Pos Z="<<it->position.z<<endl;
            }
        drops.erase(remove_if(drops.begin(),drops.end(),Cull),drops.end());
        }
        return;

    }

    void cull_offscreen(){
        drops.erase(remove_if(drops.begin(),drops.end(),Offscreen),drops.end());
    }
};

int main()
{

//Init vars
    dropcontroller my_dc;

//Init raylib
    InitWindow(800, 800, "Drops");
    Camera camera = { 0 };
    camera.position = (Vector3){0.0, 0.0, -400.0};
    camera.target=(Vector3){0.0,0.0,0.0};
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.type = CAMERA_PERSPECTIVE;

    //Make the glass
    Mesh planemesh=GenMeshCube(400.0,400.0,1.0);
    Model glass=LoadModelFromMesh(planemesh);
    Vector3 glass_rotation_axis={1.0,0.0,0.0};
    Vector3 glass_position={0.0,0.0,0.0};
    float glass_rotation_angle = 0.0;
    Color glass_color={200,216,200,32};

    //Make the background
    Texture2D background=LoadTextureFromImage(GenImagePerlinNoise(500,500,1,1,1.0));
    Vector3 bg_pos={0.0,0.0,100.0};
    Model background_plane=LoadModelFromMesh(planemesh);
    SetMaterialTexture(&background_plane.materials[0],MAP_DIFFUSE,background);
    Color bg_color={200,200,255,255};

    //Make the raindrop model
    Image temp=GenImageGradientH(100,100,(Color){0,0,64,127},(Color){255,255,255,127});
    Image temp2=GenImageGradientH(100,100,(Color){255,255,255,128},(Color){0,0,64,128});
    Image myimage=GenImageColor(200,100,(Color){255,255,255,0});
    ImageDraw(&myimage,temp,(Rectangle){0,0,100,100},(Rectangle){0,0,100,100});
    ImageDraw(&myimage,temp2,(Rectangle){0,0,100,100},(Rectangle){100,0,200,100});
    Texture2D mytexture=LoadTextureFromImage(myimage);
    UnloadImage(myimage);
    UnloadImage(temp);
    UnloadImage(temp2);
    Mesh hemi=GenMeshSphere(1.0,12,12); // simpler sphere mesh to push fewer vertices
    Model droplet=LoadModelFromMesh(hemi);
    SetMaterialTexture(&droplet.materials[0],MAP_DIFFUSE,mytexture);

    SetTargetFPS(25);

    while (!WindowShouldClose()){

      //Update
      //Add a raindrop
      my_dc.add_drop();
      my_dc.move_drops();
      my_dc.coll_check();
      my_dc.cull_offscreen();

      //Draw

      BeginDrawing();
      ClearBackground(BLACK);

      BeginMode3D(camera);
      DrawModel(background_plane,bg_pos,2.0,bg_color);
      DrawModelEx(glass,glass_position,glass_rotation_axis,glass_rotation_angle,(Vector3){1.0,1.0,1.0},glass_color);
      //DrawGizmo(glass_position);
      for (list<raindrop>::iterator it=my_dc.drops.begin();it!=my_dc.drops.end();++it){
         DrawModel(droplet,it->position,it->radius,WHITE);
      }
      EndMode3D();

      DrawFPS(10,10);
      DrawText(to_string(my_dc.drops.size()).c_str(),20,20,20,GREEN);
      EndDrawing();
    }

    UnloadTexture(mytexture);
    UnloadTexture(background);
    UnloadModel(droplet);
    UnloadModel(glass);
    //UnloadMesh(&planemesh);
    CloseWindow();
    return 0;
}
