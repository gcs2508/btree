#include "BasePid.h"
#include <stdio.h>
float MotionNormalLine::line_uniformly_acc(float feed_back_vel,uint32_t dt) {

    if(run_time == 0) {
        acc_ctrl.set_gain(200.0f,0.01,2.0f,0.5f,-MOTION_MAX_ACC,MOTION_MAX_ACC);
        run_time = 1;
    } else {
        run_time += dt;
    }

    float target_dt = run_time * 0.001f + 0.02f;
    float target_vel = start_speed + uniformly_acc * target_dt;
    if(target_vel > uniform_speed)
        target_vel = uniform_speed;

    double acc = acc_ctrl.update_ctrl(target_vel,feed_back_vel);
    ctrl_vel = feed_back_vel + acc * 0.02f;
    printf("target_dt ---> %f target_vel ---> %f feed_back_vel %f acc %f\n",target_dt,target_vel,feed_back_vel,acc);
    return ctrl_vel;
}

int MotionNormalLine::run(double feed_back) {

    switch(motion_status) {
        case MOTION_BASE_NOT_INIT:
        case MOTION_LINE_UNIFORMLY_ACC:
            line_uniformly_acc(feed_back,20);
            if(ctrl_vel >= uniform_speed)
                motion_status = MOTION_LINE_UNIFORM;
            break;
        case MOTION_LINE_UNIFORM:
            break;
        case MOTION_LINE_IN_SLOW:
            break;
        case MOTION_LINE_UNIFORMLY_RETARDED:
            break;
    }
}