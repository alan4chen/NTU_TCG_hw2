#ifndef __bitboard__
#define __bitboard__
#include<cctype>
#include<cstdio>
#include<algorithm>
#include<iostream>
#include<random>

#ifndef __board__
#include"board.h"
#endif

#ifndef ONE
typedef unsigned long long u64;
u64 One = 1;
#define ONE
#endif

std::random_device rd;
std::mt19937 mt(rd());

struct bitboard{
    u64 blackboard; // represent 1 X
    u64 whiteboard; // represent 2 O
    bool turn; // represent True X Black, False O White
    short pass;
};

class bitboard_controller{
public:
    

    void init_board(struct bitboard &quickboard){
        quickboard.blackboard = 0;
        quickboard.whiteboard = 0;
        quickboard.turn = true;
        quickboard.pass = 0;
    }

    void generate_board(struct bitboard &quickboard, board &matrixboard){
        quickboard.blackboard = matrixboard.get_black_bitboard();
        quickboard.whiteboard = matrixboard.get_white_bitboard();
        quickboard.turn = (matrixboard.get_my_tile()==1)?true:false;
        quickboard.pass = matrixboard.get_pass();
    }

    u64 get_valid_moves(struct bitboard &quickboard){
        u64 valid_move_bit_string;
        if (quickboard.turn == true){
            valid_move_bit_string = generate_moves(quickboard.blackboard, 
                quickboard.whiteboard);
        }
        else{
            valid_move_bit_string = generate_moves(quickboard.whiteboard, 
                quickboard.blackboard);
        }
        return valid_move_bit_string;
    }
    void update(struct bitboard &quickboard, u64 &tile_to_put){
        if(tile_to_put != 0){
            u64 to_flip_bit_string;
            if (quickboard.turn == true){
                to_flip_bit_string = get_toflip_tiles(quickboard.blackboard, 
                    quickboard.whiteboard, tile_to_put);
                quickboard.blackboard |= to_flip_bit_string;
                quickboard.blackboard |= tile_to_put;
            }
            else{
                to_flip_bit_string = get_toflip_tiles(quickboard.whiteboard, 
                    quickboard.blackboard, tile_to_put);
                quickboard.whiteboard |= to_flip_bit_string;
                quickboard.whiteboard |= tile_to_put;
            }
            quickboard.pass = 0;
        }else{
            quickboard.pass ++;
        }
        quickboard.turn = ~quickboard.turn;
        return;
    }

    void show_board(struct bitboard &quickboard, FILE*fp)const{
        static constexpr char c[]{'.','X','O'};
        fprintf(fp,"%c's turn\n",c[quickboard.turn==true?1:2]);
        {
            fprintf(fp,".|");
            for(int j=0;j!=8;++j){
                fprintf(fp,"%c|",'0'+j);
            }
            fputs("\n------------------\n",fp);
        }
        u64 biter = 0;
        for(int i=0;i!=8;++i){
            fprintf(fp,"%c|",'0'+i);
            for(int j=0;j!=8;++j){
                biter = One<<(63-(i<<3^j));
                fprintf(fp,"%c|",c[((quickboard.blackboard\
                    |quickboard.whiteboard)&biter?(quickboard.blackboard\
                    &quickboard.whiteboard?1:2):0)]);
            }
            fputs("\n------------------\n",fp);
        }
        fflush(fp);
    }

    u64 random_pick_bit(u64 &bitstring){
        int mask = bitstring;
        std::uniform_int_distribution<int> dist(0, __builtin_popcountll(mask)-1);
        int index = dist(mt);
        for (int i = 0; i < index; i++)
            mask &= mask - 1;
        mask ^= mask & (mask - 1);
        return mask;
    }

    std::pair<int,int> get_count(struct bitboard &quickboard)const{
        return std::pair<int,int>(__builtin_popcountll(quickboard.blackboard),
            __builtin_popcountll(quickboard.whiteboard));
    }
    int get_score(struct bitboard &quickboard)const{
        std::pair<int,int> count_pair = get_count(quickboard);
        return count_pair.first-count_pair.second;
    }
    bool is_game_over(struct bitboard &quickboard)const{
        return quickboard.pass==2;
    }

    /******** IMPLEMENTATION ***********/
    // Implement Gen move bit string
    u64 N(u64 x) {
        return x << 8;
    }
    u64 S(u64 x) {
        return x >> 8;
    }
    u64 E(u64 x) {
        return (x & 0xfefefefefefefefeull) >> 1;
    }
    u64 W(u64 x) {
        return (x & 0x7f7f7f7f7f7f7f7full) << 1;
    }
    u64 NW(u64 x) {
        return N(W(x));
    } 
    u64 NE(u64 x) {
        return N(E(x));
    }  
    u64 SW(u64 x) {
        return S(W(x));
    }
    u64 SE(u64 x) {
        return S(E(x));
    }    
    u64 generate_moves(u64 &own, u64 &enemy) {
        u64 result = 0ull;
        
        u64 empty = ~(own | enemy);
        u64 victims = N(own) & enemy;
        for (int i=0; i<5; ++i)
            victims |= N(victims) & enemy;
        result |= N(victims) & empty;
         
        victims = S(own) & enemy;
        for (int i=0; i<5; ++i)
            victims |= S(victims) & enemy;
        result |= S(victims) & empty;
         
        victims = W(own) & enemy;
        for (int i=0; i<5; ++i)
            victims |= W(victims) & enemy;
        result |= W(victims) & empty;
         
        victims = E(own) & enemy;
        for (int i=0; i<5; ++i)
            victims |= E(victims) & enemy;
        result |= E(victims) & empty;
         
        victims = NW(own) & enemy;
        for (int i=0; i<5; ++i)
            victims |= NW(victims) & enemy;
        result |= NW(victims) & empty;
         
        victims = NE(own) & enemy;
        for (int i=0; i<5; ++i)
            victims |= NE(victims) & enemy;
        result |= NE(victims) & empty;
         
        victims = SW(own) & enemy;
        for (int i=0; i<5; ++i)
            victims |= SW(victims) & enemy;
        result |= SW(victims) & empty;
         
        victims = SE(own) & enemy;
        for (int i=0; i<5; ++i)
            victims |= SE(victims) & enemy;
        result |= SE(victims) & empty;
     
    return result;
    }

    u64 get_toflip_tiles(u64 &own, u64 &enemy, u64 &new_tile) {
        u64 result = 0ull;
        u64 victims = N(new_tile) & enemy;
        for (int i=0; i<6; ++i)
            victims |= N(victims) & enemy;
        u64 slaves = S(own) & enemy;
        for (int i=0; i<6; ++i)
            slaves |= S(slaves) & enemy;
        result |= slaves & victims;

        victims = S(new_tile) & enemy;
        for (int i=0; i<6; ++i)
            victims |= S(victims) & enemy;
        slaves = N(own) & enemy;
        for (int i=0; i<6; ++i)
            slaves |= N(slaves) & enemy;
        result |= slaves & victims;

        victims = W(new_tile) & enemy;
        for (int i=0; i<6; ++i)
            victims |= W(victims) & enemy;
        slaves = E(own) & enemy;
        for (int i=0; i<6; ++i)
            slaves |= E(slaves) & enemy;
        result |= slaves & victims;

        victims = E(new_tile) & enemy;
        for (int i=0; i<6; ++i)
            victims |= E(victims) & enemy;
        slaves = W(own) & enemy;
        for (int i=0; i<6; ++i)
            slaves |= W(slaves) & enemy;
        result |= slaves & victims;

        victims = NW(new_tile) & enemy;
        for (int i=0; i<6; ++i)
            victims |= NW(victims) & enemy;
        slaves = SE(own) & enemy;
        for (int i=0; i<6; ++i)
            slaves |= SE(slaves) & enemy;
        result |= slaves & victims;

        victims = SE(new_tile) & enemy;
        for (int i=0; i<6; ++i)
            victims |= SE(victims) & enemy;
        slaves = NW(own) & enemy;
        for (int i=0; i<6; ++i)
            slaves |= NW(slaves) & enemy;
        result |= slaves & victims;

        victims = NE(new_tile) & enemy;
        for (int i=0; i<6; ++i)
            victims |= NE(victims) & enemy;
        slaves = SW(own) & enemy;
        for (int i=0; i<6; ++i)
            slaves |= SW(slaves) & enemy;
        result |= slaves & victims;

        victims = SW(new_tile) & enemy;
        for (int i=0; i<6; ++i)
            victims |= SW(victims) & enemy;
        slaves = NE(own) & enemy;
        for (int i=0; i<6; ++i)
            slaves |= NE(slaves) & enemy;
        result |= slaves & victims;
     
    return result;
    }
};
#endif
