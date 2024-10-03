
echo "Starting server" 
# quits previously created server
cd /home/vagrant/ss/bt/utilities && ./StrategyCommandLine cmd quit 
sleep 5 # sleep for 1 second as server takes time to quit
# starts up the server and keeps it in the background
cd /home/vagrant/ss/bt/ && ./StrategyServerBacktesting &
sleep 10 # sleep for 10 seconds as server takes time to start
echo "Started server" 
echo " **************************************************************************************************** "

