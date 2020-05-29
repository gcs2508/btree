#ifndef _VOXEL_FILTER_H__
#define _VOXEL_FILTER_H__

#include <vector>
#include <hash_set>
#include <iostream>
#include <bitset>

typedef struct point_cloud {
    float cloud_x;              //点云 x 方向偏移量
    float cloud_y;              //点云 y 方向偏移量
    float cloud_angle;          //点云 angle 方向
}point_cloud_t;

#define POINT_CLOUD_DISTANCE(cloud)             (sqrtf(powf(cloud.cloud_x,2) + powf(cloud.cloud_y,2)))

using point_cloud_vec = std::vector<point_cloud_t>;
point_cloud_vec filter_by_length(point_cloud_vec &clouds,float min_range = 0.10f,float max_range = 8.0f);

namespace voxel_filter {
class VoxelFilter {

public:
    explicit VoxelFilter(float size) : resolution_(size) {}

    VoxelFilter(const VoxelFilter&) = delete;
    VoxelFilter& operator=(const VoxelFilter&) = delete;

    point_cloud_vec voxel_filter(point_cloud_vec &clouds);
    point_cloud_vec adaptive_voxel_filter(point_cloud_vec &clouds,float range_length,float max_range,float min_numbers);

private:
    uint32_t get_cloud_to_key(point_cloud_t cloud);

private:
    float resolution_;              //体素的大小
    __gnu_cxx::hash_set<uint32_t> could_set_;
};

};

#endif //_VOXEL_FILTER_H__
