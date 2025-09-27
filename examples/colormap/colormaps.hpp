#pragma once
#include <string>
#include <array>
#include <cstdint>
#include <unordered_map>
#include <stdexcept>
#include <iostream>

#include "colormap_viridis.hpp"
#include "colormap_plasma.hpp"
#include "colormap_inferno.hpp"
#include "colormap_magma.hpp"
#include "colormap_jet.hpp"

inline const std::array<std::array<uint8_t, 3>, 256>& get_colormap_color(
    const std::string& name)
{
    static const std::unordered_map<std::string,
        const std::array<std::array<uint8_t, 3>, 256>*> cmap_map = {
        {"viridis", &colormap_viridis},
        {"plasma", &colormap_plasma},
        {"inferno", &colormap_inferno},
        {"magma", &colormap_magma},
        {"jet", &colormap_jet},
    };

    auto it = cmap_map.find(name);
    if (it == cmap_map.end()) {
        std::cerr << "unknown colormap: " << name << ". reset colormap to viridis" << std::endl;
        return *(cmap_map.at("viridis"));
    }

    return *(it->second);
}
