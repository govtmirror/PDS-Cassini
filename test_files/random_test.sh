rm xaa.tab
rm xab.tab
sort --random-sort full_index.tab > full_random_index.tab
split -l 432 full_random_index.tab
mv xaa xaa.tab
mv xab xab.tab
./PropellerTrain -t xaa.lbl -c 8
./PropellerTest -t xab.lbl -c 8
./PropellerStats
