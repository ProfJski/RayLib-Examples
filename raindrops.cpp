#include "raylib.h"
#include "raymath.h" //for vector3 math
#include <iostream>
#include <cstdio>
#include <vector>
#include <random> //for random engine
#include <cmath> //for cube root
#include <algorithm> //for remove_if and sort
#include <iterator> //for next and prev methods for iterators

using namespace std;

//Raindrop simulator by Eric Jenislawski
//Raylib library by Ramon Santamaria


default_random_engine generator;
uniform_real_distribution<float> dist1(-248,248); //Drop placement. Decrease range from 250 by max drop radius (in dist2 below) so drops don't overhang glass
uniform_real_distribution<float> dist2(0.7,1.3);  //Radius range of new drops.  Greater than 1.68 starts drop in motion (for vel=mass/30)
// The maximum number of drops on screen depends on the radius range above.
// A smaller lower bound for dist2 results in more "fine mist" and thus many more particles.  Keeps 30 fps till around 11K drops.

class raindrop{
    public:
        Vector3 position;
        float radius;
        Vector3 velocity;
        float mass;
        int coll_flag=0;

        raindrop(){

            position=(Vector3){dist1(generator),dist1(generator),0.0};
            radius=dist2(generator);
            mass=4.0*PI*radius*radius*radius/6.0;  // Mass = volume of hemisphere of this radius
            velocity={0.0,(mass<10.0)?0.0:mass/-30.0,0.0}; // velocity is a function of mass in this sim.  Keep value of mass/-30 matched to similar line below.
            coll_flag=0;
        }

        void movedrop(){
            if (position.y<-250) {  //If drop reaches bottom of glass, "free-fall" velocity is 10
                velocity=(Vector3){0.0,-10.0,0.0};
            }
            position=Vector3Add(position,velocity);
        }

        //Overload Less-than operator for .sort() method in coll_check function of drop controller class
        //Sorts drops based on whether right-hand boundary of drop1 is less than left-hand boundary of drop2
        //So that drops which might collide are near each other in this list.  Reduces collision detection from O(n^2) to O(kn) time complexity.
        bool operator<(const raindrop & a) const {
            return (position.x+radius)<(a.position.x-a.radius);
        }
};

bool Cull (const raindrop & p) {return (p.coll_flag!=0);}

bool Offscreen (const raindrop & p) {return (p.position.y<-300);} // Remove drop from system once it falls 50px below glass pane and so is off screen

class dropcontroller{

    public:
        vector<raindrop> drops;

    dropcontroller(){
        drops.push_back(raindrop());
    }

    void add_drop(){
        drops.push_back(raindrop());  // Add more lines here to add more drops per frame
        drops.push_back(raindrop());
        drops.push_back(raindrop());
        drops.push_back(raindrop());
        drops.push_back(raindrop());
        drops.push_back(raindrop());
    }

    void add_this_drop(raindrop& thisdrop) {  //currently unused
        drops.push_back(thisdrop);
    }

    void move_drops(){
        //Most of this routine adds streaks of droplets behind larger, faster-moving drops
        vector<raindrop> streaks;
        for (vector<raindrop>::iterator it=drops.begin();it!=drops.end();++it){
            if (it->velocity.y!=0.0) {  //If velocity = 0, we exit immediately
                // Explanation of IF conditions below:
                // First condition=Is drop moving faster than 3 downward.
                // Second condition = Is its speed fast enough so that a new drop won't get immediately re-absorbed?
                // New drops are spawned at current drop location, then drop is advanced by vel pixels, so vel must be greater than radius of drop
                // Third condition = Don't bother merging if we are very close to bottom of glass.
                if ( (it->velocity.y<-3.0) && (abs(it->velocity.y)>(1.1*(it->radius))) && (it->position.y>-240.0) ) {
                    raindrop newdrop;
                    newdrop.position=it->position;
                    newdrop.radius=dist2(generator); // Randomizes drops in streak using same parameters for new drops
                    newdrop.mass=4.0*newdrop.radius*newdrop.radius*newdrop.radius*PI/6.0;
                    newdrop.velocity=(Vector3){0.0,0.0,0.0};
                    newdrop.coll_flag=0;
                    streaks.push_back(newdrop);
                    it->mass-=newdrop.mass; // We decrease the mass and radius of the drop to account for part of it breaking off
                    it->radius=cbrt(6.0*it->mass/(4.0*PI));
                    it->movedrop();
                }
                else {
                    it->movedrop();  //If you don't want streaks, this is the only line you need to move the drops
                }
            }
        }
        drops.insert(drops.end(),streaks.begin(),streaks.end());  //Add our new drops after we're done moving things
        streaks.clear();
    }

    void coll_check(){
        Vector3 temp_pos={0.0,0.0,0.0};
        vector<raindrop> coalesced;
        vector<raindrop> newdrops;
        raindrop newdrop;
        float temp_mass=0.0;
        float dx,dy,ra,rb;
        int counter=0;

        //Custom sort: See less-than operator overload in drop class
        sort(drops.begin(),drops.end());
        uint dsize=drops.size();

        int CFnum=0;

        //First pass = we just detect 2 or more drops that have collided
        for (uint i=0;i<dsize-2;i++) {
            for (uint j=i+1;j<dsize-1;j++) {
                //The magic of sorting: Because the list is sorted, once the right-hand side of drop[i] is less than the left-hand side of drop[j], we've detected possible collisions, since
                //the list is sorted by left & right x coordinate boundaries.  So then we break out of inner for-loop.  Not perfect, but very good for the speed improvement.
                if (drops[i].position.x+drops[i].radius<drops[j].position.x-drops[j].radius) {break;}
                dx=drops[j].position.x-drops[i].position.x;
                dy=drops[j].position.y-drops[i].position.y;
                dx*=dx;
                dy*=dy;
                ra=drops[i].radius;
                rb=drops[j].radius;
                if ((dx+dy)<((ra+rb)*(ra+rb))) { //If they have collided, assigned both particles a coll_flag group number.  Helps when more than two drops collide.
                    CFnum++;
                    drops[i].coll_flag=CFnum;
                    drops[j].coll_flag=CFnum;
                }
            }
        }

    //All collisions are now flagged; next, coalesce

        for (int c=1;c<=CFnum;c++){  //Scan through and gather each collision-flagged particle by its collflag group number
            coalesced.clear();
            for (auto it=drops.begin();it!=drops.end();it++) {
                if (it->coll_flag==c) {
                    coalesced.insert(coalesced.end(),*it);
                }
            }
            counter=0;
            temp_mass=0.0;
            temp_pos=(Vector3){0.0,0.0,0.0};
            for (vector<raindrop>::iterator it2=coalesced.begin();it2!=coalesced.end();it2++){ //Coalesce everything with the same collflag into one new drop
                temp_pos=Vector3Add(temp_pos,it2->position);
                counter++;
                temp_mass+=it2->mass;
            }
            temp_pos=Vector3Scale(temp_pos,1.0/((float)counter));  // Crude centroid, but doesn't currently take into account the varying masses of drops.
            newdrop.position=temp_pos;                             // Nonetheless, looks good because surface tension tends to pull real drops around when they coalesce.
            newdrop.mass=temp_mass;
            newdrop.radius=cbrt(6.0*temp_mass/(4.0*PI));
            newdrop.velocity=Vector3{0.0,((temp_mass<10.0)?0.0:temp_mass/-30.0),0.0};
            newdrop.coll_flag=0;
            newdrops.push_back(newdrop);
        }
        //Having coalesced all the drops and added their merged form to newdrops, do one final pass of the list to erase all coll-flagged items
        drops.erase(remove_if(drops.begin(),drops.end(),Cull),drops.end());

        //Now add the new drops to the list
        drops.insert(drops.end(),newdrops.begin(),newdrops.end());
        newdrops.clear();
        //...and we are done.  Whew.
    }

    void cull_offscreen(){
        drops.erase(remove_if(drops.begin(),drops.end(),Offscreen),drops.end());
    }

};

// Used in render loop to make faster-moving drops more oblong.  Can be disabled below.
Vector3 shaper(raindrop r) {
    Vector3 scale=(Vector3){1.0,1.0,1.0};
    if (r.velocity.y>-0.1) {return Vector3Scale(scale,r.radius);}
    if (r.velocity.y<-9.9) {return Vector3Scale(scale,r.radius);} //If vel<-10, drop is probably in free-fall, so resume spherical shape.
    scale.y=1.0+(-0.08*r.velocity.y); // Change -0.08 factor to your taste.
    scale=Vector3Scale(scale,r.radius);
    return scale;
}

int main()
{

//Init vars

    dropcontroller my_dc;


//Init raylib
    InitWindow(1000, 1000, "Raindrops");
    SetWindowPosition(500,50);

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){0.0, 0.0, -500.0};
    camera.target=(Vector3){0.0,0.0,0.0};
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.type = CAMERA_PERSPECTIVE;

    //Make the glass
    Mesh planemesh=GenMeshCube(500.0,500.0,1.0);
    Model glass=LoadModelFromMesh(planemesh);
    Vector3 glass_rotation_axis={1.0,0.0,0.0};
    Vector3 glass_position={0.0,0.0,0.0};
    float glass_rotation_angle = 0.0;
    Color glass_color={200,216,200,32};

    //Make the background
    //Texture2D background=LoadTextureFromImage(GenImageChecked(500,500,10,10,BLUE,RED));
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
    //droplet.materials[0].ior=1.4;

    SetTargetFPS(30);

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
    for (vector<raindrop>::iterator it=my_dc.drops.begin();it!=my_dc.drops.end();++it){
       //DrawModel(droplet,it->position,it->radius,WHITE);  //If you don't want drop shaping as a function of speed, use this line and comment out the one below.  May help performance a little.
       DrawModelEx(droplet,it->position,(Vector3){1.0,0.0,0.0},0.0,shaper(*it),WHITE);
    }
    EndMode3D();

    DrawFPS(10,10);
    DrawText(to_string(my_dc.drops.size()).c_str(),20,30,20,GREEN); //Displays number of drops in system on screen
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
