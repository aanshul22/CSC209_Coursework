#!/usr/bin/env bas
./classifier -p 48 -K 1 -d euc a2_datasets/training_data.bin a2_datasets/testing_data.bin & >> results.txt
./classifier -p 48 -K 2 -d euc a2_datasets/training_data.bin a2_datasets/testing_data.bin & >> results.txt
./classifier -p 48 -K 3 -d euc a2_datasets/training_data.bin a2_datasets/testing_data.bin & >> results.txt
./classifier -p 48 -K 4 -d euc a2_datasets/training_data.bin a2_datasets/testing_data.bin & >> results.txt
./classifier -p 48 -K 1 -d cos a2_datasets/training_data.bin a2_datasets/testing_data.bin & >> results.txt
./classifier -p 48 -K 2 -d cos a2_datasets/training_data.bin a2_datasets/testing_data.bin & >> results.txt
./classifier -p 48 -K 3 -d cos a2_datasets/training_data.bin a2_datasets/testing_data.bin & >> results.txt
./classifier -p 48 -K 4 -d cos a2_datasets/training_data.bin a2_datasets/testing_data.bin & >> results.txt
