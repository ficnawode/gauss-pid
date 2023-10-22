#include "AnalysisTree/Chain.hpp"
#include <SimpleCut.hpp>
#include <TF1.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <iostream>

namespace GAUSPID {
namespace at = AnalysisTree;

class Fit1D {
public:
  Fit1D(const float p_min, const float p_max) : _p_min{p_min}, _p_max{p_max} {}

  void FillHist(const float p, const float mass2) {
    if (p > _p_min && p < _p_max) {
      _hist_data->Fill(mass2);
    }
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
      _fits.push_back(Fit1D(_p_min, _p_max));
    }
  }

  void FillHists() {
    auto chain = new at::Chain(_filename, "rTree");
    chain->InitPointersToBranches({"VtxTracks", "TofHits"});

    auto *config = chain->GetConfiguration();
    auto *data_header = chain->GetDataHeader();

    data_header->Print();
    config->Print();

    auto vtx_tracks = new at::Branch(chain->GetBranchObject("VtxTracks"));
    auto tof_hits = new at::Branch(chain->GetBranchObject("TofHits"));
    auto vtx2tof_particles = chain->GetMatching("VtxTracks", "TofHits");

    auto mc_pdg_vtx = vtx_tracks->GetField("mc_pdg");
    auto qp_tof = tof_hits->GetField("qp_tof");
    auto mass2_tof = tof_hits->GetField("mass2");

    for (long i_event = 0; i_event < chain->GetEntries(); ++i_event) {
      chain->GetEntry(i_event);
      for (size_t i = 0; i < vtx_tracks->size(); ++i) {
        auto matched_id = vtx2tof_particles->
      }
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
  auto fit2d = GAUSPID::Fit2D(2212, 0, 6, 1, "prediction_tree_little.root");
  return 0;
}
