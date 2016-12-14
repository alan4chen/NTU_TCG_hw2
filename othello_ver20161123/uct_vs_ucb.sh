killall judge
make judge
echo compile_judge_complete
make search
echo compile_search_complete
mkdir uct_vs_ucb
rm -rf uct_vs_ucb/log_ju.txt uct_vs_ucb/log_p1.txt uct_vs_ucb/log_p2.txt
./judge 27124 60 > uct_vs_ucb/log_ju.txt &
sleep 1
./search_uct 127.0.0.1 27124 > uct_vs_ucb/log_p1.txt &
./search_ucb 127.0.0.1 27124 > uct_vs_ucb/log_p2.txt &
