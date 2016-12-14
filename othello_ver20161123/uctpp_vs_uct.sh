killall judge
make judge
echo compile_judge_complete
make search
echo compile_search_complete
rm -rf ./uctpp_vs_uct
mkdir ./uctpp_vs_uct
./judge 27125 60 > uctpp_vs_uct/log_ju.txt &
sleep 1
./search_progressivepruning 127.0.0.1 27125 > uctpp_vs_uct/log_p1.txt &
./search_uct 127.0.0.1 27125 > uctpp_vs_uct/log_p2.txt &
