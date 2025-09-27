import matplotlib.pyplot as plt
import numpy as np

def save_colormap_to_hpp(name, filename, N=256):
    """
    把指定 colormap 存到 .hpp 文件
    格式: std::array<std::array<uint8_t, 3>, N>
    """
    cmap = plt.get_cmap(name, N)
    colors = (cmap(np.arange(N))[:, :3] * 255).astype(int)  # 只要 RGB

    with open(filename, "w") as f:
        f.write("#pragma once\n\n")
        f.write("#include <array>\n#include <cstdint>\n\n")
        f.write(f"// Colormap: {name}, size={N}\n")
        f.write(f"static constexpr std::array<std::array<uint8_t, 3>, {N}> colormap_{name} = {{\n")

        for i, (r, g, b) in enumerate(colors):
            f.write(f"    std::array<uint8_t, 3>{{{r:3d}, {g:3d}, {b:3d}}}, // {i}\n")

        f.write("};\n")

    print(f"Saved colormap '{name}' to {filename}")

def save_colormap_registry(names, filename="colormaps.hpp", N=256):
    with open(filename, "w") as f:
        f.write("#pragma once\n")
        f.write("#include <string>\n#include <array>\n#include <cstdint>\n")
        f.write("#include <unordered_map>\n#include <stdexcept>\n#include <iostream>\n\n")

        # include all individual headers
        for n in names:
            f.write(f'#include "colormap_{n}.hpp"\n')
        f.write("\n")

        f.write(f"inline const std::array<std::array<uint8_t, 3>, {N}>& get_colormap_color(\n")
        f.write("    const std::string& name)\n{\n")

        f.write("    static const std::unordered_map<std::string,\n")
        f.write(f"        const std::array<std::array<uint8_t, 3>, {N}>*> cmap_map = {{\n")
        for n in names:
            f.write(f'        {{"{n}", &colormap_{n}}},\n')
        f.write("    };\n\n")

        f.write("    auto it = cmap_map.find(name);\n")
        f.write("    if (it == cmap_map.end()) {\n")
        f.write(f'        std::cerr << "unknown colormap: " << name << ". reset colormap to {names[0]}" << std::endl;\n')
        f.write(f'        return *(cmap_map.at("{names[0]}"));\n')
        f.write("    }\n\n")
        f.write("    return *(it->second);\n")
        f.write("}\n")

    print(f"✅ Saved colormap registry to {filename}")


if __name__ == "__main__":
    tables = ["viridis", "plasma", "inferno", "magma", "jet"]
    for n in tables:
        save_colormap_to_hpp(n, f"colormap_{n}.hpp")
    save_colormap_registry(tables)