// Copyright (c) 2019-2023 Simons Foundation
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You may obtain a copy of the License at
//     https://www.gnu.org/licenses/gpl-3.0.txt
//
// Authors: Olivier Parcollet, Nils Wentzell

#include <triqs/gfs.hpp>

using namespace triqs::gfs;
using namespace triqs;

triqs::clef::placeholder<0> iw_;
triqs::clef::placeholder<1> inu_;

int nw      = 3;
double beta = 20;

int main() {

  // Meshes
  auto iw_mesh     = mesh::imfreq{beta, Fermion, nw};
  auto iw_inu_mesh = iw_mesh * iw_mesh;

  // -- Product Gf
  auto Gp  = gf{iw_inu_mesh, {1, 1}};
  auto Gp1 = gf{Gp};

  // Before commit 68b1d85e the following line was creating a copy
  // of the underlying storage which was immediately freed in the slicing process
  // This lead to Heap use after free (ASAN)

  std::get<0>(iw_inu_mesh);
  std::ignore = std::begin(iw_inu_mesh);
  std::get<0>(*std::begin(iw_inu_mesh));
  for (auto mp : iw_inu_mesh) {
    std::get<0>(mp);
    std::get<1>(mp);
    auto mp2 = mp;
    std::get<0>(mp2);
    //std::apply([](auto const &x, auto const &) { return x; }, mp);
    std::apply([](auto const &x, auto const &) { return x; }, mp2);
  }
  //Gp1(iw_, inu_) << Gp(iw_, inu_)(0, 0);
}
