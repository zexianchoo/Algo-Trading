#!/bin/bash

echo "*** BUILDING STRATEGY ***"

INSTANCE_NAME="$1"
STRATEGY_NAME="$2"
SYMBOLS="$3"

echo "Quitting server. This is to just to have a clean start."
echo "Ignore any errors that may occur here because of quitting the server."
cd /home/vagrant/ss/bt/utilities; ./StrategyCommandLine cmd quit
sleep 10
echo "Starting server. There should be no errors after this point."
cd /home/vagrant/ss/bt/ ; ./StrategyServerBacktesting &
sleep 1
echo "Started server"
cd /home/vagrant/ss/bt/utilities
echo "calling"
echo ./StrategyCommandLine cmd create_instance "$INSTANCE_NAME" "$STRATEGY_NAME" UIUC SIM-1001-101 dlariviere 1000000 -symbols "$SYMBOLS" 
./StrategyCommandLine cmd create_instance "$INSTANCE_NAME" "$STRATEGY_NAME" UIUC SIM-1001-101 dlariviere 1000000 -symbols "$SYMBOLS"
echo "Created instance"
sleep 1

echo "*** FINISHED STRATEGY BUILD ***"