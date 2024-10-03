# Guide to Running Projects with Strategy Studio

Authored by: Samanvay Malapally Sudhakara [MSFE '24, UIUC] [linkedin](https://www.linkedin.com/in/samanvay-malapally-sudhakara-148836212/)

## Table of Contents

1. [Introduction](#1-introduction)
2. [Overview of Strategy Studio](#2-overview-of-strategy-studio)
3. [Building Strategies with Strategy Studio](#3-building-strategies-with-strategy-studio)
   1. [Strategy Directory](#31-strategy-directory)
   2. [Makefile and Command Guide](#32-makefile-and-command-guide)
4. [Running with Python Scripts](#4-running-with-python-scripts)
    1. [Automated Makefile Command Execution Script](#41-automated-makefile-command-execution-script)
    2. [Automated Makefile Parameter Optimisation Script](#42-automated-makefile-parameter-optimisation-script)
5. [Provision Scripts](#5-provision-scripts)
    1. [Start Server Script (`start_server.sh`)](#51-start-server-script-start_serversh)
    2. [Git Pull Script (`git_pull.sh`)](#52-git-pull-script-git_pullsh)
    3. [Create Instance Script (`create_instance.sh`)](#53-create-instance-script-create_instancesh)
    4. [Delete Instance Script (`delete_instance.py`)](#54-delete-instance-script-delete_instancepy)
    5. [Run Strategy Script (`run_backtest.sh`)](#55-run-strategy-script-run_backtestsh)
    6. [Generate Results Script (`generate_results.sh`)](#56-generate-results-script-generate_resultssh)
    7. [Edit Parameters Script (`edit_parameters.sh`)](#57-edit-parameters-script-edit_parameterssh)

## 1 Introduction

This document provides a guide to running projects with Strategy Studio. It is intended for users who want to learn more on how to run projects with Strategy Studio and how to modfiy the code to suit their needs.

## 2 Overview of Strategy Studio

Strategy Studio is a high performance C++ multi asset class strategy
development platform, designed for efficient implementation, testing, and deployment of
algorithmic trading strategies. Strategy Studio provides a Strategy Development API
which is agnostic to data feed and execution providers, allowing you to focus on the
logic of your strategies. Strategy Studio also includes a set of Strategy Servers, which
are responsible for loading and running your trading strategies, along with the Strategy
Manager, a performance monitoring and risk management user interface that you can
use to centralize your research and production trading.

## 3 Building Strategies with Strategy Studio

### 3.1 Strategy Directory

Your repository should have a directory structure similar to the following:

```bash
group_<num>_project/
├── provision_scripts/
|   ├── create_strategy.sh
|   ├── delete_instance.py
|   ├── generate_results.sh
|   ├── edit_parameters.sh
|   ├── get_results.py
|   ├── git_pull.sh
|   ├── run_strategy.sh
|   ├── start_server.sh
├── <StrategyName>/
|   ├── <StrategyName>.cpp
|   ├── <StrategyName>.h
|   ├── Makefile
|   ├── <PythonScripts>.py
├── <Strategy2Name>/
.....
```

In group storage your directory should look like this:

```bash
/group_storage/group_<num>/
├── <StrategyName>/
|   ├── results/
|   |   ├── *.cra
|   |   ├── *_fill.csv
|   |   ├── *_order.csv
|   |   ├── *_trade.csv
|   ├── result_plots/
|   ├── results.json
├── <Strategy2Name>/
.....
```

## 3.2 Makefile and Command Guide

The Makefile is used to compile your strategy and facilitate the execution of strategy-related commands in an error-free manner. It should be located in the same directory as your strategy files.

### Prerequisites

Before you begin, ensure the following:

- You have the necessary permissions to execute scripts in your environment.
- All paths and dependencies in the Makefile are correctly set up according to your system configuration.

### Key Variables to Update

When running new strategies, you should update these variables in the Makefile:

- **`INSTANCE_NAME` and `STRATEGY_NAME`**: These should be set to the name of your strategy. Use a single word with no special characters. Typically, this should match the base name of your strategy's source code file.
  
- **`START_DATE` and `END_DATE`**: These variables define the period over which the strategy will be backtested. They should be formatted as `YYYY-MM-DD`.

- **`SYMBOLS`**: Set this to the symbols that your strategy will trade.

- **`PARAMETERS`**: Strategy-specific parameters formatted as `key=value` pairs separated by `|`. Adjust these parameters based on the strategy's requirements.

- **`ACCOUNTSIZE`**: This should reflect the virtual account size for the backtesting simulation.

- **`WORKINGDIR`**: The working directory where your strategy and scripts are located. This needs to be an absolute path.

- **`OUTPUT_DIR`**: Path where the output from scripts will be stored. Make sure it exists or that your scripts have the ability to create directories.

### Compiling the Strategy

To compile the strategy, use the command:

`all`: This command triggers the compilation of your strategy's source code into a shared library (.so file), considering the compiler and flags set based on the presence of `INTEL` and `DEBUG` flags.

### Managing Strategy Execution

Various custom commands are defined in the Makefile to simplify strategy management:

`make_executable`: Ensures that all scripts in the provision_scripts directory are executable.
`delete_instance`: Deletes the strategy instance from the database to ensure a clean slate for the next run.
`clean_dlls`: Removes old dynamic link library (DLL) files to prevent conflicts with new compilations.
`move_strategy_dll`: Moves the newly compiled DLL to the appropriate directory for execution.
`start_server`: Starts the server necessary for running the strategy after moving the DLL.

### Running the Strategy

To run your strategy backtest:

`create_instance`: Creates a new instance of the strategy with the specified parameters.
`run_backtest`: Executes the backtest for the given date range without processing the results.
`run_backtest_with_results`: Executes the backtest and also processes the results, generating output and graphs in the specified format and location.
`edit_params`: Allows for editing the parameters of an existing strategy instance to fine-tune strategy behavior.

### Additional Notes

It's important to ensure that paths to libraries and include directories in the Makefile correctly point to the locations in your environment.
Adjust the LDFLAGS and INCLUDES to include any additional libraries and headers your strategy might depend on.

### Example Usage on Command Line

1. To compile the strategy and start the server and mount the strategy, run the following command:

    ```bash
    make start_server
    ```

2. To create a new strategy instance, run the following command:

    ```bash
    make create_instance
    ```

3. To run the backtest for the specified date range, run the following command:

    ```bash
    make run_backtest
    ```

4. To run the backtest and generate results, run the following command:

    ```bash
    make run_backtest_with_results
    ```

5. To edit the parameters of an existing strategy instance, run the following command:

    ```bash
    make edit_params
    ```

then run the either of the two commands in step 3 or 4 based on your requirements.

Note:- keep in mind the server needs to be re-initialized after editing the core strategy logic but you can test different parameters and dates by either creating new instances or editing parameters of current instance without restarting the server.
(refer to automated python scripts below for more info)

## 4 running with python scripts

### 4.1 Automated Makefile Command Execution Script

This Python script, named `make_with_python`, automates the execution of Makefile commands for managing and running strategy backtests. It leverages the `subprocess` module to execute commands, ensuring clean error handling and structured output.

#### Functionality

- **Error Handling**: Handles execution errors, including non-zero return statuses and timeouts, with specific error messages.
- **Command Flexibility**: Supports conditional execution paths based on input arguments to cater to different operational needs.
- **Structured Output**: Captures and prints stdout and stderr for a clear understanding of command execution status.

#### Script Details

##### How It Works

1. **Function `make_with_python(command)`**:
   - Accepts a command list (e.g., `["make", "create_instance"]`).
   - Executes commands using `subprocess.run`, with outputs managed to avoid clutter.
   - Includes error handling for non-zero exit codes, timeouts, and unexpected errors.

2. **Main Execution Logic**:
   - The script determines the command sequence based on the provided argument (`sys.argv[1]`).
   - Executes basic operations (start server, create instance, run backtest) if the argument is `"0"`.
   - Performs advanced operations (edit parameters, set dates) if any other argument is provided.
   - The timeout for each command is set to by the second argument (`sys.argv[2]`) and must be configured based on execution time of one day of backtest and the number of days in the backtest.
   - If any command fails, the script terminates immediately, signaling an error.

3. **Usage**:
   - Place the script in the same directory as your Makefile.
   - Execute using Python with an appropriate argument to trigger different command sequences:

     ```bash
     python make_with_python.py 0 100 # Basic operations with timeout of 100 seconds
     python make_with_python.py 1 200 # Advanced operations with parameter editing with timeout of 200 seconds
     ```

#### Use Case

Ideal for automated testing or deployment environments where multiple make commands need to be executed in sequence under varying conditions. The script ensures that each step is handled correctly, providing a robust tool for strategy deployment and testing.

### 4.2 Automated Makefile Parameter Optimisation Script

The `parameter_optimisation.py` script is designed to optimize trading strategy parameters automatically. The optimization process involves executing the strategy multiple times with varying parameters to identify the best-performing configuration based on specific evaluation criteria.

#### Functionality

The main functionality revolves around parameter tuning for trading strategies using the `optuna` library. It leverages Makefiles to automate various tasks like starting the server, creating trading instances, editing parameters, and running backtests.

##### Optuna

Optuna is an open-source hyperparameter optimization framework designed for efficiency and ease of use. It offers features like automatic pruning of unpromising trials, which helps in efficiently finding the best hyperparameters. The library uses a state-of-the-art algorithm called Tree-structured Parzen Estimator (TPE) for Bayesian optimization but also supports grid and random searches. With its intuitive API, Optuna integrates seamlessly with machine learning frameworks like scikit-learn and PyTorch, making it an ideal tool for data scientists and researchers to automate hyperparameter tuning and improve model performance.

#### Script Details

- **Initialization**: When initializing a `ParameterOptimisation` object, the script assigns a unique optimization ID, sets the path for storing results, and defines the number of trials for optimization.
- **Helper Functions**:
  - `make_with_python`: Executes shell commands using `subprocess` with error handling.
  - `format_parameter_string`: Converts a dictionary of parameters into a formatted string.
  - `get_recent_test_results`: Retrieves the most recent backtest results from the specified file.
  - `check_data`: Validates if the stock data for the specified date range exists in the given path.
  - `filter_dates`: Filters dates within a given range.

#### How It Works

The `run_optimisation` function coordinates the optimization process:

1. **Data Validation**: Checks if the required data for the specified stock and date range is available.
2. **Server Setup**: Restarts the server and creates a trading instance for the specified stock.
3. **Objective Function**: Defines the objective for the `optuna` study, adjusting parameters and executing backtests to get scores.
4. **Optimization**: Runs the `optuna` optimization process for a specified number of trials.
5. **Final Backtest**: Conducts a final backtest with the optimized parameters and prints the results.

#### Use Case

This script is tailored for algorithmic traders who want to systematically identify the best-performing parameters for their trading strategies. By integrating automated backtesting and parameter tuning, it helps traders refine their strategies for better performance.

## 5 Provision Scripts

The provision scripts are used to automate the process of running the strategies. The scripts are located in the `provision_scripts` directory and are used to create, run, and manage the strategies.

### 5.1 Start Server Script (`start_server.sh`)

The `start_server.sh` script is designed to automate the process of starting the strategy server, which is essential for running strategies within the trading simulation environment. It navigates to the utility directory where `StrategyCommandLine` tool is located and first kills any existing server processes to ensure a clean start. Then, it starts the server.

### 5.2 Git Pull Script (`git_pull.sh`)

The `git_pull.sh` script is designed to automate the process of updating the local repository with the latest changes from the remote repository. It navigates to the root directory of the repository and executes a `git reset --hard` to remove any local changes. Then, it performs a `git pull` to fetch the latest changes from the remote repository and updates the local repository accordingly.

### 5.3 Create Instance Script (`create_instance.sh`)

The `create_instance.sh` script is designed to automate the creation of strategy instances within the trading simulation environment. It handles several preliminary checks and the final creation of a strategy instance configured with specific parameters and symbols.
This script begins by navigating to the utility directory where `StrategyCommandLine` tool is located and commands are executed. It performs a recheck of all strategies to ensure the system is up to date. Then, it creates a new strategy instance with detailed user-specified configurations such as strategy name, instance name, trading symbols, parameters, and account size.

### 5.4 Delete Instance Script (`delete_instance.py`)

The `delete_instance.py` script is designed to facilitate the removal of strategy instances from a SQLite database, ensuring that your system maintains clean data management and prevents clutter or conflicts with strategy deployment. The script operates within the database directory and offers flexibility in how data can be purged.
This Python script connects to a SQLite database that stores strategy instances and types. It provides two operational modes:

1. **Selective Deletion**: Targeted removal of a specific instance and its corresponding strategy type based on input parameters.
2. **Complete Purge**: Wipes all records from the strategy instance and type tables, effectively resetting the database status regarding strategies.

### 5.5 Run Strategy Script (`run_backtest.sh`)

The `run_backtest.sh` script is designed to facilitate the execution of backtests over a specified date range for a given strategy instance. It automates the backtest process, including checking for working days, initiating the backtest, monitoring the progress, and handling completion.

This Bash script performs several key functions:

- **Starts a backtest** for the specified instance and date range using the StrategyCommandLine utility.
- **Calculates the number of working days** (Monday to Friday) between the provided start and end dates to estimate the duration of the backtest.
- **Monitors the log file** for completion indicators and extracts relevant output data upon completion.

### 5.6 Generate Results Script (`generate_results.sh`)

The `generate_results.sh` script is designed to facilitate the process of running backtests and exporting comprehensive results for a given strategy instance. It manages the execution of a backtest, the relocation of resultant `.cra` files, and the generation of detailed backtest reports based on these files.

This Bash script automates several critical functions:

- **Running a backtest** using a specified instance, date range, and additional parameters.
- **Moving the resulting `.cra` file** to a designated output directory.
- **Generating detailed reports** from the `.cra` file using a Python script.
- It ensures that all operations are logged and that any failures are clearly reported to the user.

### 5.7 Edit Parameters Script (`edit_parameters.sh`)

The `edit_parameters.sh` script is tailored for updating the parameters of a specific strategy instance dynamically. It simplifies the process of modifying strategy settings without the need for direct database manipulation or manual configuration file edits.

This Bash script functions primarily to:

- **Modify strategy parameters** for an existing instance using the `StrategyCommandLine` tool.
- **Provide immediate feedback** on the success of the parameter changes.

### 5.8 Get Results Script (`get_results.py`)

The `get_results.py` script is tailored to process and analyze backtesting results, particularly focusing on financial performance metrics such as PnL (Profit and Loss), Sharpe ratio, and maximum drawdown. The script reads in backtest output files, performs statistical calculations, generates visual plots of the results, and appends performance data to a JSON file for easy review and record-keeping.

This script executes several key functions:

- **Extracts and reads data**: Retrieves PnL, order, and fill data from CSV files generated by a backtest.
- **Calculates performance metrics**: Computes critical financial metrics such as the Sharpe ratio, Sortino ratio, and maximum drawdown.
- **Generates plots**: Creates detailed plots for account value over time, returns distribution, and other relevant financial metrics.
- **Outputs results to JSON**: Appends the new backtest results to an existing JSON file, allowing for a cumulative view of performance over time.

## Additional Resources

given the makefile and provision scripts in the regression mean reversion strategy, the following are the expected outputs on running the scripts.

### Expected output on running `make start_server`

```plaintext
chmod +x /home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/*.sh
chmod +x /home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/*.py
pypy3 /home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/delete_instance.py "RegressionMeanReversion" "RegressionMeanReversion"
Python script at: /home/vagrant/ss/bt/db
Deleting all instances of all strategies from the sqlite database.
Displaying the contents of StrategyInstance & StrategyType tables: (should be empty if deleted successfully)
[]
[]
mkdir -p /home/vagrant/ss/bt/strategies_dlls
rm -rf /home/vagrant/ss/bt/strategies_dlls/*
rm -rf *.o RegressionMeanReversion.so
g++ -c -fPIC -fpermissive -O3 -std=c++17 -I/usr/include -I/home/vagrant/ss/sdk/RCM/StrategyStudio/includes -I/home/vagrant/ss/sdk/RCM/StrategyStudio/includes/  RegressionMeanReversion.cpp -o RegressionMeanReversion.o
g++ -shared -Wl,-soname,RegressionMeanReversion.so.1 -o RegressionMeanReversion.so RegressionMeanReversion.o /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_analytics.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_transport.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_marketmodels.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_utilities.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_flashprotocol.a
cp RegressionMeanReversion.so /home/vagrant/ss/bt/strategies_dlls/.
/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/start_server.sh
Starting server
Error occurred on connection: Connection refused
Could not connect to server
Initializing Strategy Studio Backtesting Server...
Initializing Strategy Server...
PROCESSOR_LOG_LEVEL configured as DEBUG
Strategy Studio Server v 3.4.0.0
Protocol Version v21.0
Recovery mode disabled.
Loading corporate action adapter dll /home/vagrant/ss/bt/./corporate_action_adapters/TextCorporateActionAdapter.so...
Loading instrument information from reference database...
Loading equity instruments...
Finished loading 0 equity instruments from reference database.
Loading futures instruments...
Finished loading 0 futures instruments from reference database.
Loading options instruments...
Finished loading 0 option instruments from reference database.
Loading spot currency pair instruments...
Finished loading 0 spot currency pairs from reference database.
Loading bond instruments...
Finished loading 0 bond instruments from reference database.
Finished loading instrument information from reference database.
Starting Seq Num = 1
Loading strategy dll /home/vagrant/ss/bt/strategies_dlls/RegressionMeanReversion.so...
Found new strategy type info for strategy RegressionMeanReversion
Loaded strategy type RegressionMeanReversion from strategy DLL.
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/NomuraTickReader.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/OneTickAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/TextTickReader.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/LMAXAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/TickDataDotComAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/NanexAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/MorningstarTickAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/KDBAdapter.so...
Application state changed to STATE_APP_INITIALIZED
Strategy Server initialized...
Starting Strategy Studio Backtesting Server...
Starting Strategy Server...
Application state changed to STATE_APP_STARTING
Application state changed to STATE_MARKET_OPENING
Acceptor thread started.
Application state changed to STATE_RUNNING
Strategy Server started...
Running Strategy Studio Backtesting Server...
Command line disabled. Type CTRL-C to end.
Started server
 **************************************************************************************************** 
```

### Expected output on running `make create_instance`

```plaintext
/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/create_instance.sh "RegressionMeanReversion" "RegressionMeanReversion" "XNAS.ITCH-AAPL" "absolute_stop=0.2|strategy_active=true|debug=false|max_inventory=100|window_size=30|previous_prediction=0|inventory_liquidation_increment=10|inventory_liquidation_interval=10|bar_interval=1" "1000000"
Creating instance
rechecking strategies
Login received from connection id 0

Re-checking for additional strategies...
No strategies found to initialize!

Quitting.
Connection id 0 finished.
Connection id 1 finished.
creating new instance
Login received from connection id 1
Strategy RegressionMeanReversion added to processor 0.

Quitting.
Connection id 1 finished.
Connection id 1 finished.
Created instance
active instance list:- 
Login received from connection id 2

--Strategy Instance List--
RegressionMeanReversion
----------------------------


Quitting.
Connection id 2 finished.
Connection id 1 finished.
 **************************************************************************************************** 
```

### Expected output on running `make run_backtest`

```plaintext
/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/run_backtest.sh "RegressionMeanReversion" "2023-10-20" "2023-10-25"
RegressionMeanReversion
2023-10-20
2023-10-25
Login received from connection id 3
Starting backtest experiment 1 running 1 strategy with start date 2023-Oct-20 and end date 2023-Oct-25
DataSourceMgr::BuildDataSource: Received request to build data source BacktesterReader for processor id 1
Started backtest experiment 1 with start date 2023-Oct-20 and end date 2023-Oct-25

Quitting.
Connection id 3 finished.
Connection id 1 finished.
MarketState_CLOSE
MarketState_PREOPEN
running backtest for 4 working days
Sleeping for 100 seconds
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 1 completed in 27.5199 secsand processed 5959755 ticks for a throughput of 216562 ticks per second
Starting day 2023-Oct-23 of backtest experiment 1
MarketState_CLOSE
MarketState_PREOPEN
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 1 completed in 24.9337 secsand processed 5507525 ticks for a throughput of 220887 ticks per second
Starting day 2023-Oct-24 of backtest experiment 1
MarketState_CLOSE
MarketState_PREOPEN
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 1 completed in 23.1005 secsand processed 4944521 ticks for a throughput of 214044 ticks per second
Starting day 2023-Oct-25 of backtest experiment 1
MarketState_CLOSE
MarketState_PREOPEN
MarketState_OPEN
MarketState_CLOSE
Waiting for strategy to finish


Text Backtest experiment 1 completed in 24.6445 secsand processed 5451042 ticks for a throughput of 221187 ticks per second
Backtest experiment 1 finished.
found CRA file :- 
BACK_RegressionMeanReversion_2024-04-20_232421_start_10-20-2023_end_10-25-2023.cra
Strategy finished
run backtest complete
 **************************************************************************************************** 
```

### Expected output on running `make run_backtest_with_results`

```plaintext
/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/generate_results.sh "/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project" "RegressionMeanReversion" "2023-10-20" "2023-10-25" "absolute_stop=0.2|strategy_active=true|debug=false|max_inventory=100|window_size=30|previous_prediction=0|inventory_liquidation_increment=10|inventory_liquidation_interval=10|bar_interval=1" "/groupstorage/group01/RegressionMeanReversion" "True" "1000000" "True" "results.json"
Login received from connection id 4
Starting backtest experiment 2 running 1 strategy with start date 2023-Oct-20 and end date 2023-Oct-25
DataSourceMgr::BuildDataSource: Received request to build data source BacktesterReader for processor id 2
Connection id 4 finished.
MarketState_CLOSE
MarketState_PREOPEN
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 2 completed in 25.9922 secsand processed 5959755 ticks for a throughput of 229290 ticks per second
Starting day 2023-Oct-23 of backtest experiment 2
MarketState_CLOSE
MarketState_PREOPEN
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 2 completed in 24.6326 secsand processed 5507525 ticks for a throughput of 223587 ticks per second
Starting day 2023-Oct-24 of backtest experiment 2
MarketState_CLOSE
MarketState_PREOPEN
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 2 completed in 21.3737 secsand processed 4944521 ticks for a throughput of 231336 ticks per second
Starting day 2023-Oct-25 of backtest experiment 2
MarketState_CLOSE
MarketState_PREOPEN
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 2 completed in 24.8993 secsand processed 5451042 ticks for a throughput of 218923 ticks per second
Backtest experiment 2 finished.
cra file name: BACK_RegressionMeanReversion_2024-04-20_232823_start_10-20-2023_end_10-25-2023.cra
Login received from connection id 5

export successful

Quitting.
Connection id 5 finished.
Connection id 1 finished.
output directory: /groupstorage/group01/RegressionMeanReversion/results/
****************************************************************************************************
Arguments are :-  ['get_results.py', 'BACK_RegressionMeanReversion_2024-04-20_232823_start_10-20-2023_end_10-25-2023.cra', '/groupstorage/group01/RegressionMeanReversion', '"absolute_stop=0.2|strategy_active=true|debug=false|max_inventory=100|window_size=30|previous_prediction=0|inventory_liquidation_increment=10|inventory_liquidation_interval=10|bar_interval=1"', '1000000', 'True', 'results.json', '47ed9daca8a97efddbf3d7bb61e33152c79b6892']
unique_id is :-  232823
new_data is :-  {'232823': {'commit_id': '47ed9daca8a97efddbf3d7bb61e33152c79b6892', 'total pnl': 377.308071, 'total returns': 1000377.308071, 'sharpe': 0.7374198212245463, 'sortino': 1.8436000087271052, 'max_drawdown': -0.00039280779597507763, 'parameters': {'"absolute_stop': '0.2', 'strategy_active': 'true', 'debug': 'false', 'max_inventory': '100', 'window_size': '30', 'previous_prediction': '0', 'inventory_liquidation_increment': '10', 'inventory_liquidation_interval': '10', 'bar_interval': '1"'}, 'dates': ['2023-10-25', '2023-10-24', '2023-10-20', '2023-10-23'], 'daily_stats': {'2023-10-25': {'sharpe': 1.1711288248988407, 'sortino': 5.899409964686414, 'max_drawdown': -0.00021097615969395457}, '2023-10-24': {'sharpe': -0.48064775072798366, 'sortino': -2.6630455751339523, 'max_drawdown': -0.00021496431592355668}, '2023-10-20': {'sharpe': 2.630712295789369, 'sortino': 16.785526876272776, 'max_drawdown': -0.0002019543583150208}, '2023-10-23': {'sharpe': -1.5991297147455668, 'sortino': -12.302148421189115, 'max_drawdown': -0.0003929500953378921}}}}
Data appended to: /groupstorage/group01/RegressionMeanReversion/results.json
Results have been successfully generated
```

### Expected output on running `python3 make_with_python.py 0 150`

```plaintext
chmod +x /home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/*.sh
chmod +x /home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/*.py
pypy3 /home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/delete_instance.py "RegressionMeanReversion" "RegressionMeanReversion"
Python script at: /home/vagrant/ss/bt/db
Deleting all instances of all strategies from the sqlite database.
Displaying the contents of StrategyInstance & StrategyType tables: (should be empty if deleted successfully)
[]
[]
mkdir -p /home/vagrant/ss/bt/strategies_dlls
rm -rf /home/vagrant/ss/bt/strategies_dlls/*
rm -rf *.o RegressionMeanReversion.so
g++ -c -fPIC -fpermissive -O3 -std=c++17 -I/usr/include -I/home/vagrant/ss/sdk/RCM/StrategyStudio/includes -I/home/vagrant/ss/sdk/RCM/StrategyStudio/includes/  RegressionMeanReversion.cpp -o RegressionMeanReversion.o
g++ -shared -Wl,-soname,RegressionMeanReversion.so.1 -o RegressionMeanReversion.so RegressionMeanReversion.o /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_analytics.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_transport.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_marketmodels.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_utilities.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_flashprotocol.a
cp RegressionMeanReversion.so /home/vagrant/ss/bt/strategies_dlls/.
/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/start_server.sh
Starting server

Quitting.
Connection id 1 finished.
Initializing Strategy Studio Backtesting Server...
Initializing Strategy Server...
PROCESSOR_LOG_LEVEL configured as DEBUG
Strategy Studio Server v 3.4.0.0
Protocol Version v21.0
Recovery mode disabled.
Loading corporate action adapter dll /home/vagrant/ss/bt/./corporate_action_adapters/TextCorporateActionAdapter.so...
Loading instrument information from reference database...
Loading equity instruments...
Finished loading 0 equity instruments from reference database.
Loading futures instruments...
Finished loading 0 futures instruments from reference database.
Loading options instruments...
Finished loading 0 option instruments from reference database.
Loading spot currency pair instruments...
Finished loading 0 spot currency pairs from reference database.
Loading bond instruments...
Finished loading 0 bond instruments from reference database.
Finished loading instrument information from reference database.
Starting Seq Num = 1
Loading strategy dll /home/vagrant/ss/bt/strategies_dlls/RegressionMeanReversion.so...
Found new strategy type info for strategy RegressionMeanReversion
Loaded strategy type RegressionMeanReversion from strategy DLL.
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/NomuraTickReader.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/OneTickAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/TextTickReader.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/LMAXAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/TickDataDotComAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/NanexAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/MorningstarTickAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/KDBAdapter.so...
Application state changed to STATE_APP_INITIALIZED
Strategy Server initialized...
Starting Strategy Studio Backtesting Server...
Starting Strategy Server...
Application state changed to STATE_APP_STARTING
Application state changed to STATE_MARKET_OPENING
Application state changed to STATE_RUNNING
Strategy Server started...
Running Strategy Studio Backtesting Server...
Command line disabled. Type CTRL-C to end.
Acceptor thread started.
Started server
 **************************************************************************************************** 
Command executed successfully.
/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/create_instance.sh "RegressionMeanReversion" "RegressionMeanReversion" "XNAS.ITCH-AAPL" "absolute_stop=0.2|strategy_active=true|debug=false|max_inventory=100|window_size=30|previous_prediction=0|inventory_liquidation_increment=10|inventory_liquidation_interval=10|bar_interval=1" "1000000"
Creating instance
rechecking strategies
Login received from connection id 0

Re-checking for additional strategies...

Quitting.
No strategies found to initialize!
Connection id 0 finished.
Connection id 1 finished.
creating new instance
Login received from connection id 1
Strategy RegressionMeanReversion added to processor 0.

Quitting.
Connection id 1 finished.
Connection id 1 finished.
Created instance
active instance list:- 
Login received from connection id 2

--Strategy Instance List--
RegressionMeanReversion
----------------------------


Quitting.
Connection id 2 finished.
Connection id 1 finished.
 **************************************************************************************************** 
Command executed successfully.
/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/run_backtest.sh "RegressionMeanReversion" "2023-10-20" "2023-10-25"
RegressionMeanReversion
2023-10-20
2023-10-25
Login received from connection id 3
Starting backtest experiment 1 running 1 strategy with start date 2023-Oct-20 and end date 2023-Oct-25
DataSourceMgr::BuildDataSource: Received request to build data source BacktesterReader for processor id 1
Started backtest experiment 1 with start date 2023-Oct-20 and end date 2023-Oct-25
Quitting.

Connection id 1 finished.
Connection id 3 finished.
MarketState_CLOSE
Total workdays: 4
MarketState_PREOPEN
running backtest for 4 working days
Sleeping for 100 seconds
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 1 completed in 25.926 secsand processed 5959755 ticks for a throughput of 229875 ticks per second
Starting day 2023-Oct-23 of backtest experiment 1
MarketState_CLOSE
MarketState_PREOPEN
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 1 completed in 23.6222 secsand processed 5507525 ticks for a throughput of 233150 ticks per second
Starting day 2023-Oct-24 of backtest experiment 1
MarketState_CLOSE
MarketState_PREOPEN
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 1 completed in 21.5481 secsand processed 4944521 ticks for a throughput of 229464 ticks per second
Starting day 2023-Oct-25 of backtest experiment 1
MarketState_CLOSE
MarketState_PREOPEN
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 1 completed in 23.3959 secsand processed 5451042 ticks for a throughput of 232991 ticks per second
Backtest experiment 1 finished.
found CRA file :- 
BACK_RegressionMeanReversion_2024-05-02_014119_start_10-20-2023_end_10-25-2023.cra
Strategy finished
run backtest complete
 **************************************************************************************************** 
Command executed successfully.
```

### Expected output on running `python3 make_with_python.py 1 150`

```plaintext
python3 make_with_python.py 1 150
chmod +x /home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/*.sh
chmod +x /home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/*.py
pypy3 /home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/delete_instance.py "RegressionMeanReversion" "RegressionMeanReversion"
Python script at: /home/vagrant/ss/bt/db
Deleting all instances of all strategies from the sqlite database.
Displaying the contents of StrategyInstance & StrategyType tables: (should be empty if deleted successfully)
[]
[]
mkdir -p /home/vagrant/ss/bt/strategies_dlls
rm -rf /home/vagrant/ss/bt/strategies_dlls/*
rm -rf *.o RegressionMeanReversion.so
g++ -c -fPIC -fpermissive -O3 -std=c++17 -I/usr/include -I/home/vagrant/ss/sdk/RCM/StrategyStudio/includes -I/home/vagrant/ss/sdk/RCM/StrategyStudio/includes/  RegressionMeanReversion.cpp -o RegressionMeanReversion.o
g++ -shared -Wl,-soname,RegressionMeanReversion.so.1 -o RegressionMeanReversion.so RegressionMeanReversion.o /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_analytics.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_transport.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_marketmodels.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_utilities.a /home/vagrant/ss/sdk/RCM/StrategyStudio/libs/x64/libstrategystudio_flashprotocol.a
cp RegressionMeanReversion.so /home/vagrant/ss/bt/strategies_dlls/.
/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/start_server.sh
Starting server
Login received from connection id 4
Stopping Strategy Studio Backtesting Server...
Stopping Strategy Server...
Application state changed to STATE_APP_STOPPING

Quitting.
Connection id 4 finished.
Connection id 1 finished.
Acceptor thread stopped.
Application state changed to STATE_SHUT_DOWN
Strategy Server stopped...

MessageStore message count = 105526
Initializing Strategy Studio Backtesting Server...
Initializing Strategy Server...
PROCESSOR_LOG_LEVEL configured as DEBUG
Strategy Studio Server v 3.4.0.0
Protocol Version v21.0
Recovery mode disabled.
Loading corporate action adapter dll /home/vagrant/ss/bt/./corporate_action_adapters/TextCorporateActionAdapter.so...
Loading instrument information from reference database...
Loading equity instruments...
Finished loading 0 equity instruments from reference database.
Loading futures instruments...
Finished loading 0 futures instruments from reference database.
Loading options instruments...
Finished loading 0 option instruments from reference database.
Loading spot currency pair instruments...
Finished loading 0 spot currency pairs from reference database.
Loading bond instruments...
Finished loading 0 bond instruments from reference database.
Finished loading instrument information from reference database.
Starting Seq Num = 1
Loading strategy dll /home/vagrant/ss/bt/strategies_dlls/RegressionMeanReversion.so...
Found new strategy type info for strategy RegressionMeanReversion
Loaded strategy type RegressionMeanReversion from strategy DLL.
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/NomuraTickReader.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/OneTickAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/TextTickReader.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/LMAXAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/TickDataDotComAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/NanexAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/MorningstarTickAdapter.so...
Loading backtest reader library /home/vagrant/ss/bt/./backtester-readers/KDBAdapter.so...
Application state changed to STATE_APP_INITIALIZED
Strategy Server initialized...
Starting Strategy Studio Backtesting Server...
Starting Strategy Server...
Application state changed to STATE_APP_STARTING
Application state changed to STATE_MARKET_OPENING
Acceptor thread started.
Application state changed to STATE_RUNNING
Strategy Server started...
Running Strategy Studio Backtesting Server...
Command line disabled. Type CTRL-C to end.
Started server
 **************************************************************************************************** 
Command executed successfully.
/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/create_instance.sh "RegressionMeanReversion" "RegressionMeanReversion" "XNAS.ITCH-AAPL" "absolute_stop=0.2|strategy_active=true|debug=false|max_inventory=100|window_size=30|previous_prediction=0|inventory_liquidation_increment=10|inventory_liquidation_interval=10|bar_interval=1" "1000000"
Creating instance
rechecking strategies
Login received from connection id 0

Re-checking for additional strategies...
No strategies found to initialize!

Quitting.
Connection id 0 finished.
Connection id 1 finished.
creating new instance
Login received from connection id 1
Strategy RegressionMeanReversion added to processor 0.

Quitting.
Connection id 1 finished.
Connection id 1 finished.
Created instance
active instance list:- 
Login received from connection id 2

--Strategy Instance List--
RegressionMeanReversion
----------------------------


Quitting.
Connection id 2 finished.
Connection id 1 finished.
 **************************************************************************************************** 
Command executed successfully.
/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/edit_parameters.sh "RegressionMeanReversion" "RegressionMeanReversion" "absolute_stop=0.2|strategy_active=true|max_inventory=100|window_size=30|previous_prediction=0|inventory_liquidation_increment=10|inventory_liquidation_interval=10|bar_interval=1"
changing parameters to absolute_stop=0.2|strategy_active=true|max_inventory=100|window_size=30|previous_prediction=0|inventory_liquidation_increment=10|inventory_liquidation_interval=10|bar_interval=1
Login received from connection id 3

Changing params for strategy RegressionMeanReversion...

Quitting.
Connection id 3 finished.
Connection id 1 finished.
strategy reset succesfully
 **************************************************************************************************** 
Command executed successfully.
/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project/provision_scripts/generate_results.sh "/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/group_03_project" "RegressionMeanReversion" "2023-10-29" "2023-10-31" "absolute_stop=0.2|strategy_active=true|debug=false|max_inventory=100|window_size=30|previous_prediction=0|inventory_liquidation_increment=10|inventory_liquidation_interval=10|bar_interval=1" "/groupstorage/group01/RegressionMeanReversion" "True" "1000000" "True" "results.json"
Login received from connection id 4
Starting backtest experiment 1 running 1 strategy with start date 2023-Oct-29 and end date 2023-Oct-31
DataSourceMgr::BuildDataSource: Received request to build data source BacktesterReader for processor id 1
Connection id 4 finished.
MarketState_CLOSE
MarketState_PREOPEN
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 1 completed in 3.49027 millisecsand processed 0 ticks for a throughput of 0 ticks per second
Starting day 2023-Oct-30 of backtest experiment 1
MarketState_CLOSE
MarketState_PREOPEN
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 1 completed in 21.7831 secsand processed 5185378 ticks for a throughput of 238046 ticks per second
Starting day 2023-Oct-31 of backtest experiment 1
MarketState_CLOSE
MarketState_PREOPEN
MarketState_OPEN
MarketState_CLOSE


Text Backtest experiment 1 completed in 20.0328 secsand processed 4701454 ticks for a throughput of 234687 ticks per second
Backtest experiment 1 finished.
cra file name: BACK_RegressionMeanReversion_2024-05-02_014441_start_10-29-2023_end_10-31-2023.cra
Login received from connection id 5

export successful

Quitting.
Connection id 5 finished.
Connection id 1 finished.
output directory: /groupstorage/group01/RegressionMeanReversion/results/
****************************************************************************************************
Arguments are :-  ['get_results.py', 'BACK_RegressionMeanReversion_2024-05-02_014441_start_10-29-2023_end_10-31-2023.cra', '/groupstorage/group01/RegressionMeanReversion', '"absolute_stop=0.2|strategy_active=true|debug=false|max_inventory=100|window_size=30|previous_prediction=0|inventory_liquidation_increment=10|inventory_liquidation_interval=10|bar_interval=1"', '1000000', 'True', 'results.json', '818f980e90786f82bb69c2763012f89513bfb828']
unique_id is :-  014441
new_data is :-  {'014441': {'commit_id': '818f980e90786f82bb69c2763012f89513bfb828', 'total pnl': -172.498672, 'total returns': 999827.501328, 'sharpe': -1.1972112789346343, 'sortino': -3.1232579620713015, 'max_drawdown': -0.00028597125950864953, 'parameters': {'"absolute_stop': '0.2', 'strategy_active': 'true', 'debug': 'false', 'max_inventory': '100', 'window_size': '30', 'previous_prediction': '0', 'inventory_liquidation_increment': '10', 'inventory_liquidation_interval': '10', 'bar_interval': '1"'}, 'dates': ['2023-10-30', '2023-10-31'], 'daily_stats': {'2023-10-30': {'sharpe': -2.151819055842393, 'sortino': -8.008865130552662, 'max_drawdown': -0.00020249867200001608}, '2023-10-31': {'sharpe': -1.745938156009079, 'sortino': -11.672165062862316, 'max_drawdown': -0.00028596368261230823}}}}
Data appended to: /groupstorage/group01/RegressionMeanReversion/results.json
Results have been successfully generated
Command executed successfully.
```

### Expected output on running `python3 parameter_optimisation.py 000002 5`

```plaintext
```
