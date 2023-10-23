#include "GAUSPIDFit2D.hpp"

#include "name_helpers.hpp"

namespace GAUSPID
{
    Fit2D::Fit2D(
        const std::vector<int> pdg,
        const float p_min,
        const float p_max,
        const unsigned int n_bins,
        const std::string filename) :
        _p_min{p_min}, _p_max{p_max}, _pdg{pdg}, _n_bins{n_bins}, _filename{filename}
    {
        float delta = (p_max - p_min) / n_bins;
        for(float min = 0; min < _p_max; min += delta)
        {
            _fits.push_back(Fit1D(_pdg, min, min + delta));
        }
    }

    void Fit2D::FillHists()
    {
        namespace at = AnalysisTree;
        auto chain = new at::Chain(
            std::vector<std::string>({_filename}), std::vector<std::string>({"rTree"}));
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
                    if(std::find(_pdg.begin(), _pdg.end(), mc_pdg) != _pdg.end())
                    {
                        for(auto& fit: _fits)
                        {
                            fit.FillHist(qp, mass2);
                        }
                    }
                }
            }
        }
    }

    void Fit2D::FitHists()
    {
        for(auto fit: _fits)
        {
            fit.Fit();
        }
    }

    void Fit2D::WriteHists()
    {
        for(auto fit: _fits)
        {
            fit.WriteHist();
        }
        _fit2d->SetNpx(200);
        _fit2d->SetNpy(200);
        _fit2d->Write();
    }

    TF2* Fit2D::ConcatenateFits()
    {
        auto fit_lambda = [this](Double_t* x, Double_t* p)
        {
            for(auto& fit: this->_fits)
            {
                if(x[0] > fit.GetPMin() && x[0] <= fit.GetPMax())
                {
                    return fit.GetFitFunc()->Eval(x[1]);
                }
            }
            return 0.;
        };

        auto name = name_helpers::create_2d_fit_name(_pdg);
        _fit2d = new TF2(name.c_str(), fit_lambda, this->_p_min, this->_p_max, -1., 2., 0);
        
        auto fit2dtitle = name_helpers::create_2d_fit_title(_pdg);
        _fit2d->SetTitle(fit2dtitle.c_str());
        return _fit2d;
    }
}
