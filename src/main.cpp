#include "AnalysisTree/Chain.hpp"
#include "AnalysisTree/Matching.hpp"
#include <SimpleCut.hpp>
#include <TF1.h>
#include <TF2.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2D.h>
#include <TThread.h>
#include <algorithm>
#include <iostream>
#include <string>

namespace GAUSPID {
namespace at = AnalysisTree;

class Fit1D {
public:
  Fit1D(const std::vector<int> pdg, const float p_min, const float p_max)
      : _p_min{p_min}, _p_max{p_max} {
    auto name = std::to_string(pdg[0]) + "hist_" + std::to_string(p_min) + "_" +
                std::to_string(p_max);
    _hist_data = new TH1F(name.c_str(), name.c_str(), 200, -1, 2);
    auto fit_name =
        "fit_" + std::to_string(_p_min) + "_" + std::to_string(_p_max);
    _fit = new TF1(fit_name.c_str(), "gaus", -1, 2);
  }

  const float GetPMin() const { return _p_min; }

  const float GetPMax() const { return _p_max; }

  const TF1 *GetFitFunc() const { return _fit; }

  void FillHist(const float p, const float mass2) {
    if (p > _p_min && p < _p_max) {
      _hist_data->Fill(mass2);
    }
  }

  TF1 *Fit() {
    _hist_data->Fit(_fit, "WWS", "");
    return _fit;
  }

  void WriteHist() {
    _hist_data->Write();
    // _fit->Write();
  }

private:
  TH1F *_hist_data;
  TF1 *_fit;

  const float _p_min;
  const float _p_max;
};

class Fit2D {
public:
  Fit2D(const std::vector<int> pdg, const float p_min, const float p_max,
        const unsigned int n_bins, const std::string filename)
      : _pdg{pdg}, _p_min{p_min}, _p_max{p_max}, _n_bins{n_bins},
        _filename{filename} {
    float delta = (p_max - p_min) / n_bins;
    for (float min = 0; min < _p_max; min += delta) {
      _fits.push_back(Fit1D(_pdg, min, min + delta));
    }
  }

  void FillHists() {
    auto chain = new at::Chain(std::vector<std::string>({_filename}),
                               std::vector<std::string>({"rTree"}));
    chain->InitPointersToBranches({"VtxTracks", "TofHits"});

    auto *config = chain->GetConfiguration();
    auto *data_header = chain->GetDataHeader();

    data_header->Print();
    config->Print();

    auto vtx_tracks = chain->GetBranchObject("VtxTracks");
    auto tof_hits = chain->GetBranchObject("TofHits");
    auto vtx2tof_match = chain->GetMatching("VtxTracks", "TofHits");

    auto p_vtx = vtx_tracks.GetField("p");
    auto mc_pdg_vtx = vtx_tracks.GetField("mc_pdg");
    auto qp_tof = tof_hits.GetField("qp_tof");
    auto mass2_tof = tof_hits.GetField("mass2");

    for (long i_event = 0; i_event < chain->GetEntries(); ++i_event) {
      chain->GetEntry(i_event);
      for (size_t i = 0; i < vtx_tracks.size(); ++i) {
        auto mc_pdg = vtx_tracks[i][mc_pdg_vtx];
        const auto matched_track_tof_id = vtx2tof_match->GetMatch(i);
        if (matched_track_tof_id > 0) {
          auto qp = tof_hits[matched_track_tof_id][qp_tof];
          auto mass2 = tof_hits[matched_track_tof_id][mass2_tof];
          if (std::find(_pdg.begin(), _pdg.end(), mc_pdg) != _pdg.end()) {
            for (auto &fit : _fits) {
              fit.FillHist(qp, mass2);
            }
          }
        }
      }
    }
  }

  void FitHists() {
    for (auto fit : _fits) {
      fit.Fit();
    }
  }

  void WriteHists() {
    for (auto fit : _fits) {
      fit.WriteHist();
    }
    _fit2d->Write();
  }

  TF2 *ConcatenateFits() {
    auto name = std::to_string(_pdg[0]) + "_2d_fit";
    _fit2d = new TF2(
        name.c_str(),
        [this](Double_t *x, Double_t *p) {
          for (auto &fit : this->_fits) {
            if (x[0] > fit.GetPMin() && x[0] <= fit.GetPMax()) {
              return fit.GetFitFunc()->Eval(x[1]);
            }
          }
          return 0.;
        },
        this->_p_min, this->_p_max, -1., 2., 0);
    return _fit2d;
  }

private:
  std::vector<Fit1D> _fits;
  TF2 *_fit2d;
  const float _p_min;
  const float _p_max;
  const std::vector<int> _pdg;
  const unsigned int _n_bins;
  const std::string _filename;
};
} // namespace GAUSPID

int main() {
  const auto filelist_path = "filelist.txt";
  const int nbins = 10;
  const float p_min = 0;
  const float p_max = 6;
  const std::vector<int> proton_pdg = {2212};
  const std::vector<int> kaon_pdg = {321};
  const std::vector<int> pion_pdg = {13, 211, 11};
  const std::array<std::vector<int>, 3> pdgs = {proton_pdg, kaon_pdg, pion_pdg};

  std::vector<GAUSPID::Fit2D> fits;
  for (auto &pdg : pdgs) {
    fits.push_back(GAUSPID::Fit2D(pdg, p_min, p_max, nbins, filelist_path));
  }

  std::cout << "Filling histograms..." << std::endl;
  for (auto &fit : fits) {
    fit.FillHists();
  }

  std::cout << "Fitting histograms..." << std::endl;
  for (auto &fit : fits) {
    fit.FitHists();
    fit.ConcatenateFits();
  }

  TFile *out_file = TFile::Open("gauss_out.root", "recreate");

  std::cout << "Writing to file..." << std::endl;
  for (auto &fit : fits) {
    fit.WriteHists();
  }

  std::cout << "Done." << std::endl;
  out_file->Close();
  return 0;
}
