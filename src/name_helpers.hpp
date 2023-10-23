#pragma once

#include <string>
#include <vector>

namespace name_helpers
{
    inline const std::string create_1d_hist_name(std::vector<int> pdgs, float p_min, float p_max)
    {
        std::string pdg_str = "";
        for(auto& pdg: pdgs)
        {
            pdg_str = pdg_str + std::to_string(pdg) + "_";
        }
        return pdg_str + "hist_" + std::to_string(p_min) + "_" + std::to_string(p_max);
    }

    inline const std::string create_1d_fit_name(std::vector<int> pdgs, float p_min, float p_max)
    {
        std::string pdg_str = "";
        for(auto& pdg: pdgs)
        {
            pdg_str = pdg_str + std::to_string(pdg) + "_";
        }
        return pdg_str + "fit_" + std::to_string(p_min) + "_" + std::to_string(p_max);
    }

    inline const std::string create_1d_fit_title(std::vector<int> pdgs, float p_min, float p_max)
    {
        std::string pdg_str = "";
        for(auto& pdg: pdgs)
        {
            pdg_str = pdg_str + std::to_string(pdg) + ", ";
        }
        return pdg_str + "gaussian fit for p in (" + std::to_string(p_min) + " , " + std::to_string(p_max) + ");m^2, GeV^2/c^4;counts" ;
    }

    inline const std::string create_2d_fit_name(std::vector<int> pdgs)
    {
        std::string pdg_str = "";
        for(auto& pdg: pdgs)
        {
            pdg_str = pdg_str + std::to_string(pdg) + "_";
        }
        return pdg_str + "fit";
    }

    inline const std::string create_2d_fit_title(std::vector<int> pdgs)
    {
        std::string pdg_str = "";
        for(auto& pdg: pdgs)
        {
            pdg_str = pdg_str + std::to_string(pdg) + ", ";
        }
        return pdg_str + "gaussian fit;p, GeV/c;m^2 , GeV^2/c^4 ;" ;
    }
} // namespace name_helpers
