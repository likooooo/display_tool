
## SpectrumPlot 用户手册

#### 📖 简介

`SpectrumPlot` 是一个于光谱辐照度绘图工具。
支持 **多种绘图模式**，能够将光谱数据辐照度转换并展示为：

* **模式 0**：二维曲线图 (Curve)
* **模式 1**：三维散点图 (3D Scatter)
* **模式 2**：CIE 1931 色度图 (Chromaticity Diagram)
* **模式 3**：色域填充图 (Gamut Fill, 投影到平面 X+Y+Z=1 并计算 sRGB)

---

#### 📂 输入数据格式

输入文件为 CSV 或 TXT，每行四列：

```
wavelength  X   Y   Z
380         0.0014   0.0000   0.0065
381         0.0022   0.0001   0.0105
...
780         0.0000   0.0000   0.0000
```

要求：

* 第一列：波长 (nm)
* 第二\~四列：颜色匹配函数XYZ

---

#### ▶️ 使用方法

###### 基本运行方式

```bash
python plot_spectrum.py input.csv -m MODE
```

###### 命令行参数说明

| 参数                | 必选 | 说明                                                            | 默认值 |
| ----------------- | -- | ------------------------------------------------------------- | --- |
| `file_path`       | ✅  | 输入的 CSV/TXT 文件路径                                              | -   |
| `-m, --plot-mode` | ❌  | 绘图模式：<br>0=curve<br>1=3d-scatter<br>2=chromaticity<br>3=gamut | 0   |
| `-h, --help`      | ❌  | 显示帮助信息并退出                                                     | -   |
| `-v, --version`   | ❌  | 显示版本号                                                         | -   |

---

#### 📌 使用示例

- [cie-1931-colour-matching-functions-2-degree-observer](https://cie.co.at/datatable/cie-1931-colour-matching-functions-2-degree-observer)

###### 1. 绘制二维曲线图

```bash
python plot_spectrum.py sample_xyz.txt -m 0
```

###### 2. 绘制三维散点图

```bash
python plot_spectrum.py sample_xyz.txt -m 1
```

###### 3. 绘制 CIE 1931 色度图

```bash
python plot_spectrum.py sample_xyz.txt -m 2
```

###### 4. 绘制色域填充图

```bash
python plot_spectrum.py sample_xyz.txt -m 3
```

---

#### 🎨 效果图示例

* **模式 0：二维曲线**
  ![Curve Example](docs/example_curve.png)

* **模式 1：三维散点**
  ![3D Scatter Example](docs/example_3dscatter.png)

* **模式 2：CIE 色度图**
  ![Chromaticity Example](docs/example_chromaticity.png)

* **模式 3：色域填充**
  ![Gamut Example](docs/example_gamut.png)

---

#### TODO
1. 看不见D65白点和分割线