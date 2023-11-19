#include "GAUSPIDFit1D.hpp"

#include <string>

#include "name_helpers.hpp"

namespace GAUSPID
{
    Fit1D::Fit1D(const std::vector<int> pdg, const float p_min, const float p_max, const float m2_min, const float m2_max) :
        _p_min{p_min}, _p_max{p_max}
    {
        auto hist_name = name_helpers::create_1d_hist_name(pdg, p_min, p_max);
        auto hist_title = name_helpers::create_1d_fit_title(pdg, p_min, p_max);
        _hist = new TH1F(hist_name.c_str(), hist_title.c_str(), 400, m2_min, m2_max);

        auto fit_name = name_helpers::create_1d_fit_name(pdg, p_min, p_max);
        _fit = new TF1(fit_name.c_str(), "gaus", m2_min, m2_max);
    }

    void Fit1D::FillHist(const float p, const float mass2)
    {
        if(p > _p_min && p <= _p_max)
        {
            _hist->Fill(mass2);
        }
    }

    TF1* Fit1D::Fit()
    {
        _hist->Fit(_fit, "WWS", "");
        return _fit;
    }

    void Fit1D::WriteHist()
    {
        _fit->SetNpx(1000);
        _hist->Write();
    }

}
