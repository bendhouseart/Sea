#!/usr/bin/env python3

from bokeh.io import output_notebook, show
from bokeh.models import ColumnDataSource, HoverTool, Legend, LegendItem, Whisker
from bokeh.plotting import figure
from datetime import datetime
import pandas as pd
from os import path as op


colours = ["#c9d9d3", "#718dbf", "#e84d60"]


def bokeh_gantt(df):

    output_notebook()
    y_range = sorted(df["pid"].unique(), key=lambda x: int(x))
    source_read = ColumnDataSource(df[df["action"].str.contains("read")])
    source_increment = ColumnDataSource(df[df["action"].str.contains("inc")])
    source_write = ColumnDataSource(df[df["action"].str.contains("save")])

    p = figure(toolbar_location="below", x_range=(0, 1100), y_range=y_range)
    p.hbar(
        y="pid",
        left="start",
        right="end",
        height=0.4,
        fill_color="color",
        source=source_read,
        legend_label="read",
    )
    p.hbar(
        y="pid",
        left="start",
        right="end",
        height=0.4,
        fill_color="color",
        source=source_increment,
        legend_label="increment",
    )
    p.hbar(
        y="pid",
        left="start",
        right="end",
        height=0.4,
        fill_color="color",
        source=source_write,
        legend_label="write",
    )

    p.ygrid.grid_line_color = None
    p.xaxis.axis_label = "Time (seconds)"
    p.yaxis.axis_label = "PID"

    p.legend.click_policy = "hide"

    p.outline_line_color = None
    p.add_tools(
        HoverTool(
            show_arrow=False,
            line_policy="next",
            tooltips=[
                ("Task", "@action"),
                ("Image", "@img"),
                ("Start", "@start"),
                ("End", "@end"),
                ("Duration", "@duration"),
            ],
        )
    )

    show(p)


def bokeh_stacked(df):

    df["whisker"] = df.index + 0.5
    output_notebook()

    actions = ["read", "increment", "write"]
    source = ColumnDataSource(df)

    p = figure(
        x_range=df["type"],
        toolbar_location="below",
        tools="hover",
        tooltips=[("Task", "$name"), ("Duration", "@$name")],
    )

    p.vbar_stack(
        actions,
        x="type",
        width=0.9,
        fill_color=colours,
        source=source,
        legend_label=actions,
    )

    p.add_layout(
        Whisker(
            source=source,
            base="whisker",
            upper="read_upper",
            lower="read_lower",
            level="overlay",
        ),
    )
    p.add_layout(
        Whisker(
            source=source,
            base="whisker",
            upper="inc_upper",
            lower="inc_lower",
            level="overlay",
        )
    )
    p.add_layout(
        Whisker(
            source=source,
            base="whisker",
            upper="write_upper",
            lower="write_lower",
            level="overlay",
        )
    )
    p.y_range.start = 0
    p.x_range.range_padding = 0.1
    p.xgrid.grid_line_color = None
    p.axis.minor_tick_line_color = None
    p.outline_line_color = None
    p.legend.orientation = "horizontal"
    p.legend.location = "top_left"
    p.legend.click_policy = "hide"

    show(p)


def action_columns_summ(df):

    df_read = df[df["action"].str.contains("read")].rename(columns={"mean": "read"})

    df_increment = df[df["action"].str.contains("inc")].rename(
        columns={"mean": "increment"}
    )

    df_write = df[df["action"].str.contains("save")].rename(columns={"mean": "write"})

    df = pd.merge(df_read, df_increment, on=["type"])
    df = pd.merge(df, df_write, on=["type"])

    df["read_lower"] = df["read"] - df["std"]
    df["read_upper"] = df["read"] + df["std"]
    df["inc_lower"] = df["read"] + df["increment"] - df["std"]
    df["inc_upper"] = df["read"] + df["increment"] + df["std"]
    df["write_lower"] = df["read"] + df["increment"] + df["write"] - df["std"]
    df["write_upper"] = df["read"] + df["increment"] + df["write"] + df["std"]

    return df[
        [
            "type",
            "read",
            "increment",
            "write",
            "read_lower",
            "read_upper",
            "inc_lower",
            "inc_upper",
            "write_lower",
            "write_upper",
        ]
    ]


def mean_std(df):
    df_mean = (
        df.groupby("action").mean().rename(columns={"duration": "mean"}).reset_index()
    )
    df_std = (
        df.groupby("action").std().rename(columns={"duration": "std"}).reset_index()
    )

    #if df_std.isnull().values.any():
    #    df = pd.merge(df_mean, df_std, on=["action"])
    #else:
    df = pd.merge(df_mean, df_std, on=["action"])

    return df


def group_actions(in_mem_df, mem_flush_df, lustre_df):

    group = lambda df: (
        df[["repetition", "action", "duration"]]
        .groupby(["repetition", "action"])
        .sum()
        .reset_index()
    )

    in_mem_df = group(in_mem_df)
    in_mem_df = mean_std(in_mem_df)
    in_mem_df["type"] = "Sea - Flush last"

    mem_flush_df = group(mem_flush_df)
    mem_flush_df = mean_std(mem_flush_df)
    mem_flush_df["type"] = "Sea - Flush all"

    lustre_df = group(lustre_df)
    lustre_df = mean_std(lustre_df)
    lustre_df["type"] = "Lustre"

    df = pd.concat([in_mem_df, mem_flush_df, lustre_df], axis=0)

    return action_columns_summ(df)


def format_df(df, df_type):
    df = df[df["action"].str.contains(df_type)]
    # df['time'] = df['time'].apply(lambda x: datetime.fromtimestamp(x))
    df = df.rename(columns={"time": df_type})
    df["action"] = df["action"].apply(lambda x: x.split("_")[0])
    df["pid"] = df["pid"].astype("str")  # % df["pid"].nunique()
    return df


def add_rep_idx(df_list):
    for i in range(len(df_list)):
        df_list[i]["repetition"] = i

    return df_list


def load_df(fn):
    from datetime import datetime

    if op.basename(fn) != "benchmarks.out":
        threads = int(op.basename(fn).split('_')[3].strip('t')) if 'sea' in fn else int(op.basename(fn).split('_')[2].strip('t'))
    else:
        threads = int(op.basename(op.dirname(fn)).split('_')[3].strip('t')) if 'sea' in fn else int(op.basename(op.dirname(fn)).split('_')[2].strip('t'))

    df = pd.read_csv(fn, names=["action", "img", "time", "pid", "node"], header=None, index_col=False)


    all_pids = df.sort_values(by=["time"])[["node", "pid"]].apply(lambda row:  row["node"] + str(row["pid"]), axis=1).unique().tolist()
    all_nodes = df["node"].sort_values().unique().tolist()

    def unique_pid(row):
        idx = all_pids.index(row["node"] + str(row["pid"]))
        #print("idx", idx)
        pid = (idx % threads) + (all_nodes.index(row["node"]) * 10)
        #print("pid", pid)
        return pid
    #unique_pid = lambda x: all_pids.index(x["node"] + str(x["pid"])) % threads

    df['pid'] = df.apply(lambda x: unique_pid(x), axis=1)
    df_start = format_df(df, "start")
    df_end = format_df(df, "end")

    df = pd.merge(df_start, df_end, on=["action", "img", "pid"])
    df.loc[df["action"] == "read", "color"] = colours[0]
    df.loc[df["action"] == "inc", "color"] = colours[1]
    df.loc[df["action"] == "save", "color"] = colours[2]
    df.loc[df["action"] == "ls", "action"] = "read"

    start_time = df["start"].min()
    df["start"] = df["start"] - start_time
    df["end"] = df["end"] - start_time
    df["duration"] = df["end"] - df["start"]

    return df
