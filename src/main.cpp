#include "AnalysisTree/Chain.hpp"
#include "AnalysisTree/Matching.hpp"
#include <SimpleCut.hpp>
#include <TF1.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2D.h>
#include <TThread.h>
#include <iostream>
#include <string>

namespace GAUSPID {
namespace at = AnalysisTree;

class Fit1D {
public:
  Fit1D(const float p_min, const float p_max) : _p_min{p_min}, _p_max{p_max} {
    auto name = "hist_" + std::to_string(p_min) + "_" + std::to_string(p_max);
    _hist_data = new TH1F(name.c_str(), name.c_str(), 200, -1, 2);
    auto fit_name = "fit_" + std::to_string(_p_min) + "_" + std::to_string(_p_max);
    _fit = new TF1(fit_name.c_str(), "gaus", -1, 2);
  }


  void FillHist(const float p, const float mass2) {
    if (p > _p_min && p < _p_max) {
      _hist_data->Fill(mass2);
    }
  }

  TF1 *Fit() {
    _hist_data->Fit(_fit,"gaus", "WS");
    return _fit;
  }

  void WriteHist() {
    _hist_data->Write();
    _fit->Write();
  }

private:
  TH1F *_hist_data;
  TF1 *_fit;

  const float _p_min;
  const float _p_max;
};

class Fit2D {
public:
  Fit2D(const int pdg, const float p_min, const float p_max,
        const unsigned int n_bins, const std::string filename)
      : _pdg{pdg}, _p_min{p_min}, _p_max{p_max}, _n_bins{n_bins},
        _filename{filename} {
    float delta = (p_max - p_min) / n_bins;
    for (float min = 0; min < _p_max; min += delta) {
      _fits.push_back(Fit1D(min, min + delta));
    }
  }

  void FillHists() {
    auto chain = new at::Chain(_filename, "rTree");
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
          if (mc_pdg == _pdg) {
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
  }

  TH2F ConcatenateFits();

private:
  std::vector<Fit1D> _fits;
  const float _p_min;
  const float _p_max;
  const int _pdg;
  const unsigned int _n_bins;
  const std::string _filename;
};
} // namespace GAUSPID

int main() {
  std::cout << "\nhello, world!\n" << std::endl;
  auto fit2d = GAUSPID::Fit2D(2212, 0, 6, 5, "prediction_tree_little.root");
  fit2d.FillHists();
  fit2d.FitHists();
  TFile *out_file = TFile::Open("gauss_out.root", "recreate");
  fit2d.WriteHists();
  out_file->Close();
  return 0;
}
