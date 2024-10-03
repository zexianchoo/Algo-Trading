import pandas as pd
import numpy as np
import json
import sys
import os
# import time
import re
import sys
import glob

class Utils:
    def __init__(self):
        # Paths
        self.json_file_path = None
        self.cra_file_path = None

        # Dataframes
        self.csv_pnl = None
        self.csv_fill = None
        self.csv_order = None

        # Attributes from the cra file
        self.backtest_date = None
        self.unique_id = None
        # self.file_modified_datetime = None
        self.instance_name = None
        self.start_date = None
        self.end_date = None

        # New results template json
        self.new_results = None

        # Flags
        self.empty_file_found = False
        self.increment = False

    def extract_unique_id(self, file_path):
        file_name = os.path.basename(file_path)
        return file_name.split('_')[3]

    def find_file_by_unique_id(self, unique_id, pattern, path):
        regex_pattern = fr'BACK_.*_{unique_id}_.*\.csv'
        list_of_files = glob.glob(os.path.join(path, pattern))

        for file in list_of_files:
            if re.search(regex_pattern, file):
                return file
        return None

    def read_csv(self, file_path):
        return pd.read_csv(file_path)

    def filename_without_extension(self, file_path):
        return os.path.splitext(os.path.basename(file_path))[0]

    def extract_details_from_filename(self, file_path):
        pattern = r'BACK_([A-Za-z0-9]+)_([0-9]{4}-[0-9]{2}-[0-9]{2})_([0-9]+)_start_([0-9]{2}-[0-9]{2}-[0-9]{4})_end_([0-9]{2}-[0-9]{2}-[0-9]{4})'
        match = re.search(pattern, file_path)

        if match:
            self.instance_name = match.group(1)
            self.backtest_date = match.group(2)
            self.unique_id = match.group(3)
            self.start_date = match.group(4)
            self.end_date = match.group(5)
        else:
            raise ValueError("Results filename format doesn't match expected pattern")
        # self.file_modified_datetime = time.ctime(os.path.getmtime(file_path))

    def load_csv_files(self, cra_file_path):
        self.cra_file_path = cra_file_path
        csv_dir = 'csv_files'
        if not os.path.isdir(csv_dir):
            print(f"The directory {csv_dir} does not exist, no results not extracted.")
            # return None

        unique_id = self.extract_unique_id(cra_file_path)
        print(f"unique_id: {unique_id}")
        # Dictionary to store CSV file paths
        csv_paths = {
            'pnl': self.find_file_by_unique_id(unique_id, '*_pnl*.csv', 'csv_files'),
            'fill': self.find_file_by_unique_id(unique_id, '*_fill*.csv', 'csv_files'),
            'order': self.find_file_by_unique_id(unique_id, '*_order*.csv', 'csv_files')
        }

        # Check for filename matches and load CSVs
        matches = {}
        for key, csv_path in csv_paths.items():
            if csv_path:
                filename = self.filename_without_extension(csv_path)
                matches[key] = self.filename_without_extension(cra_file_path) in filename
                setattr(self, f'csv_{key}', self.read_csv(csv_path) if matches[key] else None)
            else:
                matches[key] = False

        # Check if all matches are True
        if not all(matches.values()):
            unmatched = [key for key, matched in matches.items() if not matched]
            raise ValueError(f"The cra file ID does not match the csv file IDs for: {', '.join(unmatched)}")

        # If all matches are True, proceed to extract details and initialize the results
        self.extract_details_from_filename(cra_file_path)
        self.new_results = {
            'backtest_date': self.backtest_date,
            'unique_id': self.unique_id,
            'repeat_count': 0,
            'instance_name': self.instance_name,
            'start_date': self.start_date,
            'end_date': self.end_date,
            'pnl': {}, 'fill': {}, 'orders': {}  # Empty dicts for now
        }

    def is_any_csv_empty(self):
        if self.csv_pnl is not None:
            if len(self.csv_pnl) == 0:
                print("No P&L recorded.")
                self.empty_file_found = True
        else:
            print("PnL CSV not loaded.")

        if self.csv_fill is not None:
            if len(self.csv_fill) == 0:
                print("No fills recorded.")
                self.empty_file_found = True
        else:
            print("Fill CSV not loaded.")

        if self.csv_order is not None:
            if len(self.csv_order) == 0:
                print("No orders recorded.")
                self.empty_file_found = True
        else:
            print("Order CSV not loaded.")

        if not self.empty_file_found:
            print("No empty CSV files found.")

    def read_json(self):
        if not os.path.exists(json_file_path):
            return None
        with open(json_file_path, 'r') as file:
            return json.load(file)

    def create_json(self):
        json_data = []
        json_data.insert(0, self.new_results)
        with open(self.json_file_path, 'w') as file:
            json.dump(json_data, file, indent=4)

    def update_json(self):        
        json_data = self.read_json()     
        if self.increment:
            json_data[0]['repeat_count'] += 1
            self.new_results = json_data[0]
        else:
            json_data.insert(0, self.new_results)
        with open(self.json_file_path, 'w') as file:
            json.dump(json_data, file, indent=4)

    def check_and_update_json(self, json_file_path):
        self.json_file_path = json_file_path
        json_data = self.read_json()

        if json_data:
            first_entry = json_data[0]
            self.increment = (first_entry['backtest_date'] == self.backtest_date and
                              first_entry['unique_id'] == self.unique_id and
                              first_entry['instance_name'] == self.instance_name and
                              first_entry['start_date'] == self.start_date and
                              first_entry['end_date'] == self.end_date)

            if self.increment:
                print(f"New result same as previous result, repeat_count being incremented...")
            else:
                print("This instance has not been added to the JSON. Inserting now...")
            self.update_json()
        else:
            print("JSON file does not exist. Creating file now...")
            self.create_json()

class PnLAnalytics:
    def __init__(self):
        self.risk_free_rate = 0.0
        self.sharpe_ratio = 0.0
        self.max_drawdown = 0.0

    def calculate_sharpe_ratio(self, returns):
        # Adjust risk-free rate for the number of periods
        rf_per_period = (1 + self.risk_free_rate)**(1/252) - 1  # Assuming 252 trading days in a year
        excess_returns = returns - rf_per_period
        sharpe_ratio = np.mean(excess_returns) / np.std(excess_returns)
        return sharpe_ratio

    def calculate_max_drawdown(self, cumulative_pnl):
        rolling_max = cumulative_pnl.cummax()
        drawdowns = cumulative_pnl / rolling_max - 1.0
        max_drawdown = drawdowns.min()
        return max_drawdown

    def calculate_net_pnl(self, cumulative_pnl):
        net_pnl = cumulative_pnl.iloc[-1] - cumulative_pnl.iloc[0]
        return net_pnl


if __name__ == '__main__':

    os.chdir(os.path.expanduser('~/ss/bt/backtesting-results/'))
    utils = Utils()
    result_updated = False

    if len(sys.argv) > 2:
        cra_filepath = sys.argv[1]
        json_file_path = sys.argv[2]

        utils.load_csv_files(cra_filepath)
        utils.is_any_csv_empty()

        if utils.empty_file_found:
            print("Empty file/s found. Check your strategy.")
        else:
            analytics = PnLAnalytics()
            analytics.risk_free_rate = 0.01
            utils.new_results['pnl']['sharpe_ratio'] = analytics.calculate_sharpe_ratio(utils.csv_pnl['Cumulative PnL'])
            print(f"New results:\n{utils.new_results}")
            utils.check_and_update_json(json_file_path)
            result_updated = True
    else:
        print("Missing arguments. Usage: python3 results_analytics.py <cra_filepath> <json_file_path>")
        sys.exit(1)

    sys.exit(0 if result_updated else 1)
