/* ********************************************************************
 * Copyright (C) 2020 Pablo Hernandez-Cerdan.
 *
 * This file is part of SGEXT: http://github.com/phcerdan/sgext.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * *******************************************************************/

#include <iostream>
#include <string>
#include "fill_holes_function.hpp" // fill_holes_voting_iterative_function
// ITK
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkInvertIntensityImageFilter.h>
// boost::program_options
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
// boost::filesystem
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, char* const argv[]) {
  /*-------------- Parse command line -----------------------------*/
  po::options_description opt_desc("Allowed options are: ");
  opt_desc.add_options()("help,h", "display this message.");
  opt_desc.add_options()("input,i", po::value<std::string>()->required(),
                         "Input 3D binary image file.");
  opt_desc.add_options()("foreground,f",
                         po::value<std::string>()->default_value("white"),
                         "foreground color in binary image. [black|white]");
  opt_desc.add_options()(
      "majority", po::value<int>()->default_value(3),
      "Majority needed in the neighborhood to switch pixel to ON");
  opt_desc.add_options()("radius", po::value<int>()->default_value(1),
                         "Radius to define the neighborhood");
  opt_desc.add_options()("iterations", po::value<size_t>()->default_value(1000),
                         "Number of max iterations");
  opt_desc.add_options()(
      "output_filename_simple,z", po::bool_switch()->default_value(false),
      "Filename does not contain the parameters used for this filter.");
  opt_desc.add_options()("verbose,v", po::bool_switch()->default_value(false),
                         "verbose output");
  opt_desc.add_options()("outputFolder,o", po::value<std::string>()->required(),
                         "Folder to export the resulting binary image.");
  opt_desc.add_options()("outputFilename,e", po::value<std::string>(),
                         "FileName of the output (needs outputFolder).");

  //  Majority is the number of pixels in the neighborhood of an OFF pixel, to
  //  turn it into ON. By default majority = 1, this means that an off pixel
  //  will be turned on if in the neighborhood (set by radius) there are at
  //  least 50% + 1 pixels ON. If radius = 1,1,1, neighborhood size will be 3x3
  //  = 9 pixels. if 5 pixels around an OFF pixel are ON, then it will be
  //  switched.

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, opt_desc), vm);
    if(static_cast<bool>(vm.count("help")) || argc <= 1) {
      std::cout << "Basic usage:\n" << opt_desc << "\n";
      return EXIT_SUCCESS;
    }
    po::notify(vm);
  } catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  // Parse options
  std::string filename = vm["input"].as<std::string>();
  int majority = vm["majority"].as<int>();
  int radius = vm["radius"].as<int>();
  size_t iterations = vm["iterations"].as<size_t>();
  bool verbose = vm["verbose"].as<bool>();
  bool output_filename_simple = vm["output_filename_simple"].as<bool>();
  std::string foreground = vm["foreground"].as<std::string>();
  if(static_cast<bool>(vm.count("foreground")) &&
     (!(foreground == "white" || foreground == "black"))) {
    throw po::validation_error(po::validation_error::invalid_option_value,
                               "foreground");
  }
  const bool invert_image = (foreground == "black");
  if(static_cast<bool>(vm.count("outputFolder"))) {
    const fs::path output_folder_path{vm["outputFolder"].as<std::string>()};
    if(!fs::exists(output_folder_path)) {
      std::cerr << "output folder doesn't exist : "
                << output_folder_path.string() << std::endl;
      throw po::validation_error(po::validation_error::invalid_option_value,
                                 "output_folder_path");
    }
  }

  const fs::path input_stem = fs::path(filename).stem();
  fs::path output_full_path;
  fs::path output_folder_path;
  fs::path output_file_path;
  /*-------------- End of parse -----------------------------*/
  // Get filename without extension (and without folders).
  if(static_cast<bool>(vm.count("outputFolder"))) {
    output_folder_path = vm["outputFolder"].as<std::string>();
  }

  if(static_cast<bool>(vm.count("outputFilename"))) {
    output_file_path = fs::path(vm["outputFilename"].as<std::string>());
    output_full_path = output_folder_path / fs::path(output_file_path.string());
  } else {
    std::string output_file_string = input_stem.string() + "_FILLED";
    if(!output_filename_simple) {
      output_file_string += "_M" + std::to_string(majority) + "_R" +
                            std::to_string(radius) + "_N" +
                            std::to_string(iterations);
    }
    output_file_path = fs::path(output_file_string);
    output_full_path =
        output_folder_path / fs::path(output_file_path.string() + ".nrrd");
  }
  if(verbose) {
    std::cout << "output_folder_path: " << output_folder_path.string()
              << std::endl;
    std::cout << "output_file_path: " << output_file_path.string() << std::endl;
    std::cout << "output_full_path: " << output_full_path.string() << std::endl;
  }

  // Read Image using ITK
  using PixelType = unsigned char;
  const unsigned int Dim = 3;
  using ImageType = itk::Image<PixelType, Dim>;
  using ReaderType = itk::ImageFileReader<ImageType>;
  auto reader = ReaderType::New();
  reader->SetFileName(filename);
  reader->Update();

  // Invert Filter using ITK.
  using InverterType = itk::InvertIntensityImageFilter<ImageType, ImageType>;
  auto inverter = InverterType::New();
  if(invert_image) {
    inverter->SetInput(reader->GetOutput());
    inverter->Update();
  }
  ImageType::Pointer image =
      (invert_image) ? inverter->GetOutput() : reader->GetOutput();

  const auto filled_image = SG::fill_holes_voting_iterative_function(
      image, radius, majority, iterations, verbose);

  using WriterType = itk::ImageFileWriter<ImageType>;
  auto writer = WriterType::New();
  try {
    writer->SetFileName(output_full_path.string().c_str());
    writer->SetInput(filled_image);
    writer->UseCompressionOn();
    writer->Update();
  } catch(itk::ExceptionObject& e) {
    std::cerr << "Failure writing file: " << output_full_path.string()
              << std::endl;
    std::cerr << e.what() << std::endl;
    throw e;
  }
  if(verbose) {
    std::cout << "Filling holes finished" << std::endl;
    std::cout << "Output: " << output_full_path.string() << std::endl;
  }
}
