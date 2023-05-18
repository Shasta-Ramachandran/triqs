/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2012 by M. Ferrero, O. Parcollet
 *
 * TRIQS is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * TRIQS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * TRIQS. If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#pragma once
#include "utils.hpp"
#include "dlr_coeffs.hpp"
//#include "domains/matsubara.hpp"

#include <cppdlr/cppdlr.hpp>

#include <memory>

namespace triqs::mesh {

  struct dlr_imfreq;
  struct dlr_coeffs;

  // OPFIXME : the default constructed is not usable e.g. size() crashes ....
  // in particular one can not iterate ...

  // Why 3 classes? Why not dlr<Time | Coeffs| Freq> template on an enum ?
  // with user aliases dlr_imtime = dlr<Time> ...

  // OPFIXME public field : Invariant Violation possible.
  // Change beta, the _dlr not change, ....
  // Make private, and get_beta(), get_Lambda(), get_stat(), get_eps()

  struct dlr_imtime {

    using idx_t    = long;
    using datidx_t = long;
    using value_t  = double;

    // -------------------- Data -------------------

    double beta              = 0.0;
    statistic_enum statistic = Fermion;
    double Lambda            = 1e+10;
    double eps               = 1e-10;

    private:
    uint64_t mesh_hash_ = 0;
    std::shared_ptr<const dlr_ops> _dlr;

    // -------------------- Constructors -------------------
    public:
    dlr_imtime() = default;

    /**
     * Construct a DLR mesh in imaginary times
     *
     * @param beta Inverse temperature
     * @param statistic, Fermion or Boson
     * @param Lambda Lambda energy over beta parameter
     * @param eps Representation accuracy
     */
    dlr_imtime(double beta, statistic_enum statistic, double Lambda, double eps)
       : dlr_imtime(beta, statistic, Lambda, eps, cppdlr::build_dlr_rf(Lambda, eps)) {}

    private:
    dlr_imtime(double beta, statistic_enum statistic, double Lambda, double eps, nda::vector<double> const &dlr_freq)
       : dlr_imtime(beta, statistic, Lambda, eps,
                    dlr_ops{dlr_freq, {Lambda, dlr_freq}, {Lambda, dlr_freq, static_cast<cppdlr::statistic_t>(statistic)}}) {}

    dlr_imtime(double beta, statistic_enum statistic, double Lambda, double eps, dlr_ops dlr)
       : beta(beta),
         statistic(statistic),
         Lambda(Lambda),
         eps(eps),
         mesh_hash_(hash(beta, Lambda, eps, sum(dlr.imt.get_itnodes()))),
         _dlr{std::make_shared<dlr_ops>(std::move(dlr))} {}

    friend class dlr_imfreq;
    friend class dlr_coeffs;

    public:
    template <any_of<dlr_imtime, dlr_imfreq, dlr_coeffs> M>
    explicit dlr_imtime(M const &m) : beta(m.beta), statistic(m.statistic), Lambda(m.Lambda), eps(m.eps), _dlr(m._dlr) {
      if constexpr (std::is_same_v<M, dlr_imtime>)
        mesh_hash_ = m.mesh_hash_;
      else
        mesh_hash_ = hash(beta, Lambda, eps, sum(_dlr->imt.get_itnodes()));
    }

    // -------------------- Comparisons -------------------

    bool operator==(dlr_imtime const &m) const { return mesh_hash_ == m.mesh_hash_ and statistic == m.statistic; }
    bool operator!=(dlr_imtime const &m) const { return !(operator==(m)); }

    // -------------------- mesh_point -------------------

    struct mesh_point_t {
      using mesh_t       = dlr_imtime;
      long idx           = 0;
      long datidx        = 0;
      uint64_t mesh_hash = 0;
      double val         = {};
      [[nodiscard]] double value() const { return val; }
      operator double() const { return val; }
    };

    // -------------------- Accessors -------------------

    [[nodiscard]] auto const &dlr_freq() const { return _dlr->freq; }

    [[nodiscard]] auto const &dlr_it() const { return _dlr->imt; }

    [[nodiscard]] auto const &dlr_if() const { return _dlr->imf; }

    [[nodiscard]] uint64_t mesh_hash() const noexcept { return mesh_hash_; }

    /// The total number of points in the mesh
    [[nodiscard]] long size() const noexcept { return _dlr->freq.size(); }

    // -------------------- idx checks and conversions -------------------

    [[nodiscard]] bool is_idx_valid(idx_t idx) const noexcept { return 0 <= idx and idx < size(); }

    [[nodiscard]] datidx_t to_datidx(idx_t idx) const noexcept {
      EXPECTS(is_idx_valid(idx));
      return idx;
    }

    // there is no to_datidx for a closest mesh point, as it does not make sense here.
    [[nodiscard]] datidx_t
    to_datidx(closest_mesh_point_t<double> const &cmp) const = delete; // closest_mesh_point makes no sense for a dlr_imtime mesh

    [[nodiscard]] idx_t to_idx(long datidx) const noexcept {
      EXPECTS(is_idx_valid(datidx));
      return datidx;
    }

    // -------------------- operator [] () -------------------

    [[nodiscard]] mesh_point_t operator[](long datidx) const { return (*this)(datidx); }

    [[nodiscard]] mesh_point_t operator()(idx_t idx) const {
      EXPECTS(is_idx_valid(idx));
      return {idx, idx, mesh_hash_, to_value(idx)};
    }

    // -------------------- to_value ------------------

    [[nodiscard]] double to_value(idx_t idx) const noexcept {
      EXPECTS(is_idx_valid(idx));
      auto res = _dlr->imt.get_itnodes()[idx] * beta;
      if (res < 0) res = beta + res; // OPFIXME how can it become <0 ??
      return res;
    }

    // -------------------------- Range & Iteration --------------------------

    private:
    [[nodiscard]] auto r_() const {
      return itertools::transform(range(size()), [this](long i) { return (*this)[i]; });
    }

    public:
    [[nodiscard]] auto begin() const { return r_().begin(); }
    [[nodiscard]] auto cbegin() const { return r_().cbegin(); }
    [[nodiscard]] auto end() const { return r_().end(); }
    [[nodiscard]] auto cend() const { return r_().cend(); }

    // -------------------- print  -------------------

    friend std::ostream &operator<<(std::ostream &sout, dlr_imtime const &m) {
      auto stat_cstr = (m.statistic == Boson ? "Boson" : "Fermion");
      return sout << fmt::format("DLR imtime mesh of size {} with beta = {}, statistic = {}, Lambda = {}, eps = {}", m.beta, m.size(), stat_cstr,
                                 m.Lambda, m.eps);
    }

    // -------------------- HDF5 -------------------

    [[nodiscard]] static std::string hdf5_format() { return "MeshDLRImTime"; }

    /// Write into HDF5
    friend void h5_write(h5::group fg, std::string const &subgroup_name, dlr_imtime const &m) {
      h5::group gr = fg.create_group(subgroup_name);
      write_hdf5_format(gr, m); //NOLINT

      h5::write(gr, "beta", m.beta);
      h5::write(gr, "statistic", (m.statistic == Fermion ? "F" : "B"));
      h5::write(gr, "Lambda", m.Lambda);
      h5::write(gr, "eps", m.eps);
      h5::write(gr, "dlr_freq", m.dlr_freq());
      h5::write(gr, "dlr_it", m.dlr_it());
      h5::write(gr, "dlr_if", m.dlr_if());
    }

    /// Read from HDF5
    friend void h5_read(h5::group fg, std::string const &subgroup_name, dlr_imtime &m) {
      h5::group gr = fg.open_group(subgroup_name);
      assert_hdf5_format(gr, m, true);

      auto beta      = h5::read<double>(gr, "beta");
      auto statistic = (h5::read<std::string>(gr, "statistic") == "F" ? Fermion : Boson);
      auto Lambda    = h5::read<double>(gr, "Lambda");
      auto eps       = h5::read<double>(gr, "eps");
      auto _dlr_freq = h5::read<nda::vector<double>>(gr, "dlr_freq");
      auto _dlr_it   = h5::read<cppdlr::imtime_ops>(gr, "dlr_it");
      auto _dlr_if   = h5::read<cppdlr::imfreq_ops>(gr, "dlr_if");
      m              = dlr_imtime(beta, statistic, Lambda, eps, {_dlr_freq, _dlr_it, _dlr_if});
    }
  };

  double evaluate(dlr_imtime const &m, ...) = delete;

  // check concept
  static_assert(MeshWithValues<dlr_imtime>);

} // namespace triqs::mesh
