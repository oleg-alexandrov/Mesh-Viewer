#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include <fstream>
#include <string.h>
#include <map>

#include <GL/gl.h>

//#include <GL/freeglut.h>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>


#include <iostream>

struct Vector3D {
  float x, y, z;
  Vector3D(aiVector3D const& p) {
    // Temporary
    x = p.x;
    y = p.y;
    z = p.z;
  }

  float operator[](int i) {
     if (i == 0) 
       return x;
     if (i == 1)
       return y;
     if (i == 3) 
       return z;
  }
  
  Vector3D operator+(const Vector3D& b) {
    Vector3D out;
    out.x = this->x + b.x;
    out.y = this->y + b.y;
    out.z = this->z + b.z;
    return out;
  }

  Vector3D operator/(float factor) {
    Vector3D out;
    out.x = this->x / factor;
    out.y = this->y / factor;
    out.z = this->z / factor;
    return out;
  }
  
  Vector3D(double in_x = 0, double in_y = 0, double in_z = 0): x(in_x), y(in_y), z(in_z){}
};

// struct Matrix4x4 {
//     float a1, a2, a3, a4;
//     float b1, b2, b3, b4;
//     float c1, c2, c3, c4;
//     float d1, d2, d3, d4;

//   friend Matrix4x4 operator *(const Matrix4x4& m, const Matrix4x4& n) {
//     Matrix4x4 p;

// //     // This is all broken
// //   p.a1 = m.a1 * n.a1 + m.b1 * n.a2 + m.c1 * n.a3 + m.d1 * n.a4;
// //   m.a2 * n.a1 + m.b2 * n.a2 + m.c2 * n.a3 + m.d2 * n.a4;
// //   m.a3 * n.a1 + m.b3 * n.a2 + m.c3 * n.a3 + m.d3 * n.a4;
// //   m.a4 * n.a1 + m.b4 * n.a2 + m.c4 * n.a3 + m.d4 * n.a4;
// //   m.a1 * n.b1 + m.b1 * n.b2 + m.c1 * n.b3 + m.d1 * n.b4;
// //   m.a2 * n.b1 + m.b2 * n.b2 + m.c2 * n.b3 + m.d2 * n.b4;
// //   m.a3 * n.b1 + m.b3 * n.b2 + m.c3 * n.b3 + m.d3 * n.b4;
// //   m.a4 * n.b1 + m.b4 * n.b2 + m.c4 * n.b3 + m.d4 * n.b4;
// //   m.a1 * n.c1 + m.b1 * n.c2 + m.c1 * n.c3 + m.d1 * n.c4;
// //   m.a2 * n.c1 + m.b2 * n.c2 + m.c2 * n.c3 + m.d2 * n.c4;
// //   m.a3 * n.c1 + m.b3 * n.c2 + m.c3 * n.c3 + m.d3 * n.c4;
// //   m.a4 * n.c1 + m.b4 * n.c2 + m.c4 * n.c3 + m.d4 * n.c4;
// //   m.a1 * n.d1 + m.b1 * n.d2 + m.c1 * n.d3 + m.d1 * n.d4;
// //   m.a2 * n.d1 + m.b2 * n.d2 + m.c2 * n.d3 + m.d2 * n.d4;
// //   m.a3 * n.d1 + m.b3 * n.d2 + m.c3 * n.d3 + m.d3 * n.d4;
// //   m.a4 * n.d1 + m.b4 * n.d2 + m.c4 * n.d3 + m.d4 * n.d4;

//     return p;
//   }
// };

enum TextureState { TextureOn, TextureOff};
enum DisplayColor { None, Green, Red };
enum DisplayMode { Wireframe, Flat, Flatlines };
enum ShadingMode { FlatS, Gouraud, Phong };
enum TransformMode { Rotation, Translation };

class ModelManager {

public:
    ModelManager();
    ~ModelManager();

    bool importModel(const std::string& pFile);      //加载模型
    bool loadTextures();                        //加载texture
    void renderTheModel();                      //渲染模型入口

public:
    TextureState textureState;     //当前是否已经加载纹理
    DisplayMode displayMode;       //当前显示方式
    DisplayColor displayColor;     //当前颜色
    ShadingMode shadingMode;       //当前shading模式
    TransformMode transformMode;   //当前变换模式
    int subdivisionDepth;          //细分深度，0位不细分

    Vector3D scene_min, scene_max, scene_center;

private:

    //获取包围盒，得到x/y/z的最大/最小值：用于将模型放置在正中央
    void getBoundingBox();
    void getBoundingBoxRecursive(const struct aiNode* nd);

    //递归渲染
    void recursiveRender(const struct aiScene *sc, const struct aiNode* nd);
    void applyMaterial(const aiMaterial *mtl);    //给模型添加texture

    //处理面的函数入口
    void processFace(const struct aiMesh* mesh, GLenum face_mode, const struct aiFace* face);
    void subdivision(Vector3D vPos1, Vector3D vPos2, Vector3D vPos3,
                     Vector3D vNor1, Vector3D vNor2, Vector3D vNor3,
                     Vector3D vTexPos1, Vector3D vTexPos2, Vector3D vTexPos3,
                     int depth);

    //四边形渲染模式
    void renderPolygonFaceFlat(const struct aiMesh* mesh, GLenum face_mode, const struct aiFace* face);
    void renderPolygonFaceWireframe(const struct aiMesh* mesh, const struct aiFace* face);

    //三角形渲染模式
    void renderTriangleFaceFlat(Vector3D vPos1, Vector3D vPos2, Vector3D vPos3,
                                Vector3D vNor1, Vector3D vNor2, Vector3D vNor3,
                                Vector3D vTexPos1, Vector3D vTexPos2, Vector3D vTexPos3);
    void renderTriangleFaceWireframe(Vector3D vPos1, Vector3D vPos2, Vector3D vPos3);
    //三角形渲染phong模式
    void renderFaceFlatPhong(Vector3D vPos1, Vector3D vPos2, Vector3D vPos3,
                                Vector3D vNor1, Vector3D vNor2, Vector3D vNor3,
                                Vector3D vTexPos1, Vector3D vTexPos2, Vector3D vTexPos3);
    void renderSubFaceFlatPhong(Vector3D vPos1, Vector3D vPos2, Vector3D vPos3,
                                Vector3D vNor1, Vector3D vNor2, Vector3D vNor3,
                                Vector3D vTexPos1, Vector3D vTexPos2, Vector3D vTexPos3);

private:
    std::string modelPath = "";
    const aiScene* scene = NULL;
    GLuint scene_list = 0;

    std::map<std::string, GLuint*> textureIdMap;        // map image filenames to textureIds
  //GLuint* textureIds = NULL;	        	  // pointer to texture Array
};

#endif // MODELMANAGER_H
