#include "voxel_filter.h"

point_cloud_vec filter_by_length(point_cloud_vec &clouds,float min_range,float max_range) {

    point_cloud_vec result;
    for(auto &it : clouds) {
        float range = POINT_CLOUD_DISTANCE(it);

        if(range > min_range && range < max_range) {
            result.push_back(it);
        }
    }

    return result;
}
namespace voxel_filter {

uint32_t VoxelFilter::get_cloud_to_key(point_cloud_t cloud) {

    float cloud_x = cloud.cloud_x / resolution_;
    float cloud_y = cloud.cloud_y / resolution_;

    uint32_t x = std::lroundf(cloud_x);
    uint32_t y = std::lroundf(cloud_y);

    return (uint32_t)((x << 16) | y);
}

point_cloud_vec VoxelFilter::voxel_filter(point_cloud_vec &clouds) {
    point_cloud_vec result;

    for(auto &it : clouds) {
        uint32_t key = get_cloud_to_key(it);
        auto tmp = could_set_.insert(key);
        if(tmp.second) {
            result.push_back(it);
        }
    }

    return result;
}

point_cloud_vec VoxelFilter::adaptive_voxel_filter(
        point_cloud_vec &clouds,float range_length,
        float max_range,float min_numbers) {

    //没有足够的点
    if(clouds.size() <= min_numbers) {
        return clouds;
    }

    point_cloud_vec result;
    result = VoxelFilter(range_length).voxel_filter(clouds);
    if(result.size() >= min_numbers) {
        return result;
    }

    for (float high_length = range_length;
         high_length > 1e-2f * range_length; high_length /= 2.f) {
        float low_length = high_length / 2.f;
        result = VoxelFilter(low_length).voxel_filter(clouds);
        if (result.size() >= min_numbers) {
            while ((high_length - low_length) / low_length > 1e-1f) {
                const float mid_length = (low_length + high_length) / 2.f;
                const point_cloud_vec candidate =
                        VoxelFilter(mid_length).voxel_filter(clouds);
                if (candidate.size() >= min_numbers) {
                    low_length = mid_length;
                    result = candidate;
                } else {
                    high_length = mid_length;
                }
            }
            return result;
        }
    }

    return result;
}
};