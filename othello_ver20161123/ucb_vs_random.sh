killall judge
make judge
echo compile_judge_complete
make search
echo compile_search_complete
mkdir ucb_vs_random
rm -rf ucb_vs_random/log_ju.txt ucb_vs_random/log_p1.txt ucb_vs_random/log_p2.txt
./judge 27122 60 > ucb_vs_random/log_ju.txt &
sleep 1
./search_ucb 127.0.0.1 27122 > ucb_vs_random/log_p1.txt &
./search 127.0.0.1 27122 > ucb_vs_random/log_p2.txt &