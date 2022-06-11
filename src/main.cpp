#include <iostream>

#include <traact/traact.h>
#include <traact/facade/DefaultFacade.h>
#include <traact/component/spatial/util/Pose6DTestSource.h>

#include <fstream>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <traact/serialization/JsonGraphInstance.h>
#include <traact/component/generic/ApplicationSyncSink.h>

#include <traact/util/Logging.h>
#include <signal.h>
#include <spdlog/sinks/basic_file_sink.h>

bool running = true;
traact::facade::DefaultFacade my_facade;
void ctrlC(int i) {
    SPDLOG_INFO("User requested exit (Ctrl-C).");
    my_facade.stop();
}

#define WITH_RENDERER

int main(int argc, char **argv) {

    using namespace traact::facade;
    using namespace traact;
    using namespace traact::dataflow;
    util::initLogging(spdlog::level::trace);

    DefaultInstanceGraphPtr pattern_graph_ptr = std::make_shared<DefaultInstanceGraph>("tracking");

    DefaultPatternInstancePtr
        source_pattern =
        pattern_graph_ptr->addPattern("source", my_facade.instantiatePattern("KinectAzureSingleFilePlayer"));

    DefaultPatternInstancePtr
        color_to_gray_pattern =
        pattern_graph_ptr->addPattern("color_to_gray", my_facade.instantiatePattern("OpenCvColorToGray"));

    DefaultPatternInstancePtr
        aruco_input_pattern = pattern_graph_ptr->addPattern("aruco_input", my_facade.instantiatePattern("ArucoInput"));
    DefaultPatternInstancePtr
        aruco_output0_pattern =
        pattern_graph_ptr->addPattern("aruco_output0", my_facade.instantiatePattern("ArucoOutput"));
    DefaultPatternInstancePtr
        aruco_output1_pattern =
        pattern_graph_ptr->addPattern("aruco_output1", my_facade.instantiatePattern("ArucoOutput"));
    DefaultPatternInstancePtr
        pose_print0_pattern = pattern_graph_ptr->addPattern("pose_print0", my_facade.instantiatePattern("Pose6DPrint"));
    DefaultPatternInstancePtr
        pose_print1_pattern = pattern_graph_ptr->addPattern("pose_print1", my_facade.instantiatePattern("Pose6DPrint"));
    DefaultPatternInstancePtr
        undistort_color_pattern =
        pattern_graph_ptr->addPattern("undistort_color", my_facade.instantiatePattern("OpenCVUndistortImage"));
    // track marker
    pattern_graph_ptr->connect("source", "output", "undistort_color", "input");
    pattern_graph_ptr->connect("source", "output_calibration", "undistort_color", "input_calibration");

    pattern_graph_ptr->connect("undistort_color", "output", "color_to_gray", "input");
    pattern_graph_ptr->connect("color_to_gray", "output", "aruco_input", "input");
    pattern_graph_ptr->connect("undistort_color", "output_calibration", "aruco_input", "input_calibration");

    // log output
    pattern_graph_ptr->connect("aruco_output0", "output", "pose_print0", "input");
    pattern_graph_ptr->connect("aruco_output1", "output", "pose_print1", "input");

    source_pattern->local_pattern.parameter["file"]["value"] =
        "/home/frieder/data/recording_20210611_calib1/cn03/k4a_capture.mkv";
    source_pattern->local_pattern.parameter["stop_after_n_frames"]["value"] = -1;
    aruco_input_pattern->local_pattern.parameter["Dictionary"]["value"] = "DICT_4X4_50";
    aruco_input_pattern->local_pattern.parameter["MarkerSize"]["value"] = 0.08;
    aruco_output0_pattern->local_pattern.parameter["marker_id"]["value"] = 1;
    aruco_output1_pattern->local_pattern.parameter["marker_id"]["value"] = 4;

#ifdef WITH_RENDERER
    DefaultPatternInstancePtr
        aruco_debug_output_pattern =
        pattern_graph_ptr->addPattern("aruco_debug_output", my_facade.instantiatePattern("ArucoDebugOutput"));
    DefaultPatternInstancePtr
        render_window_pattern = pattern_graph_ptr->addPattern("sink", my_facade.instantiatePattern("RenderImage"));
    DefaultPatternInstancePtr
        render_pose0_pattern =
        pattern_graph_ptr->addPattern("sink_pose0", my_facade.instantiatePattern("RenderPose6D"));
    DefaultPatternInstancePtr
        render_pose1_pattern =
        pattern_graph_ptr->addPattern("sink_pose1", my_facade.instantiatePattern("RenderPose6D"));


    pattern_graph_ptr->connect("aruco_debug_output", "output", "sink", "input");
    pattern_graph_ptr->connect("aruco_output0", "output", "sink_pose0", "input");
    pattern_graph_ptr->connect("undistort_color", "output_calibration", "sink_pose0", "input_calibration");
    pattern_graph_ptr->connect("aruco_output1", "output", "sink_pose1", "input");
    pattern_graph_ptr->connect("undistort_color", "output_calibration", "sink_pose1", "input_calibration");


    render_window_pattern->local_pattern.parameter["window"]["value"] = "ArucoImage";
    render_pose0_pattern->local_pattern.parameter["window"]["value"] = "ArucoImage";
    render_pose1_pattern->local_pattern.parameter["window"]["value"] = "ArucoImage";
#endif

    buffer::TimeDomainManagerConfig td_config;
    td_config.time_domain = 0;
    td_config.ringbuffer_size = 3;
    td_config.master_source = "source";
    td_config.source_mode = SourceMode::WAIT_FOR_BUFFER;
    td_config.missing_source_event_mode = MissingSourceEventMode::WAIT_FOR_EVENT;
    td_config.max_offset = std::chrono::milliseconds(10);
    td_config.max_delay = std::chrono::milliseconds(100);
    td_config.measurement_delta = std::chrono::nanoseconds(33333333);

    pattern_graph_ptr->timedomain_configs[0] = td_config;

    std::string filename = pattern_graph_ptr->name + ".json";
    {
        nlohmann::json jsongraph;
        ns::to_json(jsongraph, *pattern_graph_ptr);

        std::ofstream myfile;
        myfile.open(filename);
        myfile << jsongraph.dump(4);
        myfile.close();

        std::cout << jsongraph.dump(4) << std::endl;
    }

    my_facade.loadDataflow(filename);

    my_facade.blockingStart();

//    while(running) {
//        std::this_thread::sleep_for(std::chrono::milliseconds(500));
//    }
//
//    my_facade.stop();

    SPDLOG_INFO("exit program");

    SPDLOG_INFO("run the same dataflow again using: traactConsole {0}", filename);

    return 0;
}