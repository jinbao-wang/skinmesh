#ifndef SKINNEDMESH_H
#define SKINNEDMESH_H

#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions_3_3_Core>
#include <QFileInfo>
#include <QDir>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "utils.h"
#include <QMap>
#include <assert.h>
#include <QDebug>

class SkinnedMesh {
public:

    SkinnedMesh();
    ~SkinnedMesh();

    bool loadMesh(QString                    filename,
                  QOpenGLFunctions_3_3_Core *context);
    void render(QOpenGLFunctions_3_3_Core *context,
                GLenum                     mode);
    void BoneTransform(QVector<QMatrix4x4>& Transforms);
    void makeBoneTranformHeirarchySetAll(QMap<QString, QMatrix4x4>bodySkeletonMap);

    QMap<QString, QMatrix4x4>NodeTransformationTPoseTmp;
    QMap<QString, QMatrix4x4>NodeTransformationTPoseMatrix;

private:

    struct BoneInfo
    {
        QMatrix4x4 BoneOffset;
        QMatrix4x4 FinalTransformation;
    };

    struct VertexBoneData
    {
        uint  IDs[4];
        float Weights[4];

        VertexBoneData()
        {
            for (int i = 0; i < 4; i++)
            {
                IDs[i]     = 0;
                Weights[i] = 0;
            }
        }

        void AddBoneData(uint  BoneID,
                         float Weight);
    };

    struct MeshEntry {
        MeshEntry()
        {
            NumIndices    = 0;
            BaseVertex    = 0;
            BaseIndex     = 0;
            MaterialIndex = -1;
        }

        uint NumIndices;
        uint BaseVertex;
        uint BaseIndex;
        uint MaterialIndex;
    };
    QVector<MeshEntry>m_Entries;

    struct AnimationNode
    {
        AnimationNode()
        {
            trans     = QVector3D();
            animation = false;
        }

        QVector3D trans;
        bool      animation;
    };
    QVector<AnimationNode>m_AnimationNode;

    void clear();
    bool InitFromScene(const aiScene             *pScene,
                       QString                    Filename,
                       QOpenGLFunctions_3_3_Core *context);
    bool InitMaterials(const aiScene *pScene);
    void InitMesh(uint                     MeshIndex,
                  const aiMesh            *paiMesh,
                  QVector<QVector3D>     & Positions,
                  QVector<QVector3D>     & Normals,
                  QVector<QVector2D>     & TexCoords,
                  QVector<VertexBoneData>& Bones,
                  QVector<uint>          & Indices);
    void LoadBones(uint                     MeshIndex,
                   const aiMesh            *pMesh,
                   QVector<VertexBoneData>& Bones);

    void ReadNodeHeirarchy(aiNode           *pNode,
                           const QMatrix4x4& ParentTransform);
    void ReadNodeHeirarchy2(aiNode           *pNode,
                            const QMatrix4x4& ParentTransform);
    void ReadNodeHeirarchySetAll(const aiNode *pNode,
                                 QMap<QString, QMatrix4x4>bodySkeletonMap,
                                 const QMatrix4x4& ParentTransform);

    void InitNodeFinaltranform();

    QMap<QString, uint>m_BoneMapping;
    uint m_NumBones;
    QVector<BoneInfo>m_BoneInfo;
    QMatrix4x4 m_GlobalInverseTransform;

    QOpenGLVertexArrayObject m_vao;

    QOpenGLBuffer indexbuffer;
    QOpenGLBuffer positionbuffer;
    QOpenGLBuffer normalbuffer;
    QOpenGLBuffer texcoodbuffer;
    QOpenGLBuffer bonebuffer;

    QVector<QOpenGLTexture *>m_textures;

    QFileInfo m_fileInfo;

    Assimp::Importer importer;
    const aiScene *scene;
};

#endif // SKINNEDMESH_H
