#!/bin/bash
POPULATION_SIZE=10000
INITITAL_INFECTED=100
GRID_HEIGHT=200
GRID_WIDTH=200
TIME_STEP=$(expr 60 \* 10)
MAX_SPEED=2
SPREAD_DISTANCE=2
SUSCEPTIBILITY_THR=$(expr 60 \* 60 \* 24 \* 90)
INFECTION_THR=$(expr 60 \* 10)
IMMUNITY_THR=$(expr 60 \* 60 \* 24 \* 10)
COUNTRY_HEIGHT=50
COUNTRY_WIDTH=50
DAY=$(expr 60 \* 60 \* 24)
END_TIME=$(expr $DAY \* 30 \* 4)
DENSITY_THR=0.9

mpirun -np 4 ./obj/main.out $POPULATION_SIZE $INITITAL_INFECTED $GRID_HEIGHT $GRID_WIDTH $TIME_STEP $MAX_SPEED $SPREAD_DISTANCE $SUSCEPTIBILITY_THR $INFECTION_THR $IMMUNITY_THR $COUNTRY_HEIGHT $COUNTRY_WIDTH $DAY $END_TIME $DENSITY_THR
