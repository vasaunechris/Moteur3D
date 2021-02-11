#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

using namespace std;

class Model {
private :
    
    std::vector<vec<3>> vertex;
    std::vector<std::vector<int>>faces;
    std::vector<std::vector<int>>faces_tex;
    std::vector<vec<2>> uv;
    TGAImage texture;
        
public:
    
    Model(const string file);
    ~Model();
    vec<3> getVertex(int i);
    std::vector<int> getFace(int i);
    std::vector<int> getFaceTex(int i);
    vec<2> getUV(int i);
    TGAImage getTexture();
    int getVertexSize();
    int getFacesSize();
    
};


#endif //__MODEL_H__
