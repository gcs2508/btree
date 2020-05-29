#include "BTreeTest.h"
#include "BasePid.h"
#include <stdio.h>
#include <cmath>
#include <zconf.h>
#include <voxel_filter.h>

#include "thread_pool/ThreadPool.h"
#include "data_observer/data_observer.h"

using namespace data_observer;
int main() {

//    DataObserver observe;
//    char nmpa[] = {'a','b','c'};
//    observe.RegisterPublish("a",3);
//    observe.RegisterPublish("b",3);
//    observe.RegisterPublish("c",3);
//    observe.RegisterPublish("d",3);
//    observe.RegisterPublish("e",3);
//    DORB_node_id_t a;
//    observe.Publish("a",nmpa);
//
//    int ret = observe.RegisterObserver("a",a);
//    if(ret) {
//        printf("ret ---> %d\n",ret);
//    }
//    printf("a=--> %d %d \n",a.fream_id,a.node_id);
//
//    char tmp_data[3];
//    observe.CopyData(a,tmp_data);
//
//    printf("tmp_data ---> %s \n",tmp_data);
//
//    ret = observe.RegisterObserver("b",a);
//    if(ret) {
//        printf("ret ---> %d\n",ret);
//    }
//    printf("b=--> %d %d \n",a.fream_id,a.node_id);
//
//    observe.UnRegisterPublish("a");
//    ret = observe.RegisterObserver("a",a);
//    if(ret) {
//        printf("ret ---> %d\n",ret);
//    }
//
//    return 0;

    ThreadPool pool;
    std::thread thd1([&pool] {
        for(int i = 0; i < 10; i++) {
            auto thdId = this_thread::get_id();
            pool.add_task([thdId] {
                cout << "同步层线程 1 的线程ID : "<< thdId << endl;
            });
        }
    });

    std::thread thd2([&pool] {
        for(int i = 0; i < 10; i++) {
            auto thdId = this_thread::get_id();
            pool.add_task([thdId] {
                cout << "同步层线程 2 的线程ID : "<< thdId << endl;
            });
        }
    });

    this_thread::sleep_for(std::chrono::seconds(2));
    pool.stop();
    thd1.join();
    thd2.join();

    return 0;

    point_cloud_vec clouds;

    for (int i = 0; i < 400; ++i) {
        point_cloud_t pos;
        pos.cloud_x = 1.0f;
        pos.cloud_y = 0.025f * i;
        pos.cloud_angle = 0.08f *i;
        clouds.push_back(pos);
    }

    auto tmp_vec = filter_by_length(clouds,0.10f,8.0f);
    int i = 0;
    for(auto it : tmp_vec) {
        printf(" i : %d ---> [%f %f %f]\n",i++,it.cloud_x,it.cloud_y,it.cloud_angle);
    }

    printf("\n=============================================\n\n");
    auto tmp = voxel_filter::VoxelFilter(0.2f).voxel_filter(tmp_vec);
    i = 0;

    for(auto it : tmp) {
        printf(" i : %d ---> [%f %f %f]\n",i++,it.cloud_x,it.cloud_y,it.cloud_angle);
    }

    return 0;
}