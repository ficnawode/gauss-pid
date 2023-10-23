#pragma once

#include <vector>
#include <TF1.h>
#include <TH1F.h>

namespace GAUSPID
{
    class Fit1D
    {
    public:
        Fit1D(const std::vector<int> pdg, const float p_min, const float p_max);

        void FillHist(const float p, const float mass2);
        TF1* Fit();
        void WriteHist();

        const float GetPMin() const
        {
            return _p_min;
        }

        const float GetPMax() const
        {
            return _p_max;
        }

        const TF1* GetFitFunc() const
        {
            return _fit;
        }

    private:
        TH1F* _hist;
        TF1* _fit;

        const float _p_min;
        const float _p_max;
    };

}
