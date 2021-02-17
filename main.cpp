////////////////////////////////////////
#include <vector>
#include <cmath>
#include <unistd.h>
#include <limits>

////////////////////////////////////////
#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

using namespace std;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0, 255,   0,   255);
const TGAColor blue = TGAColor(0, 0,   255,   255);
int width = 800;
int height = 800;
vec<3> light = {0,0,-1};
vec<3> cam = {0,0,3};
    
float *zbuffer = new float[width*height];


void line (vec<2> p1, vec<2> p2, TGAImage &image, TGAColor color){
    
    int dx = abs(p2.x - p1.x);
    int dy = abs(p2.y - p1.y);
    float erreur;
    int sx = 1;
    int sy = 1;
    int y = p1.y;
    int x = p1.x;

    if(p1.x > p2.x){
        sx = -1;
    }
    
    if(p1.y > p2.y){
        sy = -1;
    }

    //////// si le parcours de dx < dy alors on fait un parcours en y ////
    if(dx >= dy){
        erreur = -dx;
        while(x != p2.x){
            image.set(x, y, color); // on colorie 1 pixel
            ////// ici on regarde si l'on doit passer a la ligne superrieur ou pas ////
            erreur += dy*2;
            if(erreur > 0){
                y += sy;
                erreur -= dx*2; // je remet l'erreur a ligne supérieur
            }
            /////////
            x += sx;
        } 
    }else{
        erreur = -dy;
        while(y != p2.y){
            image.set(x, y, color); // on colorie 1 pixel
            ////// ici on regarde si l'on doit passer a la ligne superrieur ou pas////
            erreur += dx*2;
            if(erreur > 0){
                x += sx;
                erreur -= dy*2; // je remet l'erreur a ligne supérieur
            }
            /////////
            y += sy;
        }
    }
    //////////
}

mat<4,4> vec2Mat (vec<3> v){
    
    mat<4,4> m;
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    
    return m;
    
}

mat<4,4> projection (mat<4,4> m, vec<3> cam){
    
    mat<4,4> proj;
    proj[3][2] = -1.f/cam.z;
    proj[0][0] = 1;
    proj[1][1] = 1;
    proj[2][2] = 1;
    proj[3][3] = 1;

    return proj*m;
    
}

vec<3> mat2Vec (mat<4,4> mat){
    
    return vec<3>((mat[0][0]/mat[3][0]),(mat[1][0]/mat[3][0]),(mat[2][0]/mat[3][0]));

}

mat<4,4> viewport (int x, int y, int w, int h){
    
    mat<4,4> m;
    m[0][3] = x + w/2.f;
    m[1][3] = y + h/2.f;
    m[2][3] = 255/2.f;
    m[3][3] = 1;
    
    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = 255/2.f;
    
    return m;
}

mat<4,4> lookAt (vec<3>origine, vec<3> target){
    
    vec<3> up = vec<3>(.0,1.,.0);
    mat<4,4> m = mat<4,4>::identity();
    mat<4,4> m2 = mat<4,4>::identity();
    vec<3> dir = (target - origine).normalize();
    vec<3> right = cross(up,dir).normalize();
    vec<3> cUp = cross(dir,right);
    
    m2[0][3] = -origine.x;
    m2[1][3] = -origine.y;
    m2[2][3] = -origine.z;
    
    m[0][0] = right.x;  m[0][1] = right.y;   m[0][2] = right.z;
    m[1][0] = dir.x;    m[1][1] = dir.y;     m[1][2] = dir.z;
    m[2][0] = cUp.x;    m[2][1] = cUp.y;     m[2][2] = cUp.z;
    
    return m*m2;
}


vec<3> barycentric (vec<3> pts[3], vec<2> p){
    
    double w1 = ((pts[1].y - pts[2].y)*(p.x - pts[2].x) + (pts[2].x - pts[1].x)*(p.y - pts[2].y)) / ((pts[1].y - pts[2].y)*(pts[0].x - pts[2].x) + (pts[2].x - pts[1].x)*(pts[0].y - pts[2].y));
    
    double w2 = ((pts[2].y - pts[0].y)*(p.x - pts[2].x) + (pts[0].x - pts[2].x)*(p.y - pts[2].y)) / ((pts[1].y - pts[2].y)*(pts[0].x - pts[2].x) + (pts[2].x - pts[1].x)*(pts[0].y - pts[2].y));
    
    double w3 = 1 - w1 - w2;

    return vec<3>(w1, w2, w3);
}


void triangle (vec<3> *pts, vec<2> *uvs, vec<3> *intensites,float *zbuffer, TGAImage &image, TGAImage &texture, float intensite){
    
    vec<2> p1 = vec<2>(numeric_limits<double>::max(),0);
    vec<2> p2 = vec<2>(0,numeric_limits<double>::max());
    
    for(int i = 0; i < 3; i++){
        p1.x = min(p1.x, pts[i].x);
        p1.y = max(p1.y, pts[i].y);
        p2.x = max(p2.x, pts[i].x);
        p2.y = min(p2.y, pts[i].y);
    }
    
    vec<3> baryc;
    float z;
    for(int x = p1.x; x <= p2.x; x++){
        for(int y = p2.y; y <= p1.y; y++){
            baryc = barycentric(pts, vec<2>(x,y));
            vec<2> uv;
            vec<3> intensit;
            z = vec<3>(pts[0].z, pts[1].z, pts[2].z)*baryc ;
            if((baryc.x > 0.0 && baryc.x < 1.0) && (baryc.y > 0.0 && baryc.y < 1.0) && (baryc.z > 0.0 && baryc.z < 1.0)){
                if(zbuffer[int(x+y*width)]<z){
                    uv = baryc.x*uvs[0] + baryc.y*uvs[1] + baryc.z*uvs[2];
                    intensit = baryc.x*intensites[0] + baryc.y*intensites[1] + baryc.z*intensites[2];
                    zbuffer[int(x+y*width)] = z;
                    TGAColor coul = texture.get(uv.x*texture.get_width(),uv.y*texture.get_height());
                    image.set(x, y, TGAColor(coul.r*intensite, coul.g*intensite, coul.b*intensite, 255));
                }
            }
        }
    }
    
}


int main(int argc, char** argv) {
    
	TGAImage image(width, height, TGAImage::RGB);
    Model *model = new Model("obj/african_head.obj");
    TGAImage texture = model->getTexture();
    
    mat<4,4> view = viewport(width/8, height/8, width * 3/4, height * 3/4);
    mat<4,4> look = lookAt(vec<3>(1.,1.,3.),vec<3>(.0,.0,.0));
    
    model->getFacesSize();
    for(int i = 0; i < width*height;i++){
        zbuffer[i] = -numeric_limits<double>::max();
    }
    
    for(int i = 0; i < model->getFacesSize(); i++){
        vector<int> face = model->getFace(i);
        vector<int> face_tex = model->getFaceTex(i);
        vector<int> face_norm = model->getFaceNorm(i);
        vec<3> screen_coords[3];
        vec<3> world_coords[3];
        vec<3> light_coords[3];
        vec<2> uv[3];
        for(int j = 0; j < 3; j++){
            light_coords[j] = model->getNorm(face_norm[j]);
            uv[j] = model->getUV(face_tex[j]);
            world_coords[j] = model->getVertex(face[j]);
            screen_coords[j] = mat2Vec(view * projection(vec2Mat(world_coords[j]),cam)* look);
        }
        
        vec<3> n = cross(world_coords[2] - world_coords[0],world_coords[1] - world_coords[0]).normalize();
        float intensite = n * light;
        if(intensite > 0.0){
            triangle(screen_coords, uv, light_coords, zbuffer, image, texture, intensite);
        }
    }
    
    
    delete model;
        
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
    
}
