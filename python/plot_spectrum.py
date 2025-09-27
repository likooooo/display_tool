#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
SpectrumPlot v1.0
- plot modes: 0=curve, 1=3d-scatter, 2=chromaticity, 3=gamut
- chromaticity/gamut project points onto plane X+Y+Z=1 and compute sRGB colors
"""

import argparse
import os
import csv
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D  # noqa: F401
from matplotlib.path import Path

# --- 模式映射 & 版本 ---
PLOT_MODES = {0: 'curve', 1: '3d-scatter', 2: 'chromaticity', 3: 'gamut'}
VERSION = "SpectrumPlot v1.0, © 2025 by https://github.com/likooooo"

# --- XYZ -> linear RGB 矩阵 (D65 白点) ---
XYZ_TO_LINEAR_RGB = np.array([
    [3.2404542, -1.5371385, -0.4985314],
    [-0.9692660,  1.8760108,  0.0415560],
    [0.0556434, -0.2040259,  1.0572252]
], dtype=float)


# -------------------------
# 文件解析
# -------------------------
def parse_data(file_path):
    if not os.path.exists(file_path):
        print(f"错误：文件未找到！请检查路径: {file_path}")
        return None, None, None, None

    wavelengths, ix, iy, iz = [], [], [], []
    try:
        with open(file_path, mode='r', newline='', encoding='utf-8') as f:
            reader = csv.reader(f)
            for i, row in enumerate(reader):
                if len(row) < 4:
                    continue
                try:
                    w, x, y, z = map(float, row[:4])
                    wavelengths.append(w)
                    ix.append(x)
                    iy.append(y)
                    iz.append(z)
                except ValueError:
                    print(f"警告：第 {i+1} 行包含非数字，跳过。")
        if not wavelengths:
            print("错误：未找到有效数据。")
            return None, None, None, None
        print(f"成功读取 {len(wavelengths)} 条数据。")
        return np.array(wavelengths), np.array(ix), np.array(iy), np.array(iz)
    except Exception as e:
        print(f"读取文件出错: {e}")
        return None, None, None, None


# -------------------------
# 数学工具：XYZ <-> RGB
# -------------------------
def linear_rgb_from_xyz_array(Xs, Ys, Zs):
    """输入 Xs,Ys,Zs shape=(N,)；返回 linear RGB (3,N)，裁剪至 [0,1]"""
    XYZs = np.vstack([Xs, Ys, Zs])
    linear = XYZ_TO_LINEAR_RGB.dot(XYZs)
    return np.clip(linear, 0.0, 1.0)


def linear_to_srgb_array(linear):
    """输入 linear (3,N)，输出 gamma 校正后的 sRGB (3,N)"""
    mask = linear <= 0.0031308
    srgb = np.where(mask, 12.92 * linear,
                    1.055 * (linear ** (1.0 / 2.4)) - 0.055)
    return np.clip(srgb, 0.0, 1.0)

# -------------------------
# CIE 背景图像
# -------------------------
def generate_cie_image(res=400, xlim=(0.0, 0.8), ylim=(0.0, 0.9)):
    xs = np.linspace(xlim[0], xlim[1], res)
    ys = np.linspace(ylim[0], ylim[1], res)
    xv, yv = np.meshgrid(xs, ys)
    img = np.ones((res, res, 3), dtype=float)

    mask = (xv >= 0) & (yv >= 0) & (xv + yv <= 1)
    if np.any(mask):
        x_mask, y_mask = xv[mask], yv[mask]
        z_mask = 1.0 - x_mask - y_mask
        srgb = linear_to_srgb_array(linear_rgb_from_xyz_array(x_mask, y_mask, z_mask))
        img[mask] = srgb.T
    return xv, yv, img


# -------------------------
# 绘图函数
# -------------------------
def plot_2d_curves(W, IX, IY, IZ, filename):
    plt.figure(figsize=(10, 6))
    plt.plot(W, IX, 'r', label='Irradiance X')
    plt.plot(W, IY, 'g', label='Irradiance Y')
    plt.plot(W, IZ, 'b', label='Irradiance Z')
    plt.title(f'2D Spectrum Curves: {filename}')
    plt.xlabel('Wavelength (波长)')
    plt.ylabel('Irradiance (辐照度)')
    plt.legend()
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.show()


def plot_3d_scatter(W, IX, IY, IZ, filename):
    fig = plt.figure(figsize=(10, 8))
    ax = fig.add_subplot(111, projection='3d')
    sc = ax.scatter(IX, IY, IZ, c=W, cmap='viridis')
    cbar = fig.colorbar(sc, ax=ax)
    cbar.set_label('Wavelength (波长)')
    ax.set_xlabel('Irradiance X')
    ax.set_ylabel('Irradiance Y')
    ax.set_zlabel('Irradiance Z')
    ax.set_title(f'3D Irradiance Scatter: {filename}')
    plt.tight_layout()
    plt.show()

def plot_cie_chromaticity(IX, IY, IZ, filename):
    sums = IX + IY + IZ
    valid = sums > 1e-6
    if not np.any(valid):
        print("错误：所有数据的 X+Y+Z 均为零或非常小，无法计算色度坐标或颜色。")
        return

    x = IX[valid] / sums[valid]
    y = IY[valid] / sums[valid]

    # 固定亮度 Y=0.5，构造与 x,y 长度一致的数组
    Y_norm_val = 0.5
    Y_norm = np.full_like(x, Y_norm_val, dtype=float)

    # 避免 y 接近 0 时除零：对小 y 使用 fallback（这里设为 0）
    safe_mask = y > 1e-9
    X_norm = np.where(safe_mask, (x / y) * Y_norm, 0.0)
    Z_norm = np.where(safe_mask, ((1.0 - x - y) / y) * Y_norm, 0.0)

    # 现在三个数组长度一致，可直接向量化转换为 sRGB
    srgb = linear_to_srgb_array(linear_rgb_from_xyz_array(X_norm, Y_norm, Z_norm))
    colors = srgb.T  # shape (N,3)

    plt.figure(figsize=(9, 9))
    plt.scatter(x, y, color=colors, marker='o', s=40, alpha=0.8)
    plt.plot(x, y, color='gray', linestyle='--', linewidth=0.5, alpha=0.5, label='Color Trajectory')

    plt.xlim(-0.1, 0.9)
    plt.ylim(-0.1, 1.0)
    plt.gca().set_aspect('equal', adjustable='box')

    plt.title(f'CIE 1931 xy Chromaticity Diagram for: {filename}', fontsize=14)
    plt.xlabel('x Chromaticity Coordinate', fontsize=12)
    plt.ylabel('y Chromaticity Coordinate', fontsize=12)
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.show()

def plot_gamut_fill(wavelengths, IX, IY, IZ, filename, res_background=600):
    sums = IX + IY + IZ
    valid = sums > 1e-9
    if np.count_nonzero(valid) < 3:
        print("错误：有效点不足以构成闭合边界。")
        return

    x, y = IX[valid] / sums[valid], IY[valid] / sums[valid]
    center = (np.mean(x), np.mean(y))
    order = np.argsort(np.arctan2(y - center[1], x - center[0]))
    polygon_pts = np.column_stack([x[order], y[order]])

    xv, yv, img = generate_cie_image(res=res_background)
    points = np.column_stack([xv.ravel(), yv.ravel()])
    mask = Path(polygon_pts).contains_points(points).reshape(xv.shape)

    final_img = np.ones_like(img)
    final_img[mask] = img[mask]

    fig, ax = plt.subplots(figsize=(9, 9))
    ax.imshow(final_img, extent=(0, 0.8, 0, 0.9), origin='lower', aspect='auto')
    ax.add_patch(plt.Polygon(polygon_pts, closed=True, edgecolor='black',
                             facecolor='none', linewidth=3))
    # ax.scatter(x, y, s=18, c='black', label='Boundary Points')
    # 假设 wavelengths 与 x, y 对应
    step = 10  # 至少 10 nm 间隔
    indices = np.arange(0, len(wavelengths), step)

    # 选取采样点
    x_sample = x[indices]
    y_sample = y[indices]
    wl_sample = wavelengths[indices]

    # 绘制稀疏采样点
    ax.scatter(x_sample, y_sample, s=40, c='black', label='Boundary Points')

    # 在点旁边标注波长
    for xi, yi, wl in zip(x_sample, y_sample, wl_sample):
        if 450 > wl or wl > 650: continue
        ax.text(xi + 0.01, yi + 0.01, f"{wl} nm",
                fontsize=8, color='black', ha='left', va='bottom')

    ax.set_xlim(0, 0.8)
    ax.set_ylim(0, 0.9)
    ax.set_aspect('equal', adjustable='box')
    ax.set_title(f'Filled Color Gamut (X+Y+Z=1): {filename}')
    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.legend()
    plt.tight_layout()
    plt.show()


# -------------------------
# 主入口
# -------------------------
def main():
    parser = argparse.ArgumentParser(description="CSV 辐照度绘图工具", add_help=False)
    parser.add_argument("file_path", type=str, help="CSV 文件路径")
    parser.add_argument("-m", "--plot-mode", type=int, default=0,
                        choices=PLOT_MODES.keys(),
                        help="绘图模式: 0=curve, 1=3d-scatter, 2=chromaticity, 3=gamut")
    parser.add_argument("-h", "--help", action="help", help="显示帮助并退出")
    parser.add_argument("-v", "--version", action="version", version=VERSION)
    args = parser.parse_args()

    W, IX, IY, IZ = parse_data(args.file_path)
    if W is None:
        return

    filename = os.path.basename(args.file_path)
    mode = PLOT_MODES[args.plot_mode]

    if mode == 'curve':
        plot_2d_curves(W, IX, IY, IZ, filename)
    elif mode == '3d-scatter':
        plot_3d_scatter(W, IX, IY, IZ, filename)
    elif mode == 'chromaticity':
        plot_cie_chromaticity(IX, IY, IZ, filename)
    elif mode == 'gamut':
        plot_gamut_fill(W, IX, IY, IZ, filename)


if __name__ == "__main__":
    main()
