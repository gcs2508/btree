#ifndef _BASE_PID_H_
#define _BASE_PID_H_

#include <math.h>
#include <stdint.h>

class PidControl {

public:
    PidControl() = default;
    ~PidControl() = default;

    void set_gain(const double &kp,const double &ki,const double &kd,const double &dt,const double &min,const double &max) {
        gain_kd = kd;
        gain_ki = ki;
        gain_kp = kp;

        _min = min;
        _max = max;

        _dt = dt;

        pre_error = 0;
        _integral = 0;
    }

    double update_ctrl(double target_distance,double current_distance) {

        double error = target_distance - current_distance;
        double p_out = gain_kp * error;

        //dt 可以为实际时间
        _integral += error * _dt;
        double i_out = gain_ki * _integral;

        double derivative = (error - pre_error) / _dt;
        double d_out = gain_kd * derivative;

        double output = p_out + d_out + i_out;

        if(output > _max) {
            output = _max;
        } else if(output < _min) {
            output = _min;
        }

        debug_control = output;
        pre_error = error;
        return output;
    }

    double get_debug_contrl() {
        return debug_control;
    }


private:
    double gain_kp;
    double gain_ki;
    double gain_kd;

    double _min;
    double _max;

    double _dt;

    double _integral;
    double pre_error;

    // for debug
    double debug_control;
};

typedef struct point_2d {
    float pos_x;
    float pos_y;

    float yaw;
}point_2d_t;

class MotionNormalBase {

public:
    MotionNormalBase() = default;
    ~MotionNormalBase() = default;

    virtual void init(point_2d_t start,point_2d_t end) {
        start_pos = start;
        end_pos = end;
    }

    virtual int run(double feed_back) {return 0;}
    virtual void stop() { }

protected:
    point_2d_t start_pos;
    point_2d_t end_pos;
};

typedef enum {
    MOTION_BASE_NOT_INIT = 0,        //未初始化动作
    MOTION_LINE_UNIFORMLY_ACC = 1,   //匀加速运动
    MOTION_LINE_UNIFORMLY_RETARDED,  //匀减速运动
    MOTION_LINE_UNIFORM,             //匀速运动
    MOTION_LINE_IN_SLOW,             //缓速运动
}MOTION_NORMAL_STATUS;

/**
 * @brief  Chassis Imu 数据结构体
 * */
typedef struct chassis_imu_sensor {

//    uint64_t system_time;
    uint32_t imu_stamp_time;
    uint32_t encoder_stamp_time;

    int32_t left_encoder;
    int32_t right_encoder;

    float imu_pitch;
    float imu_roll;
    float imu_yaw;
}chassis_imu_sensor_t;

#define POINT_2D_DISTANCE(a,b)  (sqrt(powf(a.pos_x - b.pos_x,2)+powf(a.pos_y - b.pos_y,2)))
#define JUDGE_ZERO_F(a)         (a < 0.0001f && a > -0.0001f)

#define MOTION_MAX_SPEED            (0.30f)
#define MOTION_MAX_ACC              (0.25f)

// 直线 运动
class MotionNormalLine : public MotionNormalBase {
public:
    MotionNormalLine() : MotionNormalBase(),motion_status() {}
    ~MotionNormalLine() = default;

    virtual void init(point_2d_t start,point_2d_t end) {
//        start_pos = start;
//        end_pos = end;

//        float time = uniform_speed / uniformly_acc;
//        float distance = uniformly_acc * time * time;

//        double total_distance = POINT_2D_DISTANCE(start_pos,end_pos);
//        if(total_distance > distance) {
//            max_speed = uniform_speed;
//            total_time = (uint32_t)time * 1000;
            motion_status = MOTION_LINE_UNIFORMLY_ACC;
//        } else {
//            max_speed = end_speed;
//            total_time = 0;
//            motion_status = MOTION_LINE_UNIFORM;
//        }

        run_time = 0;
    }

    void set_max_speed(float speed) {
        max_speed = speed;
    }

    void set_start_speed(float speed) {
        start_speed = speed;
    }

    void set_end_speed(float speed) {
        end_speed = speed;
    }

    void set_uniform_speed(float speed) {
        uniform_speed = speed;
    }

    void set_uniformly_acc(float acc) {
        uniformly_acc = acc;
    }

    virtual int run(double freed_back);
    virtual void stop() {}

    double get_ctrl_vel() {
        return ctrl_vel;
    }

protected:
    float line_uniformly_acc(float feed_back_vel,uint32_t dt);

private:
    uint8_t motion_status;
    float max_speed;
    float uniformly_acc;
    float uniform_speed;

    float start_speed;
    float end_speed;

    float ctrl_vel;
    float stop_distance;
    uint32_t total_time;
    uint32_t run_time;

    PidControl acc_ctrl;
    chassis_imu_sensor_t last_odom;
};






#endif //_BASE_PID_H_
