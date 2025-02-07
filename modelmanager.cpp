#include "modelmanager.h"

//#include <IL/il.h>
//using namespace std;

const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 0.0f };
const GLfloat red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
const GLfloat green[] = { 0.0f, 1.0f, 1.0f, 1.0f };

// Vector3D matVecProd(aiMatrix4x4 const& m, Vector3D const& v) {

//   Vector3D w;
//   w.x = m.a1 * v.x + m.a2 * v.y + m.a3 * v.z + m.a4;
//   w.y = m.b1 * v.x + m.b2 * v.y + m.b3 * v.z + m.b4;
//   w.z = m.c1 * v.x + m.c2 * v.y + m.c3 * v.z + m.c4;
  
//   return w;
// }

void normalizeVector(Vector3D& v) {
    GLfloat d = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    v.x /= d;
    v.y /= d;
    v.z /= d;
}

ModelManager::ModelManager() {
    textureState = TextureOff;
    displayMode = Flat;
    displayColor = None;
    shadingMode = FlatS; // Gouraud;
    transformMode = Rotation;
    subdivisionDepth = 0;

}

ModelManager::~ModelManager() {
    textureIdMap.clear();

//     if (textureIds != NULL) {
//         delete[] textureIds;
//         textureIds = NULL;
//     }

    aiReleaseImport(scene); 
}

std::string getBasePath(const std::string& path) {
    size_t pos = path.find_last_of("\\/");
    return (std::string::npos == pos) ? "" : path.substr(0, pos + 1);
}

bool ModelManager::importModel(const std::string& pFile) {
    std::ifstream modelFilePath(pFile.c_str());
    if (modelFilePath.fail()) {
        std::cout << "Error::could not read model path file." << std::endl;
        return false;
    }

    int modelCount = 3;
    while (getline(modelFilePath, modelPath)) {
        if (modelPath.empty()) {
            std::cout << "Error::model path empty." << std::endl;
            return false;
        }
        else if (modelPath[0] == '#') {
            std::cout << "Next Line" << std::endl;
        }
        else {
            std::cout << "modelPath " << modelPath << std::endl;
            break;
        }

        modelCount--;
        if (modelCount <= 0)
            return false;
    }
    modelFilePath.close();

    scene = aiImportFile(modelPath.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

    if (!scene)
        return false;
    else {
        getBoundingBox();
        scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
        scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
        scene_center.z = (scene_min.z + scene_max.z) / 2.0f;

    }
    return true;
}

bool ModelManager::loadTextures() {
#if 0
    ILboolean success;

    // Before calling ilInit() version should be checked.
    if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION) {
        /// wrong DevIL version ///
        std::string err_msg = "Wrong DevIL version. Old devil.dll in system32/SysWow64?";
        std::cout << err_msg << std::endl;
        return false;
    }

    ilInit(); // Initialization of DevIL

    //对于每一种材质 Material：
    for (int m = 0; m < (int)scene->mNumMaterials; m++) {
        int texIndex = 0;
        aiReturn texFound = AI_SUCCESS;

        aiString path;	// filename

        //对当前材质，获取所有texture的图片id
        while (texFound == AI_SUCCESS) {
            texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
            textureIdMap[path.data] = NULL;    //把纹理图片路径加到map的key值，value（纹理指针）为空
            texIndex++;
        }
    }

    int numTextures = textureIdMap.size();    //texture数量

    // array with DevIL image IDs
    ILuint* imageIds = NULL;
    imageIds = new ILuint[numTextures];

    // generate DevIL Image IDs
    ilGenImages(numTextures, imageIds); // Generation of numTextures image names

    // create and fill array with GL texture ids
    if (textureIds != NULL) {
        delete[] textureIds;
    }
    textureIds = new GLuint[numTextures];
    glGenTextures(numTextures, textureIds);    //根据纹理参数返回n个纹理名称（不一定是连续的整数集合）

    // get iterator
    std::map<std::string, GLuint*>::iterator itr = textureIdMap.begin();

    std::string basepath = getBasePath(modelPath);

    //对于每个texture
    for (int i = 0; i < numTextures; i++) {
        //save IL image ID
        std::string filename = (*itr).first;  // get filename
        std::cout << "filename " << filename << std::endl;
        (*itr).second = &textureIds[i];	  //把每个纹理Id放进map的value
        itr++;								  // next texture

        ilBindImage(imageIds[i]);    //每个图像id绑定一张图
        std::string fileloc = basepath + filename;
        std::cout << "fileloc: " << fileloc << std::endl;
        success = ilLoadImage(fileloc.c_str()); 

        if (success) { // If no error occurred:

           // Convert every colour component into unsigned byte.If your image contains
           // alpha channel you can replace IL_RGB with IL_RGBA
            success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
            if (!success) {
                // Error occurred
                return false;
            }
            // Binding of texture name
            glBindTexture(GL_TEXTURE_2D, textureIds[i]);
            // redefine standard texture values
            // We will use linear interpolation for magnification filter
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // We will use linear interpolation for minifying filter
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            // Texture specification
            glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH),
                ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE,
                ilGetData());
            // we also want to be able to deal with odd texture dimensions
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
        }
    }
    // Because we have already copied image data into texture data  we can release memory used by image.
    ilDeleteImages(numTextures, imageIds);

    // Cleanup
    delete[] imageIds;
    imageIds = NULL;
#endif
    
    return true;
}

void ModelManager::getBoundingBox() {
//     aiMatrix4x4 trafo;
//     aiIdentityMatrix4(&trafo);

    scene_min.x = scene_min.y = scene_min.z =  1e10f;    //初始化为最大
    scene_max.x = scene_max.y = scene_max.z = -1e10f;    //初始化为最小
    getBoundingBoxRecursive(scene->mRootNode);
}
void ModelManager::getBoundingBoxRecursive(const struct aiNode* nd) {
//     aiMatrix4x4 prev;
//     prev = *trafo;
//     aiMultiplyMatrix4(trafo, &nd->mTransformation);

//     aiMatrix4x4 x = nd->mTransformation;
//     std::cout << "--xvals " << x.a1 << ' ' << x.a2 << ' ' << x.a3 << ' ' << x.a4 << std::endl;

  //std::cout << "--num meshes " << nd->mNumMeshes << std::endl;
  for (int n = 0; n < (int)nd->mNumMeshes; ++n) {
    const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
        
        for (int t = 0; t < (int)mesh->mNumVertices; ++t) {
          Vector3D tmp = mesh->mVertices[t];
          //tmp = matVecProd(*trafo, tmp);
          
          //std::cout << "---got " << tmp.x << ' ' << tmp.y << ' ' << tmp.z << std::endl;

          //std::cout << "--vertex " << tmp.x << ' ' << tmp.y << ' ' << tmp.z << std::endl;
          
          scene_min.x = std::min(scene_min.x, tmp.x);
          scene_min.y = std::min(scene_min.y, tmp.y);
          scene_min.z = std::min(scene_min.z, tmp.z);
          
          scene_max.x = std::max(scene_max.x, tmp.x);
          scene_max.y = std::max(scene_max.y, tmp.y);
          scene_max.z = std::max(scene_max.z, tmp.z);
        }
    }

    for (int n = 0; n < (int)nd->mNumChildren; ++n) {
        getBoundingBoxRecursive(nd->mChildren[n]);
    }
    //*trafo = prev;
}

void ModelManager::renderTheModel() {
    recursiveRender(scene, scene->mRootNode);
}
void ModelManager::recursiveRender(const struct aiScene *sc, const struct aiNode* nd) {
//     aiMatrix4x4 mTrans = nd->mTransformation;

//     //更新每个节点的变换方式
//     mTrans.Transpose();
//     glPushMatrix();
//     glMultMatrixf((float*)&mTrans);

  //对当前节点，遍历该节点的所有mMeshes(contains index to a mesh in scene.mMeshes[])

  //std::cout << "--recursive render " << std::endl;
    for (int m = 0; m < (int)nd->mNumMeshes; m++) {
        const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[m]];

        //添加texture
        //applyMaterial(sc->mMaterials[mesh->mMaterialIndex]);

        if (mesh->mNormals == NULL)
            glDisable(GL_LIGHTING);
        else
            glEnable(GL_LIGHTING);

        //对当前的mesh，遍历所有面face
        for (int f = 0; f < (int)mesh->mNumFaces; f++) {
            const struct aiFace* face = &(mesh->mFaces[f]);

            GLenum face_mode;

            switch (face->mNumIndices) {
            case 1:
                face_mode = GL_POINTS;
                break;
            case 2:
                face_mode = GL_LINES;
                break;
            case 3:
                face_mode = GL_TRIANGLES;
                break;
            default:
                face_mode = GL_POLYGON;
                break;
            }


            //glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
            //glEnable(GL_COLOR_MATERIAL);

            processFace(mesh, face_mode, face);
            //glDisable(GL_COLOR_MATERIAL);
//             std::cout << "--stop here!" << std::endl;
//             break;
        }
    }

    //递归绘制其他子节点
    for (int n = 0; n < (int)nd->mNumChildren; ++n) {
        recursiveRender(sc, nd->mChildren[n]);
    }

//     glPopMatrix();
}

void ModelManager::processFace(const struct aiMesh* mesh, GLenum face_mode, const struct aiFace* face) {
    if (face_mode == GL_POLYGON) {    //四边形
        if (displayMode == Flat || displayMode == Flatlines)
            renderPolygonFaceFlat(mesh, face_mode, face);

        if (displayMode == Wireframe || displayMode == Flatlines)
            renderPolygonFaceWireframe(mesh, face);
    }
    else if (face_mode == GL_TRIANGLES) {    //三角形
        Vector3D vPos[3];
        Vector3D vNor[3];
        Vector3D vTexPos[3];

        for (int i = 0; i < (int)face->mNumIndices; i++) {
            int index = face->mIndices[i];    //顶点索引index

            vPos[i] = mesh->mVertices[index];
            if (mesh->mNormals != NULL) {
              //if (mesh->HasTextureCoords(0)) {    //有纹理坐标时
              //      vTexPos[i] = mesh->mTextureCoords[0][index];
              //  }
              vNor[i] = mesh->mNormals[index];
            }
        }
        subdivision(vPos[0], vPos[1], vPos[2],
                    vNor[0], vNor[1], vNor[2],
                    vTexPos[0], vTexPos[1], vTexPos[2], subdivisionDepth);

    }
}

void ModelManager::subdivision(Vector3D vPos1, Vector3D vPos2, Vector3D vPos3,
                               Vector3D vNor1, Vector3D vNor2, Vector3D vNor3,
                               Vector3D vTexPos1, Vector3D vTexPos2, Vector3D vTexPos3,
                               int depth) {
    if (depth == 0) {
        if (displayMode == Flat || displayMode == Flatlines) {
            if (shadingMode == Phong) {
                renderFaceFlatPhong(vPos1, vPos2, vPos3, vNor1, vNor2, vNor3, vTexPos1, vTexPos2, vTexPos3);
            }
            else {    //如果是Flat shading或Gouraud shading，只需要渲染一个面
                renderTriangleFaceFlat(vPos1, vPos2, vPos3, vNor1, vNor2, vNor3, vTexPos1, vTexPos2, vTexPos3);
            }
        }

        if (displayMode == Wireframe || displayMode == Flatlines) {
            renderTriangleFaceWireframe(vPos1, vPos2, vPos3);
        }

        return;
    }

    Vector3D vPosMi[3];
    Vector3D vNorMi[3];
    Vector3D vTexPosMi[3];

    //取中间点
    vPosMi[0] = (vPos1 + vPos2) / 2.f;
    vPosMi[1] = (vPos2 + vPos3) / 2.f;
    vPosMi[2] = (vPos3 + vPos1) / 2.f;
    vNorMi[0] = (vNor1 + vNor2) / 2.f;
    vNorMi[1] = (vNor2 + vNor3) / 2.f;
    vNorMi[2] = (vNor3 + vNor1) / 2.f;
    normalizeVector(vNorMi[0]);
    normalizeVector(vNorMi[1]);
    normalizeVector(vNorMi[2]);
    vTexPosMi[0] = (vTexPos1 + vTexPos2) / 2.f;
    vTexPosMi[1] = (vTexPos2 + vTexPos3) / 2.f;
    vTexPosMi[2] = (vTexPos3 + vTexPos1) / 2.f;

    //顺序很重要
    subdivision(vPos1, vPosMi[0], vPosMi[2],
                vNor1, vNorMi[0], vNorMi[2],
                vTexPos1, vTexPosMi[0], vTexPosMi[2], depth - 1);
    subdivision(vPosMi[0], vPos2, vPosMi[1],
                vNorMi[0], vNor2, vNorMi[1],
                vTexPosMi[0], vTexPos2, vTexPosMi[1], depth - 1);
    subdivision(vPos3, vPosMi[2], vPosMi[1],
                vNor3, vNorMi[2], vNorMi[1],
                vTexPos3, vTexPosMi[2], vTexPosMi[1], depth - 1);
    subdivision(vPosMi[0], vPosMi[1], vPosMi[2],
                vNorMi[0], vNorMi[1], vNorMi[2],
                vTexPosMi[0], vTexPosMi[1], vTexPosMi[2], depth - 1);
}

void ModelManager::renderFaceFlatPhong(Vector3D vPos1, Vector3D vPos2, Vector3D vPos3,
                                Vector3D vNor1, Vector3D vNor2, Vector3D vNor3,
                                Vector3D vTexPos1, Vector3D vTexPos2, Vector3D vTexPos3) {
    Vector3D vPosMi[3];
    Vector3D vNorMi[3];
    Vector3D vTexPosMi[3];

    //取中间点
    vPosMi[0] = (vPos1 + vPos2) / 2.f;
    vPosMi[1] = (vPos2 + vPos3) / 2.f;
    vPosMi[2] = (vPos3 + vPos1) / 2.f;
    vNorMi[0] = (vNor1 + vNor2) / 2.f;
    vNorMi[1] = (vNor2 + vNor3) / 2.f;
    vNorMi[2] = (vNor3 + vNor1) / 2.f;
    normalizeVector(vNorMi[0]);
    normalizeVector(vNorMi[1]);
    normalizeVector(vNorMi[2]);
    vTexPosMi[0] = (vTexPos1 + vTexPos2) / 2.f;
    vTexPosMi[1] = (vTexPos2 + vTexPos3) / 2.f;
    vTexPosMi[2] = (vTexPos3 + vTexPos1) / 2.f;

    switch(displayColor) {
        case None: glColor4fv(white); break;
        case Red: glColor4fv(red); break;
        case Green: glColor4fv(green); break;
    }

    renderSubFaceFlatPhong(vPos1, vPosMi[0], vPosMi[2],
                            vNor1, vNorMi[0], vNorMi[2],
                            vTexPos1, vTexPosMi[0], vTexPosMi[2]);
    renderSubFaceFlatPhong(vPosMi[0], vPos2, vPosMi[1],
                            vNorMi[0], vNor2, vNorMi[1],
                            vTexPosMi[0], vTexPos2, vTexPosMi[1]);
    renderSubFaceFlatPhong(vPos3, vPosMi[2], vPosMi[1],
                            vNor3, vNorMi[2], vNorMi[1],
                            vTexPos3, vTexPosMi[2], vTexPosMi[1]);
    renderSubFaceFlatPhong(vPosMi[0], vPosMi[1], vPosMi[2],
                            vNorMi[0], vNorMi[1], vNorMi[2],
                            vTexPosMi[0], vTexPosMi[1], vTexPosMi[2]);
}

void ModelManager::renderSubFaceFlatPhong(Vector3D vPos1, Vector3D vPos2, Vector3D vPos3,
                            Vector3D vNor1, Vector3D vNor2, Vector3D vNor3,
                            Vector3D vTexPos1, Vector3D vTexPos2, Vector3D vTexPos3) {
    glBegin(GL_TRIANGLES);
    glTexCoord2f(vTexPos1.x, 1 - vTexPos1.y); // this is for texture
    glNormal3f(vNor1.x, vNor1.y, vNor1.z);  
    glVertex3f(vPos1.x, vPos1.y, vPos1.z);
    
    glTexCoord2f(vTexPos2.x, 1 - vTexPos2.y); // this is for texture
    glNormal3f(vNor2.x, vNor2.y, vNor2.z);   
    glVertex3f(vPos2.x, vPos2.y, vPos2.z);
    
    glTexCoord2f(vTexPos3.x, 1 - vTexPos3.y); // this is for texture
    glNormal3f(vNor3.x, vNor3.y, vNor3.z);   
    glVertex3f(vPos3.x, vPos3.y, vPos3.z);
    glEnd();
}

void ModelManager::renderTriangleFaceFlat(Vector3D vPos1, Vector3D vPos2, Vector3D vPos3,
                            Vector3D vNor1, Vector3D vNor2, Vector3D vNor3,
                            Vector3D vTexPos1, Vector3D vTexPos2, Vector3D vTexPos3) {
    switch(displayColor) {
        case None: glColor4fv(white); break;
        case Red: glColor4fv(red); break;
        case Green: glColor4fv(green); break;
    }

    glBegin(GL_TRIANGLES);

        if (shadingMode == FlatS) {
           // has just one normal per face
          
            glNormal3f(vNor1.x, vNor1.y, vNor1.z);

            glTexCoord2f(vTexPos1.x, 1 - vTexPos1.y);
            glVertex3f(vPos1.x, vPos1.y, vPos1.z);

            glTexCoord2f(vTexPos2.x, 1 - vTexPos2.y);
            glVertex3f(vPos2.x, vPos2.y, vPos2.z);

            glTexCoord2f(vTexPos3.x, 1 - vTexPos3.y);
            glVertex3f(vPos3.x, vPos3.y, vPos3.z);
        }

        else if (shadingMode == Gouraud) {
            glNormal3f(vNor1.x, vNor1.y, vNor1.z);
            glTexCoord2f(vTexPos1.x, 1 - vTexPos1.y);
            glVertex3f(vPos1.x, vPos1.y, vPos1.z);

            glNormal3f(vNor2.x, vNor2.y, vNor2.z);
            glTexCoord2f(vTexPos2.x, 1 - vTexPos2.y);
            glVertex3f(vPos2.x, vPos2.y, vPos2.z);

            glNormal3f(vNor3.x, vNor3.y, vNor3.z);
            glTexCoord2f(vTexPos3.x, 1 - vTexPos3.y);
            glVertex3f(vPos3.x, vPos3.y, vPos3.z);
        }

    glEnd();
}

void ModelManager::renderTriangleFaceWireframe(Vector3D vPos1, Vector3D vPos2, Vector3D vPos3) {
    glColor4fv(black);
    glLineWidth(1.0f);

    glBegin(GL_LINES);
        glVertex3f(vPos1.x, vPos1.y, vPos1.z);
        glVertex3f(vPos2.x, vPos2.y, vPos2.z);
    glEnd();

    glBegin(GL_LINES);
        glVertex3f(vPos3.x, vPos3.y, vPos3.z);
        glVertex3f(vPos2.x, vPos2.y, vPos2.z);
    glEnd();

    glBegin(GL_LINES);
        glVertex3f(vPos1.x, vPos1.y, vPos1.z);
        glVertex3f(vPos3.x, vPos3.y, vPos3.z);
    glEnd();
}

void ModelManager::renderPolygonFaceFlat(const struct aiMesh* mesh, GLenum face_mode, const struct aiFace* face) {
    int firstIndex, i;
    glBegin(face_mode);
    for (i = 0; i < (int)face->mNumIndices; i++) {
        int index = face->mIndices[i];    //顶点索引index

        if (i == 0)
            firstIndex = index;

        if (mesh->mNormals != NULL) {
          //if (mesh->HasTextureCoords(0)) {    //有纹理坐标时
          //  glTexCoord2f(mesh->mTextureCoords[0][index].x,
          //               1 - mesh->mTextureCoords[0][index].y); //mTextureCoords[channel][vertex]
          //}

            //设置shading
            if (shadingMode == FlatS)
                glNormal3fv(&mesh->mNormals[firstIndex].x);    //仅为第一个点的法向量
            else if (shadingMode == Gouraud || shadingMode == Phong)
                glNormal3fv(&mesh->mNormals[index].x);         //各个点自身的法向量

        }

        switch(displayColor) {
            case None: glColor4fv(white); break;
            case Red: glColor4fv(red); break;
            case Green: glColor4fv(green); break;
        }
        glVertex3fv(&(mesh->mVertices[index].x));
    }
    glEnd();
}

void ModelManager::renderPolygonFaceWireframe(const struct aiMesh* mesh, const struct aiFace* face) {
    //遍历所有顶点，每两个顶点之间连线
    for (int i = 0; i < (int)face->mNumIndices; i++) {
        int indexA = face->mIndices[i];
        int indexB = face->mIndices[i == (int)face->mNumIndices - 1 ? 0 : i + 1];

        glColor4fv(black);
        glLineWidth(1.0f);
        glBegin(GL_LINES);

        glVertex3fv(&(mesh->mVertices[indexA].x));
        glVertex3fv(&(mesh->mVertices[indexB].x));

        glEnd();
    }
}

void color4_to_float4(const aiColor4D *c, float f[4]) {
    f[0] = c->r;
    f[1] = c->g;
    f[2] = c->b;
    f[3] = c->a;
}

void set_float4(float f[4], float a, float b, float c, float d) {
    f[0] = a;
    f[1] = b;
    f[2] = c;
    f[3] = d;
}

void ModelManager::applyMaterial(const aiMaterial * mtl) {
  //  std::cout << "--apply material!" << std::endl;
#if 0
  float c[4];

    int ret1, ret2;
    aiColor4D diffuse;
    aiColor4D specular;
    aiColor4D ambient;
    aiColor4D emission;
    float shininess, strength;
    int two_sided;
    unsigned int max;

//     int texIndex = 0;
//     aiString texPath;	//contains filename of texture

//     if (AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath)) {
//         if (textureState == TextureOn) {    //已经加载纹理：绑定
//             //bind texture
//             unsigned int texId = *(textureIdMap[texPath.data]);
//             glBindTexture(GL_TEXTURE_2D, texId);
//         }
//         else {                    //不需要加载纹理：解绑、删除
//             if (!textureIdMap.empty()) {
//                 glDeleteTextures(1, textureIdMap[texPath.data]);
//             }
//         }
//     }

    set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
        color4_to_float4(&diffuse, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

    set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
        color4_to_float4(&specular, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

    set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
        color4_to_float4(&ambient, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

    set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
        color4_to_float4(&emission, c);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

    max = 1;
    ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
    if (ret1 == AI_SUCCESS) {
        max = 1;
        ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
        if (ret2 == AI_SUCCESS)
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
        else
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    }
    else {
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
        set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    max = 1;
    if ((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
        glDisable(GL_CULL_FACE);
    else
      glEnable(GL_CULL_FACE);
#endif
    
}
