workingDir="$1"
instanceName="$2"
startDate="$3"
endDate="$4"
parameters="$5"
outputDirectory="$6"
fullReports="$7"
accountSize="$8"
dailyStats="$9"
name="${10}"
waitTime="$11"

CRA_path="/home/vagrant/ss/bt/backtesting-results/"
# Check if output directory exists; if not, create it
if [ ! -d "$outputDirectory" ]; then
    mkdir -p "$outputDirectory"
fi
# runs the backtest and gets the last cra file name
last_cra_file_name=$("$workingDir"/provision_scripts/run_backtest.sh "$instanceName" "$startDate" "$endDate" "$waitTime" | grep '\.cra' | tail -n1 | awk '{print $NF}')

# check if the last cra file is equal to ""
if [ -z "$last_cra_file_name" ]; then
    echo "The last .cra file mentioned is empty"
    exit 1
fi

mv "$CRA_path$last_cra_file_name" "$outputDirectory/results/"

echo "cra file name: $last_cra_file_name"
cd /home/vagrant/ss/bt/utilities/ && ./StrategyCommandLine cmd export_cra_file "$outputDirectory/results/$last_cra_file_name" 
echo "output directory: $outputDirectory/results/"
echo "****************************************************************************************************"

cd "$workingDir"/provision_scripts/
# Generate the results
latest_commit_id=$(git rev-parse HEAD)

python3 get_results.py "$last_cra_file_name" "$outputDirectory" "\"$parameters\"" "$accountSize" "$dailyStats" "$name" "$latest_commit_id"
# check if above program has run successfully
if [ $? -ne 0 ]; then
    echo "Error in generating results"
    exit 1
fi