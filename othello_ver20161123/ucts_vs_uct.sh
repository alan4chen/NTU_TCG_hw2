
make judge
echo compile_judge_complete
make search
echo compile_search_complete
rm -rf ./ucts_vs_uct
mkdir ucts_vs_uct
./judge 27126 4 > ucts_vs_uct/log_ju.txt &
sleep 1
./search_ucts 127.0.0.1 27126 > ucts_vs_uct/log_p1.txt &
./search_uct 127.0.0.1 27126 > ucts_vs_uct/log_p2.txt &
