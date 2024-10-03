import pandas as pd
import numpy as np
import json
import sys
import os
import datetime as dt

# import time
import matplotlib.pyplot as plt
import warnings
from collections import OrderedDict


class Utils:
    def __init__(self, cra_file, backtest_directory):
        self.cra_file = cra_file
        self.backtest_directory = backtest_directory + "/results/"
        self.extract_unique_id()

    def extract_unique_id(self):
        self.unique_id = self.cra_file.split("_")[3]
        print("unique_id is :- ", self.unique_id)

    def extract_files(self):
        pnl_file_name = self.backtest_directory + self.cra_file.replace(
            ".cra", "_pnl.csv"
        )
        fill_file_name = self.backtest_directory + self.cra_file.replace(
            ".cra", "_fill.csv"
        )
        order_file_name = self.backtest_directory + self.cra_file.replace(
            ".cra", "_order.csv"
        )
        if (
            os.path.exists(pnl_file_name)
            and os.path.exists(fill_file_name)
            and os.path.exists(order_file_name)
        ):
            return (
                pd.read_csv(pnl_file_name),
                pd.read_csv(fill_file_name),
                pd.read_csv(order_file_name),
            )
        else:
            return None, None, None


class PNLAnalytics:
    def __init__(
        self,
        pnl_df,
        unique_id,
        market_open="09:30",
        market_close="16:00",
        account_size=1000000,
        rf=0.0,
        daily_stats=False,
    ):
        self.account_size = float(account_size)
        self.pnl_df = pnl_df
        self.unique_id = unique_id
        self.days = 1
        self.filter_pnl_df(market_open, market_close)
        self.sharpe_ratio_scale = (252) / np.sqrt(
            252 * 6.5 * 60
        )  # change based on asset 6.5hrs for equities
        self.sortino_ratio_scale = np.sqrt(252 * 6.5 * 60)
        self.rf = rf
        self.calculate_params()
        self.daily = daily_stats
        if daily_stats:
            self.create_individual_dfs()
            self.calculate_daily_params()

    def parse_datetime_or_na(self, datetime_str):
        try:
            # Attempt to parse with fractional seconds
            return dt.datetime.strptime(datetime_str, "%Y-%b-%d %H:%M:%S.%f")
        except ValueError:
            # Return 'NA' if parsing fails
            return "NA"

    def filter_pnl_df(self, market_open, market_close):
        # Function to check for fractional seconds
        def has_fractional_seconds(time_str):
            return "." in time_str.split()[1]

        # Filter out rows without fractional seconds
        self.pnl_df = self.pnl_df[
            self.pnl_df["Time"].astype(str).apply(has_fractional_seconds)
        ]

        # Parse datetime values and remove rows with 'NA'
        self.pnl_df.loc[:, "Time"] = pd.to_datetime(
            self.pnl_df["Time"], errors="coerce"
        )
        self.pnl_df = self.pnl_df.dropna(subset=["Time"])

        # Convert to New York timezone
        self.pnl_df["Time"] = pd.to_datetime(self.pnl_df["Time"])
        self.pnl_df["Time"] = (
            self.pnl_df["Time"].dt.tz_localize("UTC").dt.tz_convert("America/New_York")
        )

        # Convert market_open and market_close to datetime time objects
        market_open_time = pd.to_datetime(market_open, format="%H:%M").time()
        market_close_time = pd.to_datetime(market_close, format="%H:%M").time()

        # Filter rows based on the specified market open and close times
        def is_within_market_hours(timestamp):
            local_time = timestamp.time()
            return market_open_time <= local_time <= market_close_time

        self.pnl_df = self.pnl_df[self.pnl_df["Time"].apply(is_within_market_hours)]
        # Extract unique days
        self.day_list = {timestamp.date() for timestamp in self.pnl_df["Time"]}
        self.days = len(self.day_list)

    def create_individual_dfs(self):
        self.pnl_dfs = {}
        for date in self.day_list:
            date = date.strftime("%Y-%m-%d")
            self.pnl_dfs[date] = self.pnl_df[
                self.pnl_df["Time"].dt.date == pd.to_datetime(date).date()
            ]
            self.pnl_dfs[date].loc[:, "Cumulative PnL"] = (
                self.pnl_dfs[date]["Cumulative PnL"]
                - self.pnl_dfs[date].loc[self.pnl_dfs[date].index[0], "Cumulative PnL"]
            )

    def create_pnl_stats(self, df_):
        df = df_.copy()
        df.loc[:, "account_value"] = df["Cumulative PnL"] + self.account_size
        df.loc[:, "PnL"] = df["Cumulative PnL"].diff().fillna(0)
        df.loc[:, "cumulative_pnl_percentage"] = (
            df["account_value"] / self.account_size
        ) - 1
        df.loc[:, "returns"] = df["account_value"].pct_change().fillna(0)
        return df

    def calculate_sharpe_ratio(self, df, scale=1):
        # Adjust risk-free rate for the number of periods
        return (
            self.sharpe_ratio_scale
            * (
                df.loc[df.index[-1], "Cumulative PnL"]
                - df.loc[df.index[0], "Cumulative PnL"]
            )
            - self.rf
        ) / (np.std(df.account_value) * scale)

    def calculate_sortino_ratio(self, df, scale=1):
        downside_returns = df["returns"][df["returns"] < 0]
        downside_deviation = np.sqrt(np.mean(np.square(downside_returns)))
        excess_returns = df["returns"]
        return (
            (np.mean(excess_returns) * self.sortino_ratio_scale - self.rf)
            / (downside_deviation * scale)
            if downside_deviation != 0
            else np.nan
        )

    def calculate_max_drawdown(self, df):
        df = df.copy()
        df.loc[:, "drawdown"] = (
            df["account_value"] - df["account_value"].cummax()
        ) / df["account_value"].cummax()
        return df["drawdown"].min(), df

    def calculate_params(self):
        self.pnl_df = self.create_pnl_stats(self.pnl_df)
        self.sharpe = self.calculate_sharpe_ratio(self.pnl_df, self.days)
        self.sortino = self.calculate_sortino_ratio(self.pnl_df, self.days)
        self.max_drawdown, self.pnl_df = self.calculate_max_drawdown(self.pnl_df)

    def calculate_daily_params(self):
        self.daily_stats = {}
        for date in self.pnl_dfs.keys():
            self.pnl_dfs[date] = self.create_pnl_stats(self.pnl_dfs[date])
            self.daily_stats[date] = {}
            self.daily_stats[date]["sharpe"] = self.calculate_sharpe_ratio(
                self.pnl_dfs[date]
            )
            self.daily_stats[date]["sortino"] = self.calculate_sortino_ratio(
                self.pnl_dfs[date]
            )
            self.daily_stats[date]["max_drawdown"], self.pnl_dfs[date] = (
                self.calculate_max_drawdown(self.pnl_dfs[date])
            )

    def create_pnl_plots(self, directory, text_to_add="", num_ticks=10):

        def add_text_to_figure(figure, text):
            figure.text(0.1, 0.01, text, ha="center", fontsize=12, color="grey")

        def plot_helper(y, axs, title, xlabel, ylabel, tick_df):
            axs.plot(np.arange(len(y)), y)
            axs.set_title(title)
            axs.set_xlabel(xlabel)
            axs.set_ylabel(ylabel)
            axs.set_xticks(list(tick_df.keys()))
            axs.set_xticklabels(
                list(tick_df.values()), rotation=90, ha="right", fontsize=8
            )
            axs.grid(True)

        def hist_helper(y, axis, title, xlabel, ylabel, bins=50):
            axis.hist(y, bins=50)
            axis.set_title(title)
            axis.set_xlabel(xlabel)
            axis.set_ylabel(ylabel)
            axis.grid(True)

        def get_tick_df(df, num_ticks=10):
            df_ = df.copy()
            df_["time_index"] = np.arange(len(df_))
            ticks = {
                row["time_index"]: row["Time"].strftime("%Y-%m-%d %H:%M")
                for index, row in df_.iterrows()
            }
            # Select 5 representative ticks evenly spaced across the data
            if len(ticks) > num_ticks:
                tick_keys = np.linspace(
                    min(ticks.keys()), max(ticks.keys()), num=num_ticks, endpoint=True
                )
                tick_keys = [
                    int(key) for key in tick_keys
                ]  # Ensure the keys are integers
                selected_ticks = {key: ticks[key] for key in tick_keys if key in ticks}
            else:
                selected_ticks = ticks
            return selected_ticks

        fig, axs = plt.subplots(4, 1, figsize=(20, 40))
        ticks = get_tick_df(self.pnl_df, num_ticks)
        plot_helper(
            self.pnl_df["account_value"],
            axs[0],
            "Account Value Over Time",
            "Time",
            "Account Value",
            ticks,
        )

        plot_helper(
            self.pnl_df["Cumulative PnL"],
            axs[1],
            "Cumulative PnL",
            "Time",
            "Cumulative PnL",
            ticks,
        )

        title = "returns vs drawdowns"
        plot_helper(
            self.pnl_df["cumulative_pnl_percentage"],
            axs[2],
            title,
            "Time",
            "Cumulative PnL Percentage",
            ticks,
        )
        plot_helper(
            self.pnl_df["drawdown"],
            axs[2],
            title,
            "Time",
            "Cumulative PnL Percentage",
            ticks,
        )

        hist_helper(
            self.pnl_df["returns"],
            axs[3],
            "Returns Distribution",
            "Returns",
            "Frequency",
        )

        add_text_to_figure(fig, text_to_add + ":- total stats")
        plt.tight_layout()
        directory = directory + "result_plots/"
        os.makedirs(directory, exist_ok=True)
        file_path = os.path.join(directory, self.unique_id + "results_plot.png")
        plt.savefig(file_path)

        # Additional plots
        if self.daily and self.days > 1:
            fig, axs = plt.subplots(self.days, 1, figsize=(20, 20))
            for i, date in enumerate(self.day_list):
                date = date.strftime("%Y-%m-%d")
                ticks = get_tick_df(self.pnl_dfs[date], num_ticks)
                plot_helper(
                    self.pnl_dfs[date]["account_value"],
                    axs[i],
                    "Account Value Over Time",
                    "Time",
                    "Account Value",
                    ticks,
                )
            add_text_to_figure(fig, text_to_add + " Daily Account Value Over Time")
            plt.tight_layout()
            # Save additional plots
            additional_file_path = os.path.join(directory, self.unique_id + "_av.png")
            plt.savefig(additional_file_path)

        if self.daily and self.days > 1:
            fig, axs = plt.subplots(self.days, 1, figsize=(20, 20))
            for i, date in enumerate(self.day_list):
                date = date.strftime("%Y-%m-%d")
                title = "returns vs drawdowns"
                ticks = get_tick_df(self.pnl_dfs[date], num_ticks)
                plot_helper(
                    self.pnl_dfs[date]["cumulative_pnl_percentage"],
                    axs[i],
                    title,
                    "Time",
                    "Cumulative PnL Percentage",
                    ticks,
                )
                plot_helper(
                    self.pnl_dfs[date]["drawdown"],
                    axs[i],
                    title,
                    "Time",
                    "Cumulative PnL Percentage",
                    ticks,
                )
            add_text_to_figure(fig, text_to_add + " Daily Returns vs Drawdowns")
            plt.tight_layout()
            plt.show()
            # Save additional plots
            additional_file_path = os.path.join(directory, self.unique_id + "_rvd.png")
            plt.savefig(additional_file_path)

        if self.daily and self.days > 1:
            # returns distribution
            fig, axs = plt.subplots(self.days, 1, figsize=(20, 20))
            for i, date in enumerate(self.day_list):
                date = date.strftime("%Y-%m-%d")
                title = "Returns Distribution"
                hist_helper(
                    self.pnl_dfs[date]["returns"], axs[i], title, "Returns", "Frequency"
                )
            add_text_to_figure(fig, text_to_add + " Daily Returns Distribution")
            plt.tight_layout()
            plt.show()

    def append_data_to_json(
        self, directory, parameters, name="results.json", latest_commit_id="NA"
    ):
        file_path = os.path.join(directory, name)
        parameters = parameters.split("|")
        parameters = {
            parameter.split("=")[0]: parameter.split("=")[1] for parameter in parameters
        }
        date_list = [date.isoformat() for date in self.day_list]
        new_data = {
            self.unique_id: {
                "latest_commit_id": latest_commit_id,
                "total pnl": self.pnl_df["Cumulative PnL"].values[-1],
                "total returns": self.pnl_df["account_value"].values[-1],
                "sharpe": self.sharpe,
                "sortino": self.sortino,
                "max_drawdown": self.max_drawdown,
                "parameters": parameters,
                "dates": date_list,
                "daily_stats": self.daily_stats if self.daily else None,
            }
        }
        # Read existing data and merge with new data
        if os.path.exists(file_path):
            with open(file_path, "r") as file:
                data = json.load(file)
            data.update(
                new_data
            )  # This merges new data with existing, updating entries where keys match
        else:
            data = new_data

        # Write the updated data back to the JSON file
        with open(file_path, "w") as file:
            json.dump(data, file, indent=4)

        print("Data appended to:", file_path)


if __name__ == "__main__":

    print("Arguments are :- ", sys.argv)

    cra_file = sys.argv[1]
    output_dir = sys.argv[2]
    parameters = sys.argv[3]
    account_size = sys.argv[4]
    daily_stats = sys.argv[5]
    name = sys.argv[6]
    latest_commit_id = sys.argv[7]

    utils = Utils(cra_file, output_dir)
    pnl_df, fill_df, order_df = utils.extract_files()
    pnl_analytics = PNLAnalytics(
        pnl_df, utils.unique_id, account_size=account_size, daily_stats=daily_stats
    )
    pnl_analytics.append_data_to_json(output_dir, parameters, name, latest_commit_id)
    pnl_analytics.create_pnl_plots(output_dir, pnl_analytics.unique_id, num_ticks=10)
    print("Results have been successfully generated")
