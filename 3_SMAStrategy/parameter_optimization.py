import subprocess
import sys
import os
import json
from collections import OrderedDict
from datetime import datetime
import numpy as np
import optuna


class ParameterOptimisation:
    def __init__(
        self,
        optim_id=None,
        results_path="/groupstorage/group07/SMACrossover/",
    ):
        self.optimisation_id = (
            np.random.randint(0, 1000000) if optim_id is None else optim_id
        )
        self.results_name = str(self.optimisation_id) + "results.json"
        self.results_path = results_path + self.results_name
        self.n_trials = 100
        print(f"Optimisation ID: {self.optimisation_id}")
        print(f"Stats stored at: {self.results_path}")

    def make_with_python(self, command, timeout=300):
        try:
            # Execute the command with check=True to automatically raise an exception for non-zero return codes
            subprocess.run(command, check=True, timeout=timeout)
            print("Command executed successfully.")
            print("-" * 50)
            return True  # Indicates success
        except:
            print("An error occurred.", sys.exc_info())
            return False

    def format_parameter_string(self, parameters: dict):
        return "|".join([f"{key}={value}" for key, value in parameters.items()])

    def get_recent_test_results(self):
        if os.path.exists(self.results_path):
            with open(self.results_path, "r") as file:
                data = json.load(file, object_pairs_hook=OrderedDict)
                return next(iter(data.items()))
        else:
            raise ValueError("Results file not found.")

    def check_data(self, stock, date_range, path_to_data):
        items = [f[2:] if f.startswith("._") else f for f in os.listdir(path_to_data)]
        unique_stocks = set([item.split("_")[1] for item in items])
        if stock not in unique_stocks:
            print(f"Stock {stock} not found in the data directory.")
            return False
        filtered_items = [item for item in items if stock in item]
        extracted_dates = [item.split("_")[2] for item in filtered_items]
        extracted_dates = [
            date[:4] + "-" + date[4:6] + "-" + date[6:8] for date in extracted_dates
        ]
        available_dates = self.filter_dates(
            extracted_dates, date_range[0], date_range[1]
        )
        if len(available_dates) == 0:
            print(f"No data available for stock {stock} in the specified date range.")
            return False
        else:
            print(f"Data available for stock {stock} in the specified date range.")
            print("Available dates:")
            for date in available_dates:
                print(date)
            print("-" * 50)
            return True

    def filter_dates(self, date_list, start_date, end_date):
        # Convert the start and end date strings to datetime.date objects
        start_date = datetime.strptime(start_date, "%Y-%m-%d").date()
        end_date = datetime.strptime(end_date, "%Y-%m-%d").date()

        # Filter the list to include only dates within the specified range
        print(f"Filtering dates between {start_date} and {end_date}")
        filtered_dates = []
        for date_str in date_list:
            try:
                date = datetime.strptime(date_str, "%Y-%m-%d").date()
                if start_date <= date <= end_date:
                    filtered_dates.append(date_str)
            except ValueError:
                continue
        return filtered_dates

    def run_optimisation(
        self,
        stock,
        date_range,
        path_to_data,
        parameter_dict,
        objective_parameter="total returns",
    ):
        if not self.check_data(stock, date_range, path_to_data):
            raise ValueError("Data not found for the specified stock and date range.")
        # restart the server
        if not self.make_with_python(["make", "start_server"]):
            raise ValueError("Server could not be started.")
        if not self.make_with_python(["make", "create_instance", f"SYMBOL={stock}"]):
            raise ValueError("Instance could not be created.")

        # Define the objective function to be minimized
        def objective(trial):
            current_parameters = {}
            for parameter in parameter_dict:
                if parameter_dict[parameter]["dtype"] == "category":
                    current_parameters[parameter] = trial.suggest_categorical(
                        parameter, parameter_dict[parameter]["values"]
                    )
                elif parameter_dict[parameter]["dtype"] == "int":
                    current_parameters[parameter] = trial.suggest_int(
                        parameter,
                        parameter_dict[parameter]["values"][0],
                        parameter_dict[parameter]["values"][1],
                    )
                else:
                    current_parameters[parameter] = trial.suggest_float(
                        parameter,
                        parameter_dict[parameter]["values"][0],
                        parameter_dict[parameter]["values"][1],
                    )

            param_string = self.format_parameter_string(current_parameters)
            # Run the backtest with the current parameters
            if not self.make_with_python(
                ["make", "edit_params", f"EPISODE_PARAMETERS={param_string}"]
            ):
                raise ValueError("Parameters could not be edited.")
            if not self.make_with_python(
                [
                    "make",
                    "run_backtest_with_results",
                    f"START_DATE={date_range[0]}",
                    f"END_DATE={date_range[1]}",
                    f"FULL_REPORTS=False",
                    f"DAILY_REPORTS=False",
                    f"RESULTS_NAME={self.results_name}",
                    f"PARAMETERS={param_string}",
                ]
            ):
                raise ValueError("Backtest could not be run.")
            _, score = self.get_recent_test_results()
            print(f"Score dict: {score}")
            score = score[objective_parameter]
            return -score

        # Run the optimization
        study = optuna.create_study()
        study.optimize(objective, n_trials=self.n_trials)

        best_parameters = study.best_params
        print("Best parameters:")
        print(best_parameters)
        print(f"highest score: {study.best_value}")

        best_param_string = self.format_parameter_string(best_parameters)
        self.make_with_python(
            ["make", "edit_params", f"EPISODE_PARAMETERS={best_param_string}"]
        )
        self.make_with_python(
            [
                "make",
                "run_backtest_with_results",
                f"START_DATE={date_range[0]}",
                f"END_DATE={date_range[1]}",
            ]
        )
        backtest_id, best_results = self.get_recent_test_results()
        print("backtest_id :- ", backtest_id)
        print("results of best optimisation: - ")
        print(best_results)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        optim = ParameterOptimisation()
    else:
        optim = ParameterOptimisation(int(sys.argv[1]))
        if len(sys.argv) == 3:
            optim.n_trials = int(sys.argv[2])

    parameters = {
        "shortPeriod": {"dtype": "int", "values": [5, 25]},
        "longPeriod": {"dtype": "int", "values": [30, 60]},
    }
    optim.run_optimisation(
        "BTCUSDT",
        ("2022-01-01", "2022-01-29"),
        "/groupstorage/group07/parsed_data_2022/",
        parameters,
    )
