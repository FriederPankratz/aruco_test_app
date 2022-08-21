#include <iostream>

#include <traact/traact.h>

#include <fstream>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <traact/serialization/JsonGraphInstance.h>

#include <traact/util/Logging.h>
#include <signal.h>
#include <spdlog/sinks/basic_file_sink.h>

bool running = true;
traact::facade::DefaultFacade my_facade;
void ctrlC(int i) {
    SPDLOG_INFO("User requested exit (Ctrl-C).");
    my_facade.stop();
}

void addBasicProcessing(traact::DefaultInstanceGraphPtr &graph, int index, std::string filename) {
    using namespace traact;
    using namespace traact::dataflow;

    auto source_name = fmt::format("source_{0}", index);
    auto undistort_color_name = fmt::format("undistort_color_{0}", index);
    auto color_to_gray_name = fmt::format("color_to_gray_{0}", index);
    auto aruco_tracker_name = fmt::format("aruco_tracker_{0}", index);

    auto render_image_name = fmt::format("render_image_{0}", index);
    auto render_pose_0_name = fmt::format("render_pose_0_{0}", index);
    auto render_pose_1_name = fmt::format("render_pose_1_{0}", index);

    DefaultPatternInstancePtr source_pattern =
        graph->addPattern(source_name,
                          my_facade.instantiatePattern("traact::component::kinect::KinectAzureSingleFilePlayer"));

    DefaultPatternInstancePtr undistort_color_pattern =
        graph->addPattern(undistort_color_name, my_facade.instantiatePattern("OpenCVUndistortImage"));

    DefaultPatternInstancePtr color_to_gray_pattern =
        graph->addPattern(color_to_gray_name, my_facade.instantiatePattern("OpenCvColorToGray"));

    DefaultPatternInstancePtr aruco_tracker_pattern =
        graph->addPattern(aruco_tracker_name, my_facade.instantiatePattern("OpenCvArucoTracker"));

    DefaultPatternInstancePtr
        render_image_pattern = graph->addPattern(render_image_name, my_facade.instantiatePattern("RenderImage"));

//    DefaultPatternInstancePtr
//        render_pose_0_pattern =
//        graph->addPattern(render_pose_0_name, my_facade.instantiatePattern("RenderPose6D"));
//    DefaultPatternInstancePtr
//        render_pose_1_pattern =
//        graph->addPattern(render_pose_1_name, my_facade.instantiatePattern("RenderPose6D"));

    // configure
    source_pattern->setParameter("file", filename);
    source_pattern->setParameter("stop_after_n_frames", -1);
    aruco_tracker_pattern->setParameter("Dictionary", "DICT_4X4_50");
    aruco_tracker_pattern->setParameter("marker_size", 0.08);
    auto& marker_0 = aruco_tracker_pattern->instantiatePortGroup("output_pose");
    marker_0.setParameter("marker_id", 1);
    auto& marker_1 = aruco_tracker_pattern->instantiatePortGroup("output_pose");
    marker_1.setParameter("marker_id", 4);
    //auto& debug_output = aruco_tracker_pattern->instantiatePortGroup("output_debug");



    render_image_pattern->setParameter("Window", render_image_name);
//    render_pose_0_pattern->setParameter("Window", render_image_name);
//    render_pose_1_pattern->setParameter("Window", render_image_name);
    
    
    // setup connections
    graph->connect(source_name, "output", undistort_color_name, "input");
    graph->connect(source_name, "output_calibration", undistort_color_name, "input_calibration");

    graph->connect(undistort_color_name, "output", color_to_gray_name, "input");

    graph->connect(color_to_gray_name, "output", aruco_tracker_name, "input");
    graph->connect(undistort_color_name, "output_calibration", aruco_tracker_name, "input_calibration");
    
    //graph->connect(aruco_tracker_name, debug_output.getProducerPortName("output"), render_image_name, "input");
    graph->connect(undistort_color_name, "output", render_image_name, "input");
    
//    graph->connect(aruco_tracker_name, marker_0.getProducerPortName("output"), render_pose_0_name, "input");
//    graph->connect(undistort_color_name, "output_calibration", render_pose_0_name, "input_calibration");
//    graph->connect(aruco_tracker_name, marker_1.getProducerPortName("output"), render_pose_1_name, "input");
//    graph->connect(undistort_color_name, "output_calibration", render_pose_1_name, "input_calibration");

    

}

int main(int argc, char **argv) {

    using namespace traact;
    using namespace traact::dataflow;
    using namespace traact::facade;

    signal(SIGINT, ctrlC);

    util::initLogging(spdlog::level::trace);

    DefaultInstanceGraphPtr graph = std::make_shared<DefaultInstanceGraph>("tracking");

    addBasicProcessing(graph, 0, "/home/frieder/data/recording_20210611_calib1/cn01/k4a_capture.mkv");
    addBasicProcessing(graph, 1, "/home/frieder/data/recording_20210611_calib1/cn02/k4a_capture.mkv");
    addBasicProcessing(graph, 2, "/home/frieder/data/recording_20210611_calib1/cn03/k4a_capture.mkv");
    addBasicProcessing(graph, 3, "/home/frieder/data/recording_20210611_calib1/cn04/k4a_capture.mkv");
    addBasicProcessing(graph, 4, "/home/frieder/data/recording_20210611_calib1/cn05/k4a_capture.mkv");
    addBasicProcessing(graph, 5, "/home/frieder/data/recording_20210611_calib1/cn06/k4a_capture.mkv");

    buffer::TimeDomainManagerConfig td_config;
    td_config.time_domain = 0;
    td_config.ringbuffer_size = 3;

    td_config.source_mode = SourceMode::WAIT_FOR_BUFFER;
    td_config.missing_source_event_mode = MissingSourceEventMode::WAIT_FOR_EVENT;
    td_config.max_offset = std::chrono::milliseconds(8);
    td_config.max_delay = std::chrono::milliseconds(100);
    td_config.sensor_frequency = 30;
    td_config.cpu_count = 0;

    graph->timedomain_configs[0] = td_config;



//    std::string filename = graph->name + ".json";
//    {
//        nlohmann::json jsongraph;
//        ns::to_json(jsongraph, *graph);
//
//        std::ofstream myfile;
//        myfile.open(filename);
//        myfile << jsongraph.dump(4);
//        myfile.close();
//
//        std::cout << jsongraph.dump(4) << std::endl;
//    }

    my_facade.loadDataflow(graph);

    my_facade.blockingStart();

    SPDLOG_INFO("exit program");

    //SPDLOG_INFO("run the same dataflow again using: traactConsole {0}", filename);

    return 0;
}