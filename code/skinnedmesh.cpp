#include "skinnedmesh.h"
#include <Eigen/Dense>
#include <iostream>
#include <QDebug>

#define POSITION_LOCATION    0
#define TEX_COORD_LOCATION   1
#define NORMAL_LOCATION      2
#define BONE_ID_LOCATION     3
#define BONE_WEIGHT_LOCATION 4

void SkinnedMesh::VertexBoneData::AddBoneData(uint BoneID, float Weight)
{
    for (uint i = 0; i < 4; i++) {
        if (Weights[i] == 0.0) {
            IDs[i]     = BoneID;
            Weights[i] = Weight;
            return;
        }
    }

    // should never get here - more bones than we have space for
    // assert(0);
}

SkinnedMesh::SkinnedMesh() :
    indexbuffer(QOpenGLBuffer::IndexBuffer),
    positionbuffer(QOpenGLBuffer::VertexBuffer),
    normalbuffer(QOpenGLBuffer::VertexBuffer),
    texcoodbuffer(QOpenGLBuffer::VertexBuffer),
    bonebuffer(QOpenGLBuffer::VertexBuffer)
{
    m_NumBones = 0;
    scene      = NULL;
}

SkinnedMesh::~SkinnedMesh()
{
    clear();
}

void SkinnedMesh::clear()
{
    indexbuffer.destroy();
    positionbuffer.destroy();
    normalbuffer.destroy();
    texcoodbuffer.destroy();
    bonebuffer.destroy();

    for (int i = 0; i < m_textures.size(); i++) {
        delete m_textures[i];
    }
}

bool SkinnedMesh::loadMesh(QString filename, QOpenGLFunctions_3_3_Core *context)
{
    clear();
    m_fileInfo = filename;
    m_vao.create();
    m_vao.bind();

    indexbuffer.create();
    positionbuffer.create();
    normalbuffer.create();
    texcoodbuffer.create();
    bonebuffer.create();

    bool Ret = false;
    scene = importer.ReadFile(filename.toStdString(),
                              aiProcess_Triangulate  |
                              aiProcess_GenNormals |
                              aiProcess_FlipUVs);

    if (scene) {
        QMatrix4x4 m_GlobalTransform = getQmatrix4x4(scene->mRootNode->mTransformation);
        m_GlobalInverseTransform = m_GlobalTransform.inverted();

        // qDebug() << m_GlobalInverseTransform;
        Ret = InitFromScene(scene, filename, context);
    } else {
        qDebug() << filename << " " << importer.GetErrorString();
    }

    m_vao.release();
    return Ret;
}

bool SkinnedMesh::InitFromScene(const aiScene *pScene, QString Filename, QOpenGLFunctions_3_3_Core *context)
{
    m_Entries.resize(pScene->mNumMeshes);
    m_textures.resize(pScene->mNumMaterials);

    QVector<QVector3D> Positions;
    QVector<QVector3D> Normals;
    QVector<QVector2D> TexCoords;
    QVector<VertexBoneData> Bones;
    QVector<uint> Indices;

    uint NumVertices = 0;
    uint NumIndices  = 0;

    // Count the number of vertices and indices
    for (int i = 0; i < m_Entries.size(); i++) {
        m_Entries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
        m_Entries[i].NumIndices    = pScene->mMeshes[i]->mNumFaces * 3;
        m_Entries[i].BaseVertex    = NumVertices;
        m_Entries[i].BaseIndex     = NumIndices;

        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices  += m_Entries[i].NumIndices;
    }

    // Reserve space in the vectors for the vertex attributes and indices
    Positions.reserve(NumVertices);
    Normals.reserve(NumVertices);
    TexCoords.reserve(NumVertices);
    Bones.resize(NumVertices);
    Indices.reserve(NumIndices);

    // Initialize the meshes in the scene one by one
    for (int j = 0; j < m_Entries.size(); j++) {
        const aiMesh *paiMesh = pScene->mMeshes[j];
        InitMesh(j, paiMesh, Positions, Normals, TexCoords, Bones, Indices);
    }

    if (!InitMaterials(pScene)) {
        return false;
    }

    InitNodeFinaltranform();

    positionbuffer.bind();
    positionbuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    positionbuffer.allocate(Positions.data(), sizeof(QVector3D) * Positions.size());
    context->glEnableVertexAttribArray(POSITION_LOCATION);
    context->glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    texcoodbuffer.bind();
    texcoodbuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    texcoodbuffer.allocate(TexCoords.data(), sizeof(QVector2D) * TexCoords.size());
    context->glEnableVertexAttribArray(TEX_COORD_LOCATION);
    context->glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    normalbuffer.bind();
    normalbuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    normalbuffer.allocate(Normals.data(), sizeof(QVector3D) * Normals.size());
    context->glEnableVertexAttribArray(NORMAL_LOCATION);
    context->glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    bonebuffer.bind();
    bonebuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bonebuffer.allocate(Bones.data(), sizeof(Bones[0]) * Bones.size());
    context->glEnableVertexAttribArray(BONE_ID_LOCATION);
    context->glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid *)0);

    // context->glVertexAttribPointer(BONE_ID_LOCATION,4,GL_INT,GL_FALSE,sizeof(VertexBoneData),(const
    // GLvoid*)0);
    context->glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
    context->glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid *)16);

    indexbuffer.bind();
    indexbuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    indexbuffer.allocate(Indices.data(), sizeof(uint) * Indices.size());

    // qDebug()<<context->glGetError();
    return true;
}

void SkinnedMesh::InitMesh(uint                     MeshIndex,
                           const aiMesh            *paiMesh,
                           QVector<QVector3D>     & Positions,
                           QVector<QVector3D>     & Normals,
                           QVector<QVector2D>     & TexCoords,
                           QVector<VertexBoneData>& Bones,
                           QVector<uint>          & Indices)
{
    // Populate the vertex attribute vectors
    for (uint i = 0; i < paiMesh->mNumVertices; i++) {
        const aiVector3D *pPos    = &(paiMesh->mVertices[i]);
        const aiVector3D *pNormal = &(paiMesh->mNormals[i]);

        Positions.push_back(QVector3D(pPos->x, pPos->y, pPos->z));
        Normals.push_back(QVector3D(pNormal->x, pNormal->y, pNormal->z));

        if (paiMesh->mTextureCoords[0]) {
            aiVector3D tex = paiMesh->mTextureCoords[0][i];
            TexCoords.push_back(QVector2D(tex.x, tex.y));
        }
        else
        {
            TexCoords.push_back(QVector2D(0.0f, 0.0f));
        }
    }

    LoadBones(MeshIndex, paiMesh, Bones);

    // Populate the index buffer
    for (uint i = 0; i < paiMesh->mNumFaces; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        Indices.push_back(Face.mIndices[0]);
        Indices.push_back(Face.mIndices[1]);
        Indices.push_back(Face.mIndices[2]);
    }
}

void SkinnedMesh::LoadBones(uint MeshIndex, const aiMesh *pMesh, QVector<VertexBoneData>& Bones)
{
    for (uint i = 0; i < pMesh->mNumBones; i++) {
        uint BoneIndex = 0;
        QString BoneName(pMesh->mBones[i]->mName.data);

        if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
            // Allocate an index for a new bone
            BoneIndex = m_NumBones;
            m_NumBones++;
            BoneInfo bi;
            m_BoneInfo.push_back(bi);
            m_BoneInfo[BoneIndex].BoneOffset = getQmatrix4x4(pMesh->mBones[i]->mOffsetMatrix);
            m_BoneMapping[BoneName]          = BoneIndex;
        }
        else {
            BoneIndex = m_BoneMapping[BoneName];
        }

        for (uint j = 0; j < pMesh->mBones[i]->mNumWeights; j++) {
            uint  VertexID = m_Entries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
            float Weight   = pMesh->mBones[i]->mWeights[j].mWeight;
            Bones[VertexID].AddBoneData(BoneIndex, Weight);
        }
    }
}

bool SkinnedMesh::InitMaterials(const aiScene *pScene)
{
    bool Ret = true;
    QDir dir = m_fileInfo.dir();

    for (uint i = 0; i < pScene->mNumMaterials; i++)
    {
        const aiMaterial *material = pScene->mMaterials[i];

        if (material->GetTextureCount(aiTextureType_DIFFUSE))
        {
            aiString Filename;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &Filename);
            QString path = dir.absoluteFilePath(Filename.C_Str());
            path.replace("\\", "/");
            QImage tex(path);

            if (tex.isNull()) {
                qDebug() << "Texture not found : " << path;
                continue;
            }

            QOpenGLTexture *texture = new QOpenGLTexture(tex, QOpenGLTexture::DontGenerateMipMaps);
            m_textures[i] = texture;
        }
    }
    return Ret;
}

void SkinnedMesh::render(QOpenGLFunctions_3_3_Core *context, GLenum mode)
{
    m_vao.bind();

    for (uint i = 0; i < m_Entries.size(); i++) {
        const uint MaterialIndex = m_Entries[i].MaterialIndex;

        assert(MaterialIndex < m_textures.size());

        if (m_textures[MaterialIndex]) {
            m_textures[MaterialIndex]->bind();
        }

        // context->glDrawArrays(mode,m_Entries[i].BaseIndex,m_Entries[i].NumIndices);
        context->glDrawElementsBaseVertex(mode,
                                          m_Entries[i].NumIndices,
                                          GL_UNSIGNED_INT,
                                          (void *)(sizeof(uint) * m_Entries[i].BaseIndex),
                                          m_Entries[i].BaseVertex);
    }

    // Make sure the VAO is not changed from the outside
    m_vao.release();
}

void SkinnedMesh::BoneTransform(QVector<QMatrix4x4>& Transforms)
{
    Transforms.resize(m_NumBones);

    for (uint i = 0; i < m_NumBones; i++) {
        Transforms[i] = m_BoneInfo[i].FinalTransformation;
    }
}

void SkinnedMesh::InitNodeFinaltranform()
{
    QMatrix4x4 Identity;

    Identity.setToIdentity();
    ReadNodeHeirarchy(scene->mRootNode, Identity);
    ReadNodeHeirarchy2(scene->mRootNode, Identity);
}

// make node transformation into T pose with T pose mesh
void SkinnedMesh::ReadNodeHeirarchy(aiNode *pNode, const QMatrix4x4& ParentTransform)
{
    QString NodeName(pNode->mName.data);

    // chack the bone rigging name if right
    // qDebug() << NodeName;
    QMatrix4x4 NodeTranformation = getQmatrix4x4(pNode->mTransformation);

    // qDebug()<<NodeTranformation;
    QMatrix4x4 GlobalTransformation = ParentTransform * NodeTranformation;

    if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
        uint BoneIndex = m_BoneMapping[NodeName];

        m_BoneInfo[BoneIndex].FinalTransformation.setToIdentity();
        QMatrix4x4 Mt = ParentTransform.inverted() * m_GlobalInverseTransform.inverted() * m_BoneInfo[BoneIndex].BoneOffset.inverted();

        pNode->mTransformation.a1 = Mt(0, 0); pNode->mTransformation.a2 = Mt(0, 1); pNode->mTransformation.a3 = Mt(0, 2); pNode->mTransformation.a4 = Mt(0, 3);
        pNode->mTransformation.b1 = Mt(1, 0); pNode->mTransformation.b2 = Mt(1, 1); pNode->mTransformation.b3 = Mt(1, 2); pNode->mTransformation.b4 = Mt(1, 3);
        pNode->mTransformation.c1 = Mt(2, 0); pNode->mTransformation.c2 = Mt(2, 1); pNode->mTransformation.c3 = Mt(2, 2); pNode->mTransformation.c4 = Mt(2, 3);
        pNode->mTransformation.d1 = Mt(3, 0); pNode->mTransformation.d2 = Mt(3, 1); pNode->mTransformation.d3 = Mt(3, 2); pNode->mTransformation.d4 = Mt(3, 3);

        GlobalTransformation = ParentTransform * Mt;
    }

    // qDebug()<<GlobalTransformation;

    for (uint i = 0; i < pNode->mNumChildren; i++) {
        ReadNodeHeirarchy(pNode->mChildren[i], GlobalTransformation);
    }
}

// get node coordinate axis transformation matrix in T pose
void SkinnedMesh::ReadNodeHeirarchy2(aiNode *pNode, const QMatrix4x4& ParentTransform)
{
    QString NodeName(pNode->mName.data);

    QMatrix4x4 NodeTransformation = getQmatrix4x4(pNode->mTransformation);

    NodeTransformationTPoseTmp.insert(NodeName, NodeTransformation);

    // qDebug()<<NodeName;
    // qDebug()<<NodeTransformation;

    float scale = 1.;

    if (NodeTransformationTPoseTmp.find("VisualSceneNode") != NodeTransformationTPoseTmp.end()) {
        scale = 1. / NodeTransformationTPoseTmp["VisualSceneNode"](0, 0);
    }

    QMatrix4x4 GlobalTransformation = ParentTransform * NodeTransformation;
    NodeTransformationTPoseMatrix.insert(NodeName, GlobalTransformation * scale);

    // qDebug()<<GlobalTransformation;


    if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
        uint BoneIndex = m_BoneMapping[NodeName];
        m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
    }

    for (uint i = 0; i < pNode->mNumChildren; i++) {
        ReadNodeHeirarchy2(pNode->mChildren[i], GlobalTransformation);
    }
}

// ----------------------------------------------------------------------------------------
void SkinnedMesh::makeBoneTranformHeirarchySetAll(QMap<QString, QMatrix4x4>bodySkeletonMap)
{
    QMatrix4x4 ParentTransform;

    ParentTransform.setToIdentity();

    ReadNodeHeirarchySetAll(scene->mRootNode, bodySkeletonMap, ParentTransform);
}

void SkinnedMesh::ReadNodeHeirarchySetAll(const aiNode *pNode, QMap<QString, QMatrix4x4>bodySkeletonMap, const QMatrix4x4& ParentTransform)
{
    QString NodeName(pNode->mName.data);
    QMatrix4x4 NodeTransformation = getQmatrix4x4(pNode->mTransformation);

    QMatrix4x4 BoneMatrix;

    BoneMatrix.setToIdentity();

    if (bodySkeletonMap.find(NodeName) != bodySkeletonMap.end()) {
        BoneMatrix         = bodySkeletonMap[NodeName];
        NodeTransformation = NodeTransformation * BoneMatrix;
    }

    QMatrix4x4 GlobalTransformation = ParentTransform * NodeTransformation;

    if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
        uint BoneIndex = m_BoneMapping[NodeName];
        m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
    }

    for (uint i = 0; i < pNode->mNumChildren; i++) {
        ReadNodeHeirarchySetAll(pNode->mChildren[i], bodySkeletonMap, GlobalTransformation);
    }
}
