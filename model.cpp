#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "model.h"

using namespace std;
 
Model::Model(const string file) : vertex(), faces(), faces_tex(), faces_norm(), uv(), norm(), texture(), norm_texture(){
    
    ifstream fichier(file.c_str(), ios::in);  // on ouvre le fichier en lecture
 
    if(fichier){  // si l'ouverture a réussi       
        // instructions
        
        string ligne;
        string type;
        string info;
        string temp;
        while(getline(fichier, ligne)){
            
            type = ligne.substr(0,ligne.find(" "));
            std::istringstream iss(ligne.c_str());
            
            if(type == "v"){
                double x = atof(ligne.substr(ligne.find(" ",1),ligne.find(" ",4)).c_str());
                temp = ligne.substr(ligne.find(" ",5),ligne.size()).c_str();
                double y = atof(temp.substr(0,temp.find(" ",1)).c_str());
                double z = atof(temp.substr(temp.find(" ",1),temp.find(" ",2)).c_str());
                vertex.push_back(vec<3>(x,y,z));
                
            }else if (type == "f"){
                char trash;
                vector<int> face;
                vector<int> tex;
                vector<int> norm;
                int i, j, k;
                iss >> trash;
                while (iss >> i >> trash >> j >> trash >> k) {
                    i--; 
                    j--;
                    k--;
                    face.push_back(i);
                    tex.push_back(j);
                    norm.push_back(k);
                }
                faces.push_back(face);
                faces_tex.push_back(tex);
                faces_norm.push_back(norm);
                
            }else if (type == "vt"){
                double u = atof(ligne.substr(ligne.find(" ",1),ligne.find(" ",4)).c_str());
                temp = ligne.substr(ligne.find(" ",5),ligne.size()).c_str();
                double v = atof(temp.substr(0,temp.find(" ",1)).c_str());
                uv.push_back(vec<2>(u,v));
                
            }else if (type == "vn"){
                double x = atof(ligne.substr(ligne.find(" ",1),ligne.find(" ",4)).c_str());
                temp = ligne.substr(ligne.find(" ",5),ligne.size()).c_str();
                double y = atof(temp.substr(0,temp.find(" ",1)).c_str());
                double z = atof(temp.substr(temp.find(" ",1),temp.find(" ",2)).c_str());
                norm.push_back(vec<3>(x,y,z));
                
            }
            
        }
        
        string s = file.substr(0,file.find(".")) + "_diffuse.tga";
        texture.read_tga_file(s.c_str());
        texture.flip_vertically();
        
        s = file.substr(0,file.find(".")) + "_nm_tangent.tga";
        norm_texture.read_tga_file(s.c_str());
        norm_texture.flip_vertically();
        
        fichier.close();// on ferme le fichier
        
    }else{ // sinon
            cerr << "Impossible d'ouvrir le fichier !" << endl;
    }
    
}
 
Model::~Model(){
}

vec<3> Model::getVertex(int i){
    
    return vertex[i];
    
}


vector<int> Model::getFace(int i){
    
    return faces[i];
    
}

vector<int> Model::getFaceTex(int i){
    
    return faces_tex[i];
    
}

vector<int> Model::getFaceNorm(int i){
    
    return faces_norm[i];
    
}

vec<2> Model::getUV(int i){
    
    return uv[i];
    
}

vec<3> Model::getNorm(int i){
    
    return norm[i];
    
}


TGAImage Model::getTexture(){
    
    return texture;
    
}

int Model::getVertexSize(){
    
    return vertex.size();
    
}

int Model::getFacesSize(){
    
    return faces.size();
    
}
