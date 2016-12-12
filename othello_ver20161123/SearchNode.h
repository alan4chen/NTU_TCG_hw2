#define __searchnode__
#include <iostream>
#include <csignal>
#include <signal.h>
#include <unistd.h>

#ifndef __bitboard__
#include "bitboard.h"
#endif

#ifndef __mytime__
#include "mytime.h"
#endif


/*******PARAMETERS*******/
double c = 1.18;
int SimulationNum = 100;

int LoopNum = 100;
int Timeout = 1;
/************************/

double MIN_DOUBLE = std::numeric_limits<double>::lowest();
double MAX_DOUBLE = std::numeric_limits<double>::max();

BitboardController bitboard_controller = BitboardController();
bool ret_flag = false;

// int DEBUG_SIMULATION = 0;

void alarm_handler(int signal){
    ret_flag = true;
    std::cout << "alram handler!!!" << std::endl;
    // std::cout << "DEBUG SimulationNum: " << DEBUG_SIMULATION << std::endl;
    return;
}

class Node{
public:
    // As a parent node
    double best_UCB = 0;
    int best_UCBchild = 0;
    double best_win = 0;
    int best_winchild = 0;
    Node *nodes[64];
    int ChildNum = 0;

    // As a child node
    double W = 0;
    double N = 0; // Total Simulation in this Node;
    struct bitboard current_board;

    Node(struct bitboard this_bitboard){
        current_board = this_bitboard;
    }

    void make_children(){
        u64 valid_moves = bitboard_controller.get_valid_moves(current_board);
        u64 move;

        //DEBUGG
        // std::cout << "---show valid_moves:" << std::endl;
        // bitboard_controller.show_bit_string(valid_moves);

        for(int i = 0; i < 64; i++){
            move = (valid_moves & (One << i));
            if(move){
                struct bitboard new_board = current_board;
                bitboard_controller.update(new_board, move);
                // DEBUGG
                // std::cout << "--- --- --- after update i: " << i << std::endl;
                // bitboard_controller.show_board(new_board, stderr);

                nodes[ChildNum++] = new Node(new_board);
            }

        }
    }

    void make_simulation(){
        for(int i = 0; i < SimulationNum; i++){
            // std::cout << "--- --- make_simulation i: " << i << std::endl;
            if(simulator()){
                W++;
            }
            N++;

            // if(i > 5){
            //     std::cout << DEBUG_VALID_MOVE << std::endl;
            //     std::cout << DEBUG_RANDOM_PICK << std::endl;
            //     std::cout << DEBUG_UPDATE << std::endl;
            //     exit(1);
            // }
            // DEBUG_SIMULATION ++;

        }
        return;
    }

    // return true if win
    bool simulator(){
        struct bitboard simulation_board = current_board;
        u64 valid_moves;
        u64 tile_to_put;
        // int DEBUGG_COUNT = 0;
        // bitboard_controller.show_board(simulation_board, stderr);
        // std::cout << "-- -- before loop" << std::endl;

        // double DEBUG_CUR = getCurrentTimestamp();
        // double DEBUG_NEW_CUR = 0;

        while(!bitboard_controller.is_game_over(simulation_board)){
            // std::cout << "-- -- -- -- before get_valid_moves" << std::endl;
            
            valid_moves = bitboard_controller.get_valid_moves(simulation_board);
            // DEBUG_NEW_CUR = getCurrentTimestamp();
            // DEBUG_VALID_MOVE += DEBUG_NEW_CUR-DEBUG_CUR;
            // DEBUG_CUR = DEBUG_NEW_CUR;

            // bitboard_controller.show_board(simulation_board, stderr);
            // bitboard_controller.show_bit_string(valid_moves);

            // std::cout << "-- -- -- -- before random_pick_move" << std::endl;

            tile_to_put = bitboard_controller.random_pick_move(valid_moves);
            // DEBUG_NEW_CUR = getCurrentTimestamp();
            // DEBUG_RANDOM_PICK += DEBUG_NEW_CUR-DEBUG_CUR;
            // DEBUG_CUR = DEBUG_NEW_CUR;
            // std::cout << "-- -- -- -- before update" << std::endl;
            bitboard_controller.update(simulation_board, tile_to_put);
            // DEBUG_NEW_CUR = getCurrentTimestamp();
            // DEBUG_UPDATE += DEBUG_NEW_CUR-DEBUG_CUR;
            // DEBUG_CUR = DEBUG_NEW_CUR;
            // std::cout << "-- -- -- -- after update" << std::endl;

            // DEBUGG_COUNT ++;
            // // bitboard_controller.show_board(simulation_board, stderr);
            // if(DEBUGG_COUNT > 100){
            //     exit(1);
            // }

            // std::cout << "--- --- --- --- turn: " << simulation_board.turn << std::endl;

        }
        // std::cout << "-- -- after loop" << std::endl;
        // std::cout << "--- --- --- DEBUGG_COUNT depth: " << DEBUGG_COUNT << std::endl;
        // bitboard_controller.show_board(simulation_board, stderr);
        // exit(1);
        int score = bitboard_controller.get_score(simulation_board);
        if(((current_board.turn==true) && score > 0) || 
            ((current_board.turn==false) && score < 0)){
            return true;
        }
        return false;
    }

    // return move in u64
    u64 UCB(){
        ret_flag = false;
        signal(SIGALRM, alarm_handler);
        alarm(Timeout);

        /* Step 1: Make Children */
        make_children();
        // No child means no valid moves
        if(ChildNum == 0){
            return 0;
        }
        std::cout << "---ChildNum is: " << ChildNum << std::endl;

        /* Step 2: Init Simulation */
        // Simulation Among the child
        for(int i=0; i<ChildNum; i++){
            // std::cout << "---simulate i: " << i << std::endl;
            nodes[i]->make_simulation();
        }
        /* Step 3: Simulation according best UCB child */
        int win_num;
        double ucb_score;
        double win_score;
        while(ret_flag!=true){
            // Calculate children's UCB Score
            // Add N
            N = 0;
            for(int i=0; i<ChildNum; i++){
                N += nodes[i]->N;
            }
            // Find best ucb score in children
            for(int i=0; i<ChildNum; i++){
                win_num = nodes[i]->N - nodes[i]->W;
                win_score = win_num / nodes[i]->N;
                ucb_score = win_score + c * sqrt(log(N)/nodes[i]->N);
                if(ucb_score > best_UCB){
                    best_UCB = ucb_score;
                    best_UCBchild = i;
                }
                
            }
            // std::cout << "best child is: " << best_UCBchild << std::endl;
            nodes[best_UCBchild]->make_simulation();
        }

        /* Step 4: find the child with best win rate */
        for(int i=0; i<ChildNum; i++){
            win_num = nodes[i]->N - nodes[i]->W;
            win_score = win_num / nodes[i]->N;
            if(win_score > best_win){
                best_win = win_score;
                best_winchild = i;
            }
        }

        // return best move with largest win_rate
        for(int i=0; i<ChildNum; i++){
            std::cout << "child " << i << " has win score: " << nodes[i]->W/nodes[i]->N << std::endl;
        }
        std::cout << "choose child: " << best_winchild << std::endl;
        std::cout << "return board:" << std::endl;
        bitboard_controller.show_bit_string(bitboard_controller.get_filled_board(nodes[best_winchild]->current_board) ^\
            bitboard_controller.get_filled_board(current_board));
        std::cout << "return blackboard: " <<nodes[best_winchild]->current_board.blackboard << std::endl;
        std::cout << "return whiteboard: " <<nodes[best_winchild]->current_board.whiteboard << std::endl;
        return bitboard_controller.get_filled_board(nodes[best_winchild]->current_board) ^\
            bitboard_controller.get_filled_board(current_board);

    }

    ~Node(){
        // cout << "delete:" << a << endl;
        for(int i=0; i<ChildNum; i++){
            delete nodes[ChildNum];
        }
    }
};