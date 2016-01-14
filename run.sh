#!/bin/bash

gnome-terminal -e "./bin/main -p 9003 -f host.txt &" 
gnome-terminal -e "./bin/main -p 9000 -f host.txt &" 
gnome-terminal -e "./bin/main -p 9001 -f host.txt &" 
gnome-terminal -e "./bin/main -p 9002 -f host.txt &" 

