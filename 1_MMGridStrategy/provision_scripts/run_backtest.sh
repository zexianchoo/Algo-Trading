#!/bin/bash

echo "*** RUNNING BACKTESTING ***"


INSTANCE_NAME="$1"
START_DATE="$2"
END_DATE="$3"
REPO_DIR="$4"
STRATEGY_DIR="$5"

# Start the backtesting server
(cd /home/vagrant/ss/bt && pwd && ls && ./StrategyServerBacktesting&)

echo "Sleeping for 2 seconds while waiting for strategy studio to boot"
sleep 2

# Start the backtest
(cd /home/vagrant/ss/bt/utilities/ && pwd && ls && ./StrategyCommandLine cmd start_backtest "$START_DATE" "$END_DATE" "$INSTANCE_NAME" 0)

foundFinishedLogFile=$(grep -nr "finished.$" /home/vagrant/ss/bt/logs/main_log.txt | gawk '{print $1}' FS=":"|tail -1)

echo "Last line found:",$foundFinishedLogFile

while ((logFileNumLines > foundFinishedLogFile))
do
    foundFinishedLogFile=$(grep -nr "finished.$" /home/vagrant/ss/bt/logs/main_log.txt | gawk '{print $1}' FS=":"|tail -1)
    echo "Waiting for strategy to finish"
    sleep 1
done

echo "Sleeping for 10 seconds..."
sleep 10
echo "run_backtest.sh completed"

cd /home/vagrant/ss/bt/backtesting-results
latestCRA=$(ls /home/vagrant/ss/bt/backtesting-results/BACK_*.cra -t | head -n1)
echo "Latest CRA file path:", $latestCRA
cd /home/vagrant/ss/bt/utilities/
pwd
ls
./StrategyCommandLine cmd export_cra_file $latestCRA /home/vagrant/ss/bt/backtesting-results/csv_files
sleep 5
# ./StrategyCommandLine cmd quit
cd /home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/$REPO_DIR/$STRATEGY_DIR/provision_scripts
# # Path to your Python script
python_script="results_analytics.py"
json_file_path="/home/vagrant/ss/sdk/RCM/StrategyStudio/examples/strategies/$REPO_DIR/$STRATEGY_DIR/backtest_data.json"

# # Call the Python script with the latest CRA file path and JSON file path
python3 "$python_script" "$latestCRA" "$json_file_path"
status=$?

if [ $status -eq 0 ]; then
    echo "Python script updated backtest_data.json."
    # cd ..
    # git config --global user.email "<NetID>@illinois.edu"
    # git config --global user.name "<Username>"
    # git add backtest_data.json
    # git commit -m "Added new backtest result"
    # git push origin main
    # echo "Pushed new backtest result to gitlab repo."
else
    echo "Python script did not update backtest_data.json."
fi

echo "*** FINISHED BACKTESTING ***"

