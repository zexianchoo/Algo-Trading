#!/bin/bash

instanceName="$1"
strategyName="$2"
symbols="$3"
params="$4"
accountsize="$5"

echo "Creating instance" 
cd /home/vagrant/ss/bt/utilities
echo "rechecking strategies"
./StrategyCommandLine cmd recheck_strategies
echo "creating new instance"
./StrategyCommandLine cmd create_instance "$instanceName" "$strategyName" UIUC SIM-1001-101 dlariviere $accountsize -symbols $symbols -params $params
echo "Created instance" 
echo "active instance list:- "
./StrategyCommandLine cmd strategy_instance_list
echo " **************************************************************************************************** "