// Copyright (c) 2015 Zachary Kann
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// ---
// Author: Zachary Kann

#include <fstream>
#include <cmath>
#include <complex>
#include <armadillo>
#include "boost/program_options.hpp"
#include "z_constants.hpp"
#include "z_conversions.hpp"
#include "z_string.hpp"

namespace po = boost::program_options;

int main (int argc, char *argv[]) {

  enum Chromophore {kOH, kOD};

  std::string V_filename;
  po::options_description desc("Options");
  desc.add_options()
    ("help,h",  "Print help messages")
    ("V_file,c", po::value<std::string>(&V_filename),
     "Choice of chromophore (OH or OD)")
    ("chromophore,c", po::value<std::string>(),
     "Choice of chromophore (OH or OD)");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    exit(EXIT_SUCCESS);
  }

  Chromophore chromophore;
  const std::string& vm_chromophore =
      vm["chromophore"].as<std::string>();
  if (vm_chromophore == "OH")
    chromophore = kOH;
  else if (vm_chromophore == "OD")
    chromophore = kOD;
  else
    assert(false && "Unrecognized chromophore option");

  double red_mass = (chromophore == kOH) ? MASS_H*MASS_OD : MASS_D*MASS_OH;
  red_mass *= AMU_TO_KG/MASS_HOD;
  double delta_x = 0.02e-10;
  int points = 56;

  arma::mat T = arma::zeros<arma::mat>(points, points);
  arma::mat V = arma::zeros<arma::mat>(points, points);
  arma::mat H = arma::zeros<arma::mat>(points, points);

  std::ifstream V_file(V_filename.c_str());

  for (int i=0; i<points; i++)
    V_file >> V(i,i);

  for (int i_col = 0; i_col < points; ++i_col) {
    for (int i_row = 0; i_row < points; ++i_row) {
      T(i_row, i_col) =
          H_BAR*H_BAR/2.0/red_mass/delta_x/delta_x/C_SPEED_CGS/PLANCK;
      V(i_row, i_col) *= 220000;
      if ((i_col-i_row)%2)
        T(i_row, i_col) *= -1.0;
      if (i_row == i_col)
        T(i_row, i_col) *= M_PI*M_PI/3.0;
      else
        T(i_row, i_col) *= 2.0/(i_col-i_row)/(i_col-i_row);
    }
  }
  H = T+V;
  arma::rowvec eigenvalues = arma::eig_sym(H);

  std::ofstream output_file("freqs.dat");
  output_file << eigenvalues(1) - eigenvalues(0);
}
