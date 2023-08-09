// Copyright (c) 2023 Simons Foundation
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
// Authors: Dominik Kiese, Nils Wentzell

#pragma once
#include <triqs/gfs.hpp>
#include <nda/nda.hpp>
#include <nda/sym_grp.hpp>

namespace triqs {
  namespace gfs {

    // symmetry concept for scalar valued gf: symmetry accepts mesh index and returns new mesh index & operation
    template <typename F, typename G, typename mesh_index_t = typename G::mesh_t::index_t>
    concept ScalarGfSymmetry = is_gf_v<G> and //
       requires(F f, mesh_index_t const &mesh_index) {
         requires(G::target_rank == 0);
         { f(mesh_index) } -> std::same_as<std::tuple<mesh_index_t, nda::operation>>;
       };

    // symmetry concept for tensor valued gf: symmetry accepts mesh + target index and returns new mesh + target index & operation
    template <typename F, typename G, typename mesh_index_t = typename G::mesh_t::index_t,
              typename target_index_t = std::array<long, static_cast<std::size_t>(G::target_rank)>>
    concept TensorGfSymmetry = is_gf_v<G> and //
       requires(F f, mesh_index_t const &mesh_index, target_index_t const &target_index) {
         requires(G::target_rank > 0);
         { f(mesh_index, target_index) } -> std::same_as<std::tuple<mesh_index_t, target_index_t, nda::operation>>;
       };

    // init function concept for scalar valued gf: init function accepts mesh index and returns gf value type
    template <typename F, typename G, typename mesh_index_t = typename G::mesh_t::index_t>
    concept ScalarGfInitFunc = is_gf_v<G> and //
       requires(F f, mesh_index_t const &mesh_index) {
         requires(G::target_rank == 0);
         { f(mesh_index) } -> std::same_as<typename G::scalar_t>;
       };

    // init function concept for tensor valued gf: init function accepts mesh index + target index and returns gf value type
    template <typename F, typename G, typename mesh_index_t = typename G::mesh_t::index_t,
              typename target_index_t = std::array<long, static_cast<std::size_t>(G::target_rank)>>
    concept TensorGfInitFunc = is_gf_v<G> and //
       requires(F f, mesh_index_t const &mesh_index, target_index_t const &target_index) {
         requires(G::target_rank > 0);
         { f(mesh_index, target_index) } -> std::same_as<typename G::scalar_t>;
       };

    // make tuple from array
    template <typename A> auto to_tuple(A const &arr) {
      constexpr auto fetch = [](auto const &...xs) { return std::tuple{xs...}; };
      return std::apply(fetch, arr);
    }

    // make tuple from array using index_sequence
    template <std::size_t R, typename A> auto to_tuple(A const &arr) {
      return [&]<std::size_t... Is>(std::index_sequence<Is...>) { return std::tuple{arr[Is]...}; }(std::make_index_sequence<R>{});
    }

    // make array from tuple
    template <typename tpl_t> auto to_array(tpl_t const &tpl) {
      constexpr auto fetch = [](auto const &...xs) { return std::array{xs...}; };
      return std::apply(fetch, tpl);
    }

    template <typename F, typename G>
      requires(is_gf_v<G> && (ScalarGfSymmetry<F, G> || TensorGfSymmetry<F, G>))
    class sym_grp {

      private:
      // data aliases
      using data_t           = typename G::data_t;
      using value_t          = typename G::scalar_t;
      using data_index_t     = std::array<long, static_cast<std::size_t>(nda::get_rank<data_t>)>;
      using data_sym_func_t  = std::function<std::tuple<data_index_t, nda::operation>(data_index_t const &)>;
      using data_init_func_t = std::function<value_t(data_index_t const &)>;

      // mesh aliases
      using mesh_index_t              = typename G::mesh_t::index_t;
      static constexpr auto mesh_rank = n_variables<typename G::mesh_t>;

      // target aliases
      static constexpr size_t target_rank = G::target_rank;
      using target_index_t                = std::array<long, static_cast<std::size_t>(target_rank)>;

      // members
      nda::sym_grp<data_sym_func_t, data_t> data_sym_grp; // symmetry group instance for the data array

      // convert from gf to nda symmetry
      data_sym_func_t to_data_symmetry(F const &f, G const &g) const {

        auto fp = [f, m = g.mesh()](data_index_t const &x) -> std::tuple<data_index_t, nda::operation> {
          // init new data index and residual operation
          data_index_t xp;

          if constexpr (target_rank == 0) { // scalar valued gfs

            if constexpr (mesh_rank == 1) {
              auto [new_mesh_index, op] = f(m.to_index(x[0]));
              xp[0]                     = m.to_data_index(new_mesh_index);
              return {xp, op};

            } else { // product mesh
              auto [new_mesh_index, op] = f(m.to_index(to_tuple(x)));
              xp                        = to_array(m.to_data_index(new_mesh_index));
              return {xp, op};
            }

          } else { // tensor valued gfs

            // convert data index to target index
            target_index_t target_index;
            for (auto i : range(target_rank)) target_index[i] = x[i + mesh_rank];

            if constexpr (mesh_rank == 1) {
              // evaluate symmetry
              auto [new_mesh_index, new_target_index, op] = f(m.to_index(x[0]), target_index);

              // convert mesh index + target index back to data index
              xp[0] = m.to_data_index(new_mesh_index);
              for (auto i : range(target_rank)) xp[i + mesh_rank] = new_target_index[i];

              return {xp, op};

            } else { // product mesh
              // evaluate symmetry
              auto [new_mesh_index, new_target_index, op] = f(m.to_index(to_tuple<mesh_rank>(x)), target_index);

              // convert mesh index + target index back to data index
              auto new_mesh_arr = to_array(m.to_data_index(new_mesh_index));
              for (auto i : range(mesh_rank)) xp[i] = new_mesh_arr[i];
              for (auto i : range(target_rank)) xp[i + mesh_rank] = new_target_index[i];

              return {xp, op};
            }
          }
        };

        return fp;
      };

      // convert from list of gf symmetries to list of nda symmetries
      std::vector<data_sym_func_t> to_data_symmetry_list(G const &g, std::vector<F> const &sym_list) const {
        std::vector<data_sym_func_t> data_sym_list;
        for (auto f : sym_list) data_sym_list.push_back(to_data_symmetry(f, g));
        return data_sym_list;
      }

      // convert from gf to nda init function
      template <typename H> data_init_func_t to_data_init_func(G const &g, H const &h) const {

        auto hp = [h, m = g.mesh()](data_index_t const &x) {
          if constexpr (target_rank == 0) { // scalar valued gfs

            if constexpr (mesh_rank == 1) {
              return h(m.to_index(x[0]));

            } else { // product mesh
              return h(m.to_index(to_tuple(x)));
            }

          } else { // tensor valued gfs

            target_index_t target_index;
            for (auto i : range(target_rank)) target_index[i] = x[i + mesh_rank];

            if constexpr (mesh_rank == 1) {
              return h(m.to_index(x[0]), target_index);

            } else { // product mesh
              return h(m.to_index(to_tuple<mesh_rank>(x)), target_index);
            }
          }
        };

        return hp;
      };

      public:
      [[nodiscard]] nda::sym_grp<data_sym_func_t, data_t> const &get_data_sym_grp() const { return data_sym_grp; }
      [[nodiscard]] long num_classes() const { return data_sym_grp.num_classes(); }

      sym_grp() = default;
      sym_grp(G const &g, std::vector<F> const &sym_list, long const max_length = 0)
         : data_sym_grp{g.data(), to_data_symmetry_list(g, sym_list), max_length} {};

      // initializer method
      template <typename H>
      void init(G &g, H const &h, bool parallel = false) const
        requires(ScalarGfInitFunc<H, G> || TensorGfInitFunc<H, G>)
      {
        data_sym_grp.init(g.data(), to_data_init_func(g, h), parallel);
      }

      // symmetrization method
      std::tuple<double, mesh_index_t, target_index_t> symmetrize(G &g) const {
        auto const &[max_diff, max_index] = data_sym_grp.symmetrize(g.data());
        auto const m                      = g.mesh();

        if constexpr (target_rank == 0) { // scalar valued gfs

          if constexpr (mesh_rank == 1) {
            return {max_diff, m.to_index(max_index[0]), {}};

          } else { // product mesh
            return {max_diff, m.to_index(to_tuple(max_index)), {}};
          }

        } else { // tensor valued gfs

          // convert data index to target index
          target_index_t target_index;
          for (auto i : range(target_rank)) target_index[i] = max_index[i + mesh_rank];

          if constexpr (mesh_rank == 1) {
            return {max_diff, m.to_index(max_index[0]), target_index};

          } else { // product mesh
            return {max_diff, m.to_index(to_tuple<mesh_rank>(max_index)), target_index};
          }
        }
      }
    };
  } // namespace gfs
} // namespace triqs
