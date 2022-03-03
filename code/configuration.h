#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <vector>

struct Configuration
{
    std::string AvatarName        = "astroBoy"; // astroBoy, girl
    std::string RootPath          = "D:/Qt_projects/SkinMesh";
    std::string AvatarPath        = RootPath + "/avatars/astroBoy/astroBoy.dae";
    std::string VertexPath        = RootPath + "/code/base.vert";
    std::string FragmentPath      = RootPath + "/code/base.frag";
    std::string FloorVertexPath   = RootPath + "/code/floor.vert";
    std::string FloorFragmentPath = RootPath + "/code/floor.frag";

    std::string FilePath          = RootPath + "/calc3d/saveDst";
    std::string ImagePath         = RootPath + "/calc3d/saveDst/img_src";
    std::string ImageOpenposePath = RootPath + "/calc3d/saveDst/img_op";
    std::string Keypoint2dPath    = RootPath +  "/calc3d/saveDst/keypoint";
    std::string Keypoint3dPath    = RootPath + "/calc3d/saveDst/keypoint3d";
};


#endif // CONFIGURATION_H
