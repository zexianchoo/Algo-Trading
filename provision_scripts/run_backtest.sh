#!/bin/bash

instanceName="$1"
startDate="$2"
endDate="$3"
strategy_wait_time="$4"

echo $instanceName &&
echo $startDate &&
echo $endDate &&

cd /home/vagrant/ss/bt/utilities/ && ./StrategyCommandLine cmd start_backtest "$startDate" "$endDate" "$instanceName" 0 

# Path to your log file
log_file="/home/vagrant/ss/bt/logs/main_log.txt"

# Convert dates to seconds since the epoch
start_sec=$(date -d "$startDate" +%s)
end_sec=$(date -d "$endDate" +%s)

# Initialize count of working days
workdays=0

# Ensure start_sec and end_sec are initialized properly
if [ -z "$start_sec" ] || [ -z "$end_sec" ]; then
    echo "Start or end timestamps not set."
    exit 1
fi

# Check if start and end date are the same
if [ "$start_sec" -eq "$end_sec" ]; then
    day_of_week=$(date -d @$start_sec +%u)
    # Consider it as one working day if it's a weekday
    if [[ $day_of_week -lt 6 ]]; then
        workdays=1
    else
        workdays=0
    fi
else
    workdays=0
    # Loop through all days
    for (( d = $start_sec; d <= $end_sec; d += 86400 )); do
        day_of_week=$(date -d @$d +%u)
        # Increment count if it's a weekday (Monday to Friday)
        if [[ $day_of_week -lt 6 ]]; then
            ((workdays++))
        fi
    done
fi

echo "Total workdays: $workdays"

# Initialize line_number to 0 in case the grep command doesn't find a match
line_number=$(grep -n "finished\.$" "$log_file" | tail -n 1 | cut -d: -f1 || echo "0")
echo "running backtest for $workdays working days"
# Multiply the number of working days by 15 adjust based on backtesting time
sleep_time=$((workdays * strategy_wait_time))
echo "Sleeping for $sleep_time seconds"
# Sleep for the calculated time
sleep "$sleep_time"
# Initialize a counter variable
counter=0
# Maximum number of checks
max_checks=2

while true; do
    # Increment the counter
    ((counter++))

    # Get the number of lines in the log file
    num_lines=$(wc -l < "$log_file")

    # Initialize line_number to 0 in case the grep command doesn't find a match
    line_number=$(grep -n "finished\.$" "$log_file" | tail -n 1 | cut -d: -f1 || echo "0")

    # Check if the counter has reached the maximum number of checks
    if [ "$counter" -le "$max_checks" ]; then
        # Check if the last line ending with "finished." is the last line in the log file
        if [ "$num_lines" -gt "$line_number" ]; then
            echo "Waiting for .CRA file to be generated."
            sleep 5
        else
            last_cra_file=$(grep '\.cra' "$log_file" | tail -n1 | awk '{print $NF}')
            last_cra_file=$(basename "$last_cra_file")
            echo "found CRA file :- "
            echo "$last_cra_file"
            echo "Strategy finished"
            break
        fi
    else
        echo "Maximum checks reached without completing strategy."
        break
    fi
done

echo "run backtest complete"
echo " **************************************************************************************************** "