#include "GAUSPIDFit2D.hpp"

int main()
{
    auto filelist_path = "filelist_train.txt";
    auto out_path = "gauss_out.root";
    int nbins = 50;

    const float p_min = 0;
    const float p_max = 6;

    const std::vector<int> proton_pdg = {2212};
    const std::vector<int> kaon_pdg = {321};
    const std::vector<int> pion_pdg = {13, 211, 11};
    const std::array<std::vector<int>, 3> pdgs = {proton_pdg, kaon_pdg, pion_pdg};

    std::vector<GAUSPID::Fit2D> fits;
    for(auto& pdg: pdgs)
    {
        fits.push_back(GAUSPID::Fit2D(pdg, p_min, p_max, nbins, filelist_path));
    }

    std::cout << "Filling histograms..." << std::endl;
    for(auto& fit: fits)
    {
        fit.FillHists();
    }

    std::cout << "Fitting histograms..." << std::endl;
    for(auto& fit: fits)
    {
        fit.FitHists();
        fit.ConcatenateFits();
    }

    TFile* out_file = TFile::Open(out_path, "recreate");

    std::cout << "Writing to file..." << std::endl;
    for(auto& fit: fits)
    {
        fit.WriteHists();
    }

    std::cout << "Done." << std::endl;
    out_file->Close();
    return 0;
}
