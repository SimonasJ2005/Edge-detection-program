"""Nubraižo grafikus iš results/benchmark.csv į results/plots/.

Paleidimas iš projekto šakninio katalogo (reikia matplotlib: pip install matplotlib):

    python scripts/plot.py
"""

import csv
import math
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter, NullLocator

ROOT = Path(__file__).resolve().parent.parent
CSV_PATH = ROOT / "results" / "benchmark.csv"
OUT_DIR = ROOT / "results" / "plots"

# Spalvos (šviesus fonas). Serijų spalvos priskiriamos fiksuota tvarka.
SURFACE = "#fcfcfb"
TEXT = "#0b0b0b"
TEXT_2 = "#52514e"
MUTED = "#898781"
GRID = "#e1e0d9"
AXIS = "#c3c2b7"
SERIES = ["#2a78d6", "#eb6834", "#1baf7a", "#eda100"]

ORDERS = ["row", "col"]
ORDER_TITLE = {"row": "Tvarka: row (eilutėmis)", "col": "Tvarka: col (stulpeliais)"}

plt.rcParams.update({
    "figure.facecolor": SURFACE,
    "axes.facecolor": SURFACE,
    "savefig.facecolor": SURFACE,
    "font.size": 10,
    "text.color": TEXT,
    "axes.labelcolor": TEXT_2,
    "axes.edgecolor": AXIS,
    "axes.titlecolor": TEXT,
    "axes.titlesize": 11,
    "axes.titlelocation": "left",
    "xtick.color": MUTED,
    "ytick.color": MUTED,
    "xtick.labelcolor": TEXT_2,
    "ytick.labelcolor": TEXT_2,
    "axes.spines.top": False,
    "axes.spines.right": False,
    "axes.grid": True,
    "axes.grid.axis": "y",
    "grid.color": GRID,
    "grid.linewidth": 0.8,
    "legend.frameon": False,
    "legend.labelcolor": TEXT_2,
    "lines.linewidth": 2,
    "lines.markersize": 6,
})

# Skaičiai su dešimtainiu kableliu, kaip ir CSV faile
comma = FuncFormatter(lambda v, _: f"{v:g}".replace(".", ","))


def number(text):
    return float(text.replace(",", "."))


def load():
    with open(CSV_PATH, newline="", encoding="utf-8") as f:
        return [
            {
                "size": int(r["size"]),
                "order": r["order"],
                "threads": int(r["threads"]),
                "seq_ms": number(r["seq_ms"]),
                "par_ms": number(r["par_ms"]),
                "speedup": number(r["speedup"]),
                "efficiency": number(r["efficiency"]),
            }
            for r in csv.DictReader(f, delimiter=";")
        ]


def size_label(size):
    return f"{size}²"


def end_labels(ax, items, gap_px=14):
    """Užrašai linijų galuose; jei per arti vienas kito, pastumiami vertikaliai."""
    fig = ax.figure
    fig.canvas.draw()
    to_px = ax.transData.transform
    placed = sorted((to_px((x, y))[1], x, y, text) for x, y, text in items)
    ys = [p[0] for p in placed]
    for i in range(1, len(ys)):
        ys[i] = max(ys[i], ys[i - 1] + gap_px)
    for (y0, x, y, text), y1 in zip(placed, ys):
        ax.annotate(text, (x, y), xytext=(8, (y1 - y0) * 72 / fig.dpi), textcoords="offset points",
                    va="center", ha="left", color=TEXT_2, fontsize=9, annotation_clip=False)


LEFT = 0.07


def header(fig, title, subtitle=None):
    fig.text(LEFT, 0.96, title, fontsize=13, color=TEXT, ha="left", va="top")
    if subtitle:
        fig.text(LEFT, 0.895, subtitle, fontsize=9, color=TEXT_2, ha="left", va="top")


def top_legend(fig, ax, y, title=None):
    """Legenda viena eilute virš grafikų"""
    handles, names = ax.get_legend_handles_labels()
    fig.legend(handles, names, title=title, loc="upper left", bbox_to_anchor=(LEFT - 0.007, y),
               ncol=len(handles), alignment="left", title_fontsize=9, columnspacing=1.6,
               handlelength=2.6)


def two_panels(title):
    fig, axes = plt.subplots(1, 2, figsize=(11, 5.0), sharey=True, dpi=120)
    fig.subplots_adjust(left=LEFT, right=0.94, top=0.72, bottom=0.12, wspace=0.16)
    header(fig, title)
    return fig, axes


def thread_axis(ax, threads):
    ax.set_xticks(threads)
    ax.set_xlim(0.5, max(threads) + 0.5)
    ax.set_xlabel("Gijų skaičius p")


def plot_by_threads(data, metric, title, ylabel, filename, ideal):
    sizes = sorted({r["size"] for r in data})
    threads = sorted({r["threads"] for r in data})
    top = max(r[metric] for r in data)
    ylim = max(5.0, math.ceil(top * 1.2)) if metric == "speedup" else max(1.2, top * 1.1)

    fig, axes = two_panels(title)
    for ax, order in zip(axes, ORDERS):
        ax.set_title(ORDER_TITLE[order])
        ideal_y = [ideal(p) for p in threads]
        ax.plot(threads, ideal_y, color=MUTED, linestyle="--", linewidth=1.2, label="ideali", zorder=1)

        labels = []
        for i, size in enumerate(sizes):
            rows = sorted((r for r in data if r["size"] == size and r["order"] == order),
                          key=lambda r: r["threads"])
            xs = [r["threads"] for r in rows]
            ys = [r[metric] for r in rows]
            ax.plot(xs, ys, color=SERIES[i], marker="o", markeredgecolor=SURFACE,
                    markeredgewidth=1.5, label=size_label(size), zorder=3)
            labels.append((xs[-1], ys[-1], size_label(size)))

        thread_axis(ax, threads)
        ax.set_ylim(0, ylim)
        ax.yaxis.set_major_formatter(comma)
        end_labels(ax, labels)

    axes[0].set_ylabel(ylabel)
    top_legend(fig, axes[0], 0.91, title="Vaizdo dydis")
    fig.savefig(OUT_DIR / filename)
    plt.close(fig)


def plot_speedup_by_size(data, filename, picked=(2, 4, 8, 12)):
    sizes = sorted({r["size"] for r in data})
    available = {r["threads"] for r in data}
    picked = [p for p in picked if p in available]
    top = max(r["speedup"] for r in data if r["threads"] in picked)

    fig, axes = two_panels("Pagreitėjimas S pagal vaizdo dydį")
    for ax, order in zip(axes, ORDERS):
        ax.set_title(ORDER_TITLE[order])
        labels = []
        for i, p in enumerate(picked):
            rows = sorted((r for r in data if r["threads"] == p and r["order"] == order),
                          key=lambda r: r["size"])
            xs = list(range(len(rows)))
            ys = [r["speedup"] for r in rows]
            ax.plot(xs, ys, color=SERIES[i], marker="o", markeredgecolor=SURFACE,
                    markeredgewidth=1.5, label=f"p = {p}", zorder=3)
            labels.append((xs[-1], ys[-1], f"p = {p}"))

        ax.set_xticks(range(len(sizes)), [size_label(s) for s in sizes])
        ax.set_xlim(-0.3, len(sizes) - 0.7)
        ax.set_xlabel("Vaizdo dydis (pikseliai)")
        ax.set_ylim(0, max(5.0, math.ceil(top * 1.2)))
        ax.yaxis.set_major_formatter(comma)
        end_labels(ax, labels)

    axes[0].set_ylabel("S = T(nuosekli) / T(lygiagreti)")
    top_legend(fig, axes[0], 0.91, title="Gijų skaičius")
    fig.savefig(OUT_DIR / filename)
    plt.close(fig)


def plot_order_effect(data, filename):
    """Laikas vienam pikseliui: nuosekli ir geriausia lygiagreti versija, abi tvarkos."""
    sizes = sorted({r["size"] for r in data})
    seq = defaultdict(dict)
    best = defaultdict(dict)
    for r in data:
        pixels = r["size"] ** 2
        seq[r["order"]][r["size"]] = r["seq_ms"] * 1e6 / pixels
        par = r["par_ms"] * 1e6 / pixels
        best[r["order"]][r["size"]] = min(par, best[r["order"]].get(r["size"], math.inf))

    series = [
        ("nuosekli, row", seq["row"], SERIES[0], "-"),
        ("nuosekli, col", seq["col"], SERIES[0], "--"),
        ("lygiagreti, row", best["row"], SERIES[1], "-"),
        ("lygiagreti, col", best["col"], SERIES[1], "--"),
    ]

    fig, ax = plt.subplots(figsize=(11, 5.0), dpi=120)
    fig.subplots_adjust(left=LEFT, right=0.86, top=0.74, bottom=0.12)
    header(fig, "Prieigos tvarkos įtaka: laikas vienam pikseliui",
           "Lygiagreti versija - geriausias rezultatas iš visų p. Y ašis logaritminė.")

    xs = list(range(len(sizes)))
    labels = []
    for name, values, color, style in series:
        ys = [values[s] for s in sizes]
        ax.plot(xs, ys, color=color, linestyle=style, marker="o", markeredgecolor=SURFACE,
                markeredgewidth=1.5, label=name, zorder=3)
        labels.append((xs[-1], ys[-1], name))

    ax.set_yscale("log")
    ax.yaxis.set_major_locator(matplotlib.ticker.FixedLocator([1, 2, 5, 10, 20, 50, 100]))
    ax.yaxis.set_minor_locator(NullLocator())
    ax.yaxis.set_major_formatter(comma)
    ymin = min(min(v.values()) for _, v, _, _ in series)
    ymax = max(max(v.values()) for _, v, _, _ in series)
    ax.set_ylim(ymin / 1.5, ymax * 1.5)
    ax.set_xticks(xs, [size_label(s) for s in sizes])
    ax.set_xlim(-0.3, len(sizes) - 0.7)
    ax.set_xlabel("Vaizdo dydis (pikseliai)")
    ax.set_ylabel("ns / pikseliui")
    end_labels(ax, labels)
    top_legend(fig, ax, 0.86)
    fig.savefig(OUT_DIR / filename)
    plt.close(fig)


def main():
    data = load()
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    plot_by_threads(data, "speedup", "Pagreitėjimas S(p) = T(nuosekli) / T(lygiagreti, p)",
                    "S(p)", "1_speedup.png", ideal=lambda p: p)
    plot_by_threads(data, "efficiency", "Efektyvumas E(p) = S(p) / p",
                    "E(p)", "2_efficiency.png", ideal=lambda p: 1.0)
    plot_speedup_by_size(data, "3_speedup_size.png")
    plot_order_effect(data, "4_order.png")
    print(f"Grafikai išsaugoti {OUT_DIR.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
