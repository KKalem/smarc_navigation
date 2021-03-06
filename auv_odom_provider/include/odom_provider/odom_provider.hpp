/* Copyright 2018 Ignacio Torroba (ignaciotb@kth.se)
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ODOM_PROVIDER_HPP
#define ODOM_PROVIDER_HPP

#include <ros/ros.h>

#include <eigen3/Eigen/Eigen>
#include <eigen3/Eigen/Dense>

#include <nav_msgs/Odometry.h>
#include <sensor_msgs/Imu.h>

#include <ros/transport_hints.h>
#include <message_filters/subscriber.h>
#include <message_filters/synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>

#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <geometry_msgs/TwistWithCovarianceStamped.h>
#include <geometry_msgs/Quaternion.h>
#include <geometry_msgs/TransformStamped.h>

#include <tf/tf.h>
#include <tf/transform_listener.h>
#include <tf/transform_broadcaster.h>

#include "noise_oneD_kf/noise_oneD_kf.hpp"

typedef message_filters::sync_policies::ApproximateTime<sensor_msgs::Imu,
        geometry_msgs::TwistWithCovarianceStamped> MsgTimingPolicy;

class OdomProvider{

public:
    OdomProvider(std::string node_name, ros::NodeHandle &nh);
    ~OdomProvider();
    void init();
    void provideOdom(const ros::TimerEvent&);

private:
    ros::NodeHandle* nh_;
    std::string node_name_;
    message_filters::Synchronizer<MsgTimingPolicy>* msg_synch_ptr_;
    message_filters::Subscriber<sensor_msgs::Imu>* imu_subs_;
    message_filters::Subscriber<geometry_msgs::TwistWithCovarianceStamped>* dvl_subs_;

    Eigen::VectorXd cumul_odom_;

    // Comms
    ros::Subscriber fast_imu_sub_;
    ros::Subscriber fast_dvl_sub_;
    ros::Subscriber tf_gt_subs_;
    ros::Publisher odom_pub_;
    ros::Publisher odom_inertial_pub_;
    ros::Publisher dvl_interpolated_;
    ros::Timer timer_;

    // Noise filters
    OneDKF* dvl_filter_x_;
    OneDKF* dvl_filter_y_;
    OneDKF* dvl_filter_z_;

    // Handlers for sensors
    std::deque<sensor_msgs::Imu> imu_readings_; // TODO: add limit size to queues
    std::deque<geometry_msgs::TwistWithCovarianceStamped> dvl_readings_;
    std::deque<nav_msgs::Odometry> gt_readings_;
    boost::mutex msg_lock_;
    bool init_filter_;

    // Aux
    double t_prev_;
    int dvl_cnt_;
    bool coord_;
    unsigned int size_imu_q_;
    unsigned int size_dvl_q_;

    // tf
    tf::TransformBroadcaster odom_bc_;
    tf::TransformListener tf_listener_;
    tf::StampedTransform transf_base_dvl_;
    tf::StampedTransform transf_odom_world_;
    std::string odom_frame_;
    std::string world_frame_;
    std::string base_frame_;
    std::string dvl_frame_;

    // Input callbacks
    void gtCB(const nav_msgs::Odometry &pose_msg);
    void synchSensorsCB(const sensor_msgs::ImuConstPtr &imu_msg, const geometry_msgs::TwistWithCovarianceStampedConstPtr &);
    void fastIMUCB(const sensor_msgs::Imu &imu_msg);
    void fastDVLCB(const geometry_msgs::TwistWithCovarianceStamped &dvl_msg);

    void interpolateDVL(ros::Time t_now, geometry_msgs::TwistWithCovarianceStampedPtr &dvl_msg_ptr);
    void computeOdom(const geometry_msgs::TwistWithCovarianceStampedPtr &dvl_msg, const tf::Quaternion &q_auv,
                     Eigen::VectorXd &u_t);

    bool sendOutput(ros::Time t_sens_input);

};


#endif // ODOM_PROVIDER_HPP
