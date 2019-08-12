/* Copyright (C) 2018 Pablo Hernandez-Cerdan
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/images/ImageContainerByITKImage.h>
#include <DGtal/images/imagesSetsUtils/ImageFromSet.h>
#include <DGtal/images/imagesSetsUtils/SetFromImage.h>
#include <DGtal/io/readers/GenericReader.h>
#include <DGtal/io/readers/ITKReader.h>
#include <chrono>
#include <iostream>
#include <unordered_map>
// boost::program_options
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
// graph
#include <DGtal/graph/ObjectBoostGraphInterface.h>
#include <DGtal/topology/Object.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graphviz.hpp>
// serialize
#include "spatial_graph_io.hpp"
// Boost Filesystem
#include <boost/filesystem.hpp>

// Reduce graph via dfs:
#include "merge_nodes.hpp"
#include "reduce_spatial_graph_via_dfs.hpp"
#include "remove_extra_edges.hpp"
#include "spatial_graph.hpp"
#include "spatial_graph_from_object.hpp"
#include "spatial_graph_utilities.hpp"

#ifdef VISUALIZE
// Viewer
#include <DGtal/io/Color.h>
#include <DGtal/io/DrawWithDisplay3DModifier.h>
#include <DGtal/io/colormaps/GradientColorMap.h>
#include <DGtal/io/viewers/Viewer3D.h>

#include "visualize_spatial_graph.hpp"
#include "visualize_spatial_graph_with_image.hpp"
// #include "itkViewImage.h"
#endif

// compute graph properties
#include "compute_graph_properties.hpp"
// transform to physical points
#include "transform_to_physical_point.hpp"
// #include "spatial_histograms.hpp"

using namespace DGtal;
using namespace std;
using namespace DGtal::Z3i;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, char *const argv[]) {
    /*-------------- Parse command line -----------------------------*/
    po::options_description opt_desc("Allowed options are: ");
    opt_desc.add_options()("help,h", "display this message.");
    opt_desc.add_options()("input,i", po::value<string>()->required(),
                           "Input thin image.");
    opt_desc.add_options()(
            "reduceGraph,r", po::bool_switch()->default_value(false),
            "Reduce obj graph into a new SpatialGraph, converting "
            "chain nodes (degree=2) into edge_points.");
    opt_desc.add_options()(
            "removeExtraEdges,c", po::bool_switch()->default_value(false),
            "Remove extra edges created because connectivity of object.");
    opt_desc.add_options()(
            "mergeThreeConnectedNodes,m",
            po::bool_switch()->default_value(false),
            "Merge three connected nodes (between themselves) into one node.");
    opt_desc.add_options()(
            "mergeFourConnectedNodes,q",
            po::bool_switch()->default_value(false),
            "Merge 4 connected nodes (between themselves) into one node.");
    opt_desc.add_options()(
            "mergeTwoThreeConnectedNodes,l",
            po::bool_switch()->default_value(false),
            "Merge 2 connected nodes of degree 3 (and edge with no "
            "points) into one node.");
    opt_desc.add_options()("checkParallelEdges,e",
                           po::bool_switch()->default_value(false),
                           "Check and print info about parallel edges in the "
                           "graph. Use verbose option for output.");
    opt_desc.add_options()("ignoreAngleBetweenParallelEdges,g",
                           po::bool_switch()->default_value(false),
                           "Don't compute angles between parallel edges.");
    opt_desc.add_options()("ignoreEdgesShorterThan,s",
                           po::value<size_t>()->default_value(0),
                           "Ignore distance and angles between edges shorter "
                           "than this value.");
    opt_desc.add_options()(
            "ignoreEdgesToEndNodes,x", po::bool_switch()->default_value(false),
            "Ignore distance and angles between edges to/from end "
            "nodes (degree = 1).");
    opt_desc.add_options()(
            "avoid_transformToPhysicalPoints,p",
            po::bool_switch()->default_value(false),
            "Positions in Spatial Graph takes into account metadata of the "
            "(origin,spacing,direction) itk image.");
    opt_desc.add_options()("spacing", po::value<string>()->default_value(""),
                           "Provide external spacing between voxels. Ignores "
                           "metadata of itk image and apply it.");
    opt_desc.add_options()(
            "output_filename_simple,z", po::bool_switch()->default_value(false),
            "Filename does not contain the parameters used for this filter.");
    opt_desc.add_options()("exportReducedGraph,o", po::value<string>(),
                           "Write .dot file with the reduced spatial graph.");
    opt_desc.add_options()("exportData,d", po::value<string>(),
                           "Write degrees, ete_distances, contour_lengths, "
                           "etc. Histograms can be "
                           "generated from these files afterwards.");
    opt_desc.add_options()(
            "exportSerialized,u", po::value<string>(),
            "Write serialized graph with the reduced spatial graph.");
#ifdef VISUALIZE
    opt_desc.add_options()("visualize,t",
                           po::bool_switch()->default_value(false),
                           "Visualize object with DGtal. Requires VISUALIZE "
                           "option enabled at build.");
#endif
    // ( "exportHistograms,e", po::value<string>(), "Export histogram." )
    // ( "binsHistoDegrees,d", po::value<size_t>()->default_value(0), "Bins for
    // the histogram of degrees. Default [0] get the breaks between 0 and
    // max_degree" ) ( "widthHistoDistances,l",
    // po::value<double>()->default_value(0.3), "Width between breaks for
    // histogram of ete distances. Use 0.0 to automatically compute breaks (not
    // recommended)." ) ( "binsHistoAngles,a",
    // po::value<size_t>()->default_value(100), "Bins for the histogram of
    // angles . Use 0 for automatic computation of breaks (not recommended)" ) (
    // "binsHistoCosines,n", po::value<size_t>()->default_value(100), "Bins for
    // the histogram of cosines .Use 0 for automatic computation of breaks (not
    // recommended)" )
    opt_desc.add_options()("verbose,v", po::bool_switch()->default_value(false),
                           "verbose output.");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, opt_desc), vm);
        if (vm.count("help") || argc <= 1) {
            std::cout << "Basic usage:\n" << opt_desc << "\n";
            return EXIT_SUCCESS;
        }
        po::notify(vm);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    string filename = vm["input"].as<string>();
    bool verbose = vm["verbose"].as<bool>();
    if (verbose)
        std::cout << "Filename: " << filename << std::endl;
    bool output_filename_simple = vm["output_filename_simple"].as<bool>();
    bool reduceGraph = vm["reduceGraph"].as<bool>();
    bool avoid_transformToPhysicalPoints =
            vm["avoid_transformToPhysicalPoints"].as<bool>();
    string spacing = vm["spacing"].as<string>();
    bool removeExtraEdges = vm["removeExtraEdges"].as<bool>();
    bool mergeThreeConnectedNodes = vm["mergeThreeConnectedNodes"].as<bool>();
    bool mergeFourConnectedNodes = vm["mergeFourConnectedNodes"].as<bool>();
    bool mergeTwoThreeConnectedNodes =
            vm["mergeTwoThreeConnectedNodes"].as<bool>();
    bool checkParallelEdges = vm["checkParallelEdges"].as<bool>();
    size_t ignoreEdgesShorterThan = vm["ignoreEdgesShorterThan"].as<size_t>();
    bool ignoreAngleBetweenParallelEdges =
            vm["ignoreAngleBetweenParallelEdges"].as<bool>();
    bool ignoreEdgesToEndNodes = vm["ignoreEdgesToEndNodes"].as<bool>();
    bool exportReducedGraph = vm.count("exportReducedGraph");
    bool exportSerialized = vm.count("exportSerialized");
    bool exportData = vm.count("exportData");
    // bool exportHistograms = vm.count("exportHistograms");
    // size_t binsHistoDegrees = vm["binsHistoDegrees"].as<size_t>();
    // double widthHistoDistances = vm["widthHistoDistances"].as<double>();
    // size_t binsHistoAngles = vm["binsHistoAngles"].as<size_t>();
    // size_t binsHistoCosines = vm["binsHistoCosines"].as<size_t>();

#ifdef VISUALIZE
    bool visualize = vm["visualize"].as<bool>();
#endif
    // Get filename without extension (and without folders).
    const fs::path input_stem = fs::path(filename).stem();
    const fs::path output_file_path =
            fs::path(input_stem.string() + "_REDUCED");

    using Domain = Z3i::Domain;
    using Image = ImageContainerByITKImage<Domain, unsigned char>;
    const unsigned int Dim = 3;
    using PixelType = unsigned char;
    using ItkImageType = itk::Image<PixelType, Dim>;
    // Read Image using ITK
    using ReaderType = itk::ImageFileReader<ItkImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(filename);
    reader->Update();

    // Convert to DGtal Container
    Image image(reader->GetOutput());

    using DigitalTopology = DT26_6;
    using DigitalSet = DGtal::DigitalSetByAssociativeContainer<
            Domain, std::unordered_set<typename Domain::Point> >;
    using Object = DGtal::Object<DigitalTopology, DigitalSet>;

    DigitalSet image_set(image.domain());
    SetFromImage<Z3i::DigitalSet>::append<Image>(image_set, image, 0, 255);

    KSpace ks;
    // Domain of kspace must be padded.
    ks.init(image.domain().lowerBound(), image.domain().upperBound(), true);
    DigitalTopology::ForegroundAdjacency adjF;
    DigitalTopology::BackgroundAdjacency adjB;
    DigitalTopology topo(adjF, adjB,
                         DGtal::DigitalTopologyProperties::JORDAN_DT);
    Object obj(topo, image_set);

#ifdef VISUALIZE
    if (visualize) {
        int argc(1);
        char **argv(nullptr);
        QApplication app(argc, argv);
        Viewer3D<> viewer(ks);
        viewer.show();

        viewer.setFillColor(Color(255, 255, 255, 255));
        viewer << image_set;

        // All kspace voxels
        // viewer.setFillColor(Color(40, 200, 55, 10));
        // viewer << all_set;

        viewer << Viewer3D<>::updateDisplay;

        app.exec();
    }
#endif

    if (reduceGraph) {
        using Graph = Object;
        const Graph &graph = obj;
        using SpatialGraph = SG::GraphAL;
        SpatialGraph sg =
                SG::spatial_graph_from_object<Object, SpatialGraph>(graph);
        // Remove extra edges
        if (removeExtraEdges) {
            if (verbose)
                std::cout << "Removing extra edges" << std::endl;
            size_t iterations = 0;
            while (true) {
                bool any_edge_removed = SG::remove_extra_edges(sg);
                if (any_edge_removed)
                    iterations++;
                else
                    break;
            }
            if (verbose)
                std::cout << "Removed extra edges iteratively " << iterations
                          << " times" << std::endl;
        }
        SpatialGraph reduced_g = SG::reduce_spatial_graph_via_dfs(sg);

        bool inPlace = true;
        if (mergeThreeConnectedNodes) {
            if (verbose) {
                std::cout << "Merging three connecting nodes... " << std::endl;
            }
            auto nodes_merged =
                    SG::merge_three_connected_nodes(reduced_g, inPlace);
            if (verbose) {
                std::cout
                        << nodes_merged
                        << " interconnected nodes with degree 3 were merged."
                           "Those nodes have now degree 0 if inPlace is not set"
                        << std::endl;
            }
        }

        if (mergeFourConnectedNodes) {
            if (verbose) {
                std::cout << "Merging four connecting nodes... " << std::endl;
            }
            auto nodes_merged =
                    SG::merge_four_connected_nodes(reduced_g, inPlace);
            if (verbose) {
                std::cout
                        << nodes_merged
                        << " interconnected nodes with degree 4 were merged."
                           "Those nodes have now degree 0 if inPlace is not set"
                        << std::endl;
            }
        }

        if (mergeTwoThreeConnectedNodes) {
            if (verbose) {
                std::cout << "Merging two degree 3 nodes... " << std::endl;
            }
            auto nodes_merged =
                    SG::merge_two_three_connected_nodes(reduced_g, inPlace);
            if (verbose) {
                std::cout
                        << nodes_merged
                        << " two interconnected nodes with degree 3 "
                           "and no edge points between them  were merged. "
                           "Those nodes have now degree 0 if inPlace is not set"
                        << std::endl;
            }
        }

        if (checkParallelEdges) {
            if (verbose)
                std::cout << "Checking parallel edges... " << std::endl;

            auto parallel_edges = SG::get_parallel_edges(reduced_g);
            auto equal_parallel_edges =
                    SG::get_equal_parallel_edges(parallel_edges, reduced_g);
            if (verbose) {
                std::cout << "Found " << parallel_edges.size()
                          << " parallel edges. " << equal_parallel_edges.size()
                          << " are equal!." << std::endl;

                if (!equal_parallel_edges.empty()) {
                    std::cout << "Equal parallel edges between vertex:\n";
                    for (const auto &edge_pair : equal_parallel_edges)
                        std::cout << boost::source(edge_pair.first, reduced_g)
                                  << "---"
                                  << boost::target(edge_pair.first, reduced_g)
                                  << std::endl;
                }
            }
        }

        ItkImageType::SpacingType itk_spacing;
        itk_spacing.Fill(1.0);
        if (!avoid_transformToPhysicalPoints) {
            if (verbose) {
                // Print metadata of itk image
                std::cout << "Origin: " << reader->GetOutput()->GetOrigin()
                          << std::endl;
                std::cout << "Spacing: " << reader->GetOutput()->GetSpacing()
                          << std::endl;
                std::cout << "Direction:\n "
                          << reader->GetOutput()->GetDirection() << std::endl;
            }
            itk_spacing = reader->GetOutput()->GetSpacing();
            SG::transform_graph_to_physical_space<ItkImageType>(
                    reduced_g, reader->GetOutput());
            if (spacing != "") {
                std::istringstream in(spacing);
                double sp;
                for (size_t i = 0; i < ItkImageType::ImageDimension; i++) {
                    in >> sp;
                    itk_spacing[i] = sp;
                }

                if (verbose) {
                    std::cout << "Changing Spacing to: " << itk_spacing
                              << std::endl;
                }

                reader->GetOutput()->SetSpacing(itk_spacing);
                SG::transform_graph_to_physical_space<ItkImageType>(
                        reduced_g, reader->GetOutput());
            }
        }
        // Format itk_spacing into a string:
        std::ostringstream sp_stream;
        sp_stream << itk_spacing[0] << "_" << itk_spacing[1] << "_"
                  << itk_spacing[2];
        auto sp_string = sp_stream.str();
        std::cout << "spacing: " << sp_string << std::endl;

        // Check unique points of graph
        auto repeated_points = SG::check_unique_points_in_graph(reduced_g);
        if (repeated_points.second) {
            std::cout << "Warning: duplicated points exist in reduced_g"
                         "Repeated Points: "
                      << repeated_points.first.size() << std::endl;
            for (const auto &p : repeated_points.first) {
                SG::print_pos(std::cout, p);
                std::cout << std::endl;
            }
        }

        if (exportReducedGraph) {
            string exportReducedGraph_filename =
                    vm["exportReducedGraph"].as<string>();
            boost::dynamic_properties dp;
            dp.property("node_id", boost::get(boost::vertex_index, reduced_g));
            dp.property("spatial_node",
                        boost::get(boost::vertex_bundle, reduced_g));
            dp.property("spatial_edge",
                        boost::get(boost::edge_bundle, reduced_g));
            {
                const fs::path output_folder_path{exportReducedGraph_filename};
                if (!fs::exists(output_folder_path)) {
                    throw std::runtime_error("output folder doesn't exist : " +
                                             output_folder_path.string());
                }
                std::string output_full_string = output_file_path.string();
                if (!output_filename_simple) {
                    output_full_string +=
                            "_sp" + sp_string + (removeExtraEdges ? "_c" : "") +
                            (mergeThreeConnectedNodes ? "_m" : "");
                }
                fs::path output_full_path =
                        output_folder_path /
                        fs::path(output_full_string + ".dot");
                std::ofstream out;
                out.open(output_full_path.string().c_str());

                boost::write_graphviz_dp(out, reduced_g, dp);
                if (verbose)
                    std::cout << "Output reduced graph (graphviz) to: "
                              << output_full_path.string() << std::endl;
            }
        }

        if (exportSerialized) {
            string exportReducedGraph_filename =
                    vm["exportReducedGraph"].as<string>();
            const fs::path output_folder_path{exportReducedGraph_filename};
            if (!fs::exists(output_folder_path)) {
                throw std::runtime_error("output folder doesn't exist : " +
                                         output_folder_path.string());
            }

            std::string output_full_string = output_file_path.string();
            if (!output_filename_simple) {
                output_full_string += "_sp" + sp_string +
                                      (removeExtraEdges ? "_c" : "") +
                                      (mergeThreeConnectedNodes ? "_m" : "");
            }
            fs::path output_full_path =
                    output_folder_path / fs::path(output_full_string + ".txt");
            SG::write_serialized_sg(output_full_path.string(), reduced_g);
            if (verbose)
                std::cout << "Output reduced graph (serialized) to: "
                          << output_full_path.string() << std::endl;
        }
#ifdef VISUALIZE
        if (visualize) {
            SG::visualize_spatial_graph(reduced_g);
            // itk::Testing::ViewImage(reader->GetOutput());
            SG::visualize_spatial_graph_with_image(reduced_g,
                                                   reader->GetOutput());
        }
#endif

        if (exportData) {
            fs::path data_output_folder_path =
                    fs::path(vm["exportData"].as<string>());

            // string exportHistograms_filename =
            // vm["exportHistograms"].as<string>(); const fs::path
            // histo_output_folder_path{exportHistograms_filename};
            // if(!fs::exists(histo_output_folder_path)) {
            //     throw std::runtime_error("histo_output folder doesn't exist :
            //     " + histo_output_folder_path.string());
            // }
            // fs::path histo_output_full_path = histo_output_folder_path /
            // fs::path(
            //     output_file_path.string() +
            //     ( removeExtraEdges ? "_c" : "")   +
            //     ( mergeThreeConnectedNodes ? "_m" : "")   +
            //     ( ignoreAngleBetweenParallelEdges ? "_iPA" : "")   +
            //     ( ignoreEdgesToEndNodes ? "_x" : "")   +
            //     ( ignoreEdgesShorterThan ?
            //        "_iShort" + std::to_string(ignoreEdgesShorterThan)  : "")
            //        +
            //     "_bD" + std::to_string(binsHistoDegrees)  +
            //     "_bA" + std::to_string(binsHistoAngles)  +
            //     "_bC" + std::to_string(binsHistoCosines)  +
            //     "_wL" + std::to_string(widthHistoDistances)  +
            //     ".histo");
            // std::ofstream histo_out;
            // histo_out.open(histo_output_full_path.string().c_str());

            if (!fs::exists(data_output_folder_path)) {
                throw std::runtime_error("data_output folder doesn't exist : " +
                                         data_output_folder_path.string());
            }

            std::string data_output_full_string =
                    output_file_path.string() + "_DATA";
            if (!output_filename_simple) {
                data_output_full_string +=
                        ("_sp" + sp_string) + (removeExtraEdges ? "_c" : "") +
                        (mergeThreeConnectedNodes ? "_m" : "") +
                        (ignoreAngleBetweenParallelEdges ? "_iPA" : "") +
                        (ignoreEdgesToEndNodes ? "_x" : "") +
                        (ignoreEdgesShorterThan
                                 ? "_iShort" + std::to_string(
                                                       ignoreEdgesShorterThan)
                                 : "");
            }
            fs::path data_output_full_path =
                    data_output_folder_path /
                    fs::path(data_output_full_string + ".txt");
            std::ofstream data_out;
            data_out.setf(std::ios_base::fixed, std::ios_base::floatfield);
            data_out.open(data_output_full_path.string().c_str());
            // Degrees
            {
                auto degrees = SG::compute_degrees(reduced_g);
                // auto histo_degrees = SG::histogram_degrees(degrees,
                // binsHistoDegrees); SG::print_histogram(histo_degrees,
                // histo_out);
                {
                    data_out.precision(
                            std::numeric_limits<decltype(
                                    degrees)::value_type>::max_digits10);
                    data_out << "# degrees" << std::endl;
                    std::ostream_iterator<decltype(degrees)::value_type>
                            out_iter(data_out, " ");
                    std::copy(std::begin(degrees), std::end(degrees), out_iter);
                    data_out << std::endl;
                }
            }
            // EndToEnd Distances
            {
                auto ete_distances = SG::compute_ete_distances(
                        reduced_g, ignoreEdgesShorterThan,
                        ignoreEdgesToEndNodes);

                auto range_ptr = std::minmax_element(ete_distances.begin(),
                                                     ete_distances.end());
                if (verbose) {
                    std::cout << "Min Distance: " << *range_ptr.first
                              << std::endl;
                    std::cout << "Max Distance: " << *range_ptr.second
                              << std::endl;
                }
                // auto histo_ete_distances =
                // SG::histogram_ete_distances(ete_distances,
                // widthHistoDistances);
                // SG::print_histogram(histo_ete_distances, histo_out);
                {
                    data_out.precision(
                            std::numeric_limits<decltype(
                                    ete_distances)::value_type>::max_digits10);
                    data_out << "# ete_distances" << std::endl;
                    std::ostream_iterator<decltype(ete_distances)::value_type>
                            out_iter(data_out, " ");
                    std::copy(std::begin(ete_distances),
                              std::end(ete_distances), out_iter);
                    data_out << std::endl;
                }
            }
            // Angles between adjacent edges
            {
                auto angles = SG::compute_angles(
                        reduced_g, ignoreEdgesShorterThan,
                        ignoreAngleBetweenParallelEdges, ignoreEdgesToEndNodes);
                // auto histo_angles = SG::histogram_angles( angles,
                // binsHistoAngles ); SG::print_histogram(histo_angles,
                // histo_out);
                {
                    data_out.precision(
                            std::numeric_limits<decltype(
                                    angles)::value_type>::max_digits10);
                    data_out << "# angles" << std::endl;
                    std::ostream_iterator<decltype(angles)::value_type>
                            out_iter(data_out, " ");
                    std::copy(std::begin(angles), std::end(angles), out_iter);
                    data_out << std::endl;
                }
                // Cosines of those angles
                {
                    auto cosines = SG::compute_cosines(angles);
                    // auto histo_cosines = SG::histogram_cosines( cosines,
                    // binsHistoCosines ); SG::print_histogram(histo_cosines,
                    // histo_out);
                    {
                        data_out.precision(
                                std::numeric_limits<decltype(
                                        cosines)::value_type>::max_digits10);
                        data_out << "# cosines" << std::endl;
                        std::ostream_iterator<decltype(cosines)::value_type>
                                out_iter(data_out, " ");
                        std::copy(std::begin(cosines), std::end(cosines),
                                  out_iter);
                        data_out << std::endl;
                    }
                }
            }
            // Contour length
            {
                auto contour_lengths = SG::compute_contour_lengths(
                        reduced_g, ignoreEdgesShorterThan,
                        ignoreEdgesToEndNodes);
                // auto histo_contour_lengths =
                // SG::histogram_contour_lengths(contour_lengths,
                // widthHistoDistances);
                // SG::print_histogram(histo_contour_lengths, histo_out);
                {
                    data_out.precision(std::numeric_limits<decltype(
                                               contour_lengths)::value_type>::
                                               max_digits10);
                    data_out << "# contour_lengths" << std::endl;
                    std::ostream_iterator<decltype(contour_lengths)::value_type>
                            out_iter(data_out, " ");
                    std::copy(std::begin(contour_lengths),
                              std::end(contour_lengths), out_iter);
                    data_out << std::endl;
                }
            }
            if (verbose) {
                std::cout << "Output data to: "
                          << data_output_full_path.string() << std::endl;
                // std::cout << "Output histograms to: " <<
                // histo_output_full_path.string() << std::endl;
            }
        } // end export histograms
    }
}
