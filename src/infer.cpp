#include <algorithm>
#include <iostream>
#include <string>
#include <numeric>
#include <utility>
#include <AnalysisTree/Chain.hpp>
#include <AnalysisTree/Matching.hpp>
#include <TF2.h>
#include <TFile.h>
#include <TH2F.h>
#include "src/name_helpers.hpp"

namespace GAUSPID
{
    class ParticleFit
    {
    public:
        ParticleFit(TFile* hist_file, std::vector<int> pdg) : _pdg{pdg}
        {
            std::string hist_name = name_helpers::create_2d_fit_name(pdg);
            _fit = LoadFit(hist_file, hist_name);
            std::cout << _fit->GetTitle() << std::endl;
            auto inferred_hist_name = name_helpers::create_2d_inferred_name(pdg);
            auto inferred_hist_title = name_helpers::create_2d_inferred_title(pdg);
            _hist = new TH2F(
                inferred_hist_name.c_str(), inferred_hist_title.c_str(), 200, 0, 6, 200, -1, 2);

            auto match_hist_name = inferred_hist_name + "match";
            auto match_hist_title = "matched " + inferred_hist_title;
            _hist_match = new TH2F(
                match_hist_name.c_str(), match_hist_title.c_str(), 200, 0, 6, 200, -1, 2);

            auto mismatch_hist_name = inferred_hist_name + "mismatch";
            auto mismatch_hist_title = "mismatched " + inferred_hist_title;
            _hist_mismatch = new TH2F(
                mismatch_hist_name.c_str(), mismatch_hist_title.c_str(), 200, 0, 6, 200, -1, 2);

            auto mc_true_hist_name = inferred_hist_name + "mc-true";
            auto mc_true_hist_title = "mc-true " + inferred_hist_title;
            _hist_mc_true = new TH2F(
                mc_true_hist_name.c_str(), mc_true_hist_title.c_str(), 200, 0, 6, 200, -1, 2);
        }

        float Eval(float p, float m2, int mc_pdg)
        {
            if(std::find(_pdg.begin(), _pdg.end(), mc_pdg) != _pdg.end())
            {
              _hist_mc_true->Fill(p,m2);
            }
            return _fit->Eval(p, m2);
        }

        void Fill(float p, float m2, int mc_pdg)
        {
            _hist->Fill(p, m2);
            if(std::find(_pdg.begin(), _pdg.end(), mc_pdg) != _pdg.end())
            {
              _hist_match->Fill(p,m2);
            }
            else 
            {
              _hist_mismatch->Fill(p,m2);
            }
        }

        void Write()
        {
            _hist->Write();
        _hist_match->Write();
        _hist_mismatch->Write();
        _hist_mc_true->Write();
        }

        void PrintStats()
        {
          auto n_classified = _hist_match->GetEntries();
          auto n_match = _hist_match->GetEntries();
          auto n_total = _hist_match->GetEntries();
          auto efficiency = n_match/n_total;
          auto purity = n_match/n_classified;
          std::string pdg_str = "";
          for (auto& pdg : _pdg)
          {
            pdg_str = pdg_str + "/" + std::to_string(pdg);
          }

          std::cout << std::endl << "Particle pdg: " << pdg_str << std::endl;
        }

        inline std::vector<int> GetPdg() const
        {
            return _pdg;
        }

    private:
        TF2* LoadFit(TFile* hist_file, std::string hist_name)
        {
            TF2* fit;
            hist_file->GetObject(hist_name.c_str(), fit);
            return fit;
        }

        TF2* _fit;
        TH2F* _hist;
        TH2F* _hist_match;
        TH2F* _hist_mismatch;
        TH2F* _hist_mc_true;
        std::vector<int> _pdg;
    };

    class Inferrer
    {
    public:
        Inferrer(std::string hist_file_path, std::vector<std::vector<int>> pdgs, const float purity_cut = 0.9) :
            _purity_cut{purity_cut}
        {
            auto hist_file = new TFile(hist_file_path.c_str(), "READ");
            for(auto& pdg: pdgs)
            {
                _classes.push_back(ParticleFit(hist_file, pdg));
            }

            auto bg_hist_name =
                name_helpers::create_2d_inferred_name("background");
            auto bg_hist_title =
                name_helpers::create_2d_inferred_title("background");
            _bg_hist = new TH2F(
                bg_hist_name.c_str(), bg_hist_title.c_str(), 200, 0, 6, 200, -1, 2);
        }

        std::vector<int> DeduceType(float p, float m2, int mc_pdg)
        {
            std::map<ParticleFit*, float> prob_map;
            for(auto& c: _classes)
            {
                prob_map.insert(std::make_pair(&c, c.Eval(p, m2, mc_pdg)));
            }

            auto most_likely_particle = *std::max_element(
                std::begin(prob_map),
                std::end(prob_map),
                [](std::pair<ParticleFit*, float> a, std::pair<ParticleFit*, float> b)
                { return a.second < b.second; });
            const float all_particle_counts = std::accumulate(
                std::begin(prob_map),
                std::end(prob_map),
                0,
                [](const float previous, const std::pair<ParticleFit*, float> p)
                { return previous + p.second; });
            if(most_likely_particle.second / all_particle_counts > _purity_cut)
            {
                most_likely_particle.first->Fill(p, m2, mc_pdg);
                return most_likely_particle.first->GetPdg();
            }
            _bg_hist->Fill(p, m2);
            return {0};
        }

        void WriteHistograms()
        {
            for(auto c: _classes)
            {
                c.Write();
            }
            _bg_hist->Write();
        }

    private:
        std::vector<ParticleFit> _classes;
        std::vector<TH2F*> _histograms;
        TH2F* _bg_hist;
        const float _purity_cut;
    };

}

int main()
{
    const std::vector<int> proton_pdg = {2212};
    const std::vector<int> kaon_pdg = {321};
    const std::vector<int> pion_pdg = {13, 211, 11};
    const std::vector<std::vector<int>> pdgs = {proton_pdg, kaon_pdg, pion_pdg};

    const std::string filelist_path = "filelist.txt";
    const std::string outfile_path = "gauss_inferred.root";

    auto inferrer = new GAUSPID::Inferrer("gauss_out.root", pdgs);

    namespace at = AnalysisTree;
    auto chain = new at::Chain(
        std::vector<std::string>({filelist_path}), std::vector<std::string>({"rTree"}));
    chain->InitPointersToBranches({"VtxTracks", "TofHits"});

    auto* config = chain->GetConfiguration();
    auto* data_header = chain->GetDataHeader();

    data_header->Print();
    config->Print();

    auto vtx_tracks = chain->GetBranchObject("VtxTracks");
    auto tof_hits = chain->GetBranchObject("TofHits");
    auto vtx2tof_match = chain->GetMatching("VtxTracks", "TofHits");

    auto p_vtx = vtx_tracks.GetField("p");
    auto mc_pdg_vtx = vtx_tracks.GetField("mc_pdg");
    auto qp_tof = tof_hits.GetField("qp_tof");
    auto mass2_tof = tof_hits.GetField("mass2");

    for(long i_event = 0; i_event < chain->GetEntries(); ++i_event)
    {
        chain->GetEntry(i_event);
        for(size_t i = 0; i < vtx_tracks.size(); ++i)
        {
            auto mc_pdg = vtx_tracks[i][mc_pdg_vtx];
            const auto matched_track_tof_id = vtx2tof_match->GetMatch(i);
            if(matched_track_tof_id > 0)
            {
                auto qp = tof_hits[matched_track_tof_id][qp_tof];
                auto mass2 = tof_hits[matched_track_tof_id][mass2_tof];
                inferrer->DeduceType(qp, mass2, mc_pdg);
            }
        }
    }
    TFile* out_file = TFile::Open(outfile_path.c_str(), "recreate");
    inferrer->WriteHistograms();
    out_file->Close();
    return 0;
}
