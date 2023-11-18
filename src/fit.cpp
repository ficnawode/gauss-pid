#include "GAUSPIDFit2D.hpp"

static inline bool check_argparse(const char* arg, const std::string long_form, const std::string short_form)
{
    auto res1 = strcmp(arg, long_form.c_str());
    auto res2 = strcmp(arg, short_form.c_str());
    return (res1 == 0) || (res2 == 0);
}

int main(int argc, char** argv)
{
    std::string filelist_path = "filelist_train.txt";
    std::string out_path = "gauss_out.root";
    int nbins = 50;

    using namespace std;
    for(int i = 1; i < argc; ++i)
    {
        if(check_argparse(argv[i], "--filelist", "-f"))
        {
            filelist_path = std::string(argv[++i]);
            cout << "Input filelist path: " << filelist_path << endl;
        }
        if(check_argparse(argv[i], "--output", "-o"))
        {
            out_path = std::string(argv[++i]);
            cout << "Output file path: " << out_path << endl;
        }
        if(check_argparse(argv[i], "--nbins", "-nb"))
        {
            nbins = atoi(argv[++i]);
            cout << "Number of bins: " << nbins << endl;
        }
    }

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

    TFile* out_file = TFile::Open(out_path.c_str(), "recreate");

    std::cout << "Writing to file..." << std::endl;
    for(auto& fit: fits)
    {
        fit.WriteHists();
    }

    std::cout << "Done." << std::endl;
    out_file->Close();
    return 0;
}
