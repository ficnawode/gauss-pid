#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <TF1.h>
#include <TF2.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2D.h>
#include <TThread.h>
#include "AnalysisTree/Chain.hpp"
#include "AnalysisTree/Matching.hpp"
#include "GAUSPIDFit1D.hpp"

namespace GAUSPID
{
    class Fit2D
    {
    public:
        Fit2D(
            const std::vector<int> pdg,
            const float p_min,
            const float p_max,
            const unsigned int n_bins,
            const std::string filename);

        void FillHists();
        void FitHists();
        void WriteHists();
        TF2* ConcatenateFits();

    private:
        std::vector<Fit1D> _fits;
        TF2* _fit2d;
        const float _p_min;
        const float _p_max;
        const std::vector<int> _pdg;
        const unsigned int _n_bins;
        const std::string _filename;
    };
} // namespace GAUSPID
