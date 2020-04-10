#include <std_msgs/String.h>
#include <boost/msm/back/mpl_graph_fsm_check.hpp>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/euml/common.hpp>
#include <boost/msm/front/euml/operator.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <iostream>
#include <math.h>

#include <ros/ros.h>
#include <mavros_msgs/SetMode.h> //For TakeOff
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/PoseStamped.h>
#include <detector_msgs/centre.h>
#include <detector_msgs/global_coord.h>
#include <detector_msgs/rotation.h>

#define echo(X) std::cout << X << std::endl

namespace msm = boost::msm;
namespace mpl = boost::mpl;

namespace ariitk::planner {

//state machine commands

struct CmdTakeOff {
    CmdTakeOff() {}
};

struct CmdEstimated {
    CmdEstimated() {}
};

struct CmdPass {
    CmdPass() {}
};

struct CmdGlobalT {
    CmdGlobalT() {}
};

class fsm : public msm::front::state_machine_def<fsm>
{
    private :

        nav_msgs::Odometry odom_;
        detector_msgs::centre centre_;
        detector_msgs::global_coord estimated_pose_;
        detector_msgs::global_coord rough_pose_;
        geometry_msgs::PoseStamped setpt_;

        ros::Publisher pose_pub_;

        ros::Subscriber odom_sub_; 
        ros::Subscriber centre_sub_;
        ros::Subscriber est_pose_sub_;

    public:
        bool verbose = true;
        template<class Event,class FSM>
        void on_entry(Event const &, FSM &);
        
        template<class Event,class FSM>
        void on_exit(Event const &, FSM &);

        //state struct definitons

        struct Rest : public msm::front::state<>
        {
            template <class Event, class FSM>
            void on_entry(Event const &, FSM &)
            {
                if (verbose){ echo("Entered Rest state"); }
            }

            template<class Event,class FSM>
            void on_exit(Event const &, FSM &)
            {
                if (verbose){ echo("Exited Rest state"); }
            }
        };

        struct Hover : public msm::front::state<>
        {
            template <class Event, class FSM>
            void on_entry(Event const &, FSM &)
            {
                if (verbose){ echo("Entered Hover state"); }
            }

            template<class Event,class FSM>
            void on_exit(Event const &, FSM &)
            {
                if (verbose){ echo("Exited Hover state"); }
            }
        };

        struct BeforePass : public msm::front::state<>
        {
            template <class Event, class FSM>
            void on_entry(Event const &, FSM &)
            {
                if (verbose){ echo("Entered before_pass state"); }
            }

            template<class Event,class FSM>
            void on_exit(Event const &, FSM &)
            {
                if (verbose){ echo("Exited before_pass state"); }
            }
        };

        struct AfterPass : public msm::front::state<>
        {
            template <class Event, class FSM>
            void on_entry(Event const &, FSM &)
            {
                if (verbose){ echo("Entered after_pass state"); }
            }

            template<class Event,class FSM>
            void on_exit(Event const &, FSM &)
            {
                if (verbose){ echo("Exited after_pass state"); }
            }
        };

        fsm(ros::NodeHandle& nh);

        //state transition funcitons
        void TakeOff (CmdTakeOff const &cmd);
        void DetectionBased (CmdEstimated const &cmd);
        void PrevCoord (CmdPass const &cmd);
        void GlobalT (CmdGlobalT const &cmd);

        //Callback functions
        void odomCallback(const nav_msgs::Odometry &msg) { odom_ = msg; };
        void centreCallback(const detector_msgs::centre &msg) { centre_ = msg; };
        void estimatedCallback(const detector_msgs::global_coord &msg) { estimated_pose_ = msg; }; 

        //transition table
        
        struct transition_table : mpl::vector<

        //      Type           Start             Event             Next               Action                      Gaurd 
        // +++ ------- + ----------------- + ---------------- + --------------- + -------------------------- + ----------------------------- +++
        
                a_row<    Rest             ,  CmdTakeOff      ,  Hover          , &fsm::TakeOff                                          >,

        // +++ ------- + ----------------- + ---------------- + --------------- + -------------------------- + ----------------------------- +++

                a_row<    Hover            ,  CmdEstimated    ,  BeforePass     , &fsm::DetectionBased                                   >,

        // +++ ------- + ----------------- + ---------------- + --------------- + -------------------------- + ----------------------------- +++

                a_row<    BeforePass       ,  CmdPass         ,  AfterPass      , &fsm::PrevCoord                                        >,

        // +++ ------- + ----------------- + ---------------- + --------------- + -------------------------- + ----------------------------- +++

                a_row<    AfterPass        ,  CmdGlobalT      ,  BeforePass     , &fsm::GlobalT                                          >

        // +++ ------- + ----------------- + ---------------- + --------------- + -------------------------- + ----------------------------- +++

        >{
        };

};

    
} // namespace ariitk::planner