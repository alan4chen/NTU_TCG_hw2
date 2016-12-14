
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

class BitboardController{
public:

    void init_board(struct bitboard &quickboard){
        quickboard.blackboard = 0;
        quickboard.whiteboard = 0;
        quickboard.turn = true;
        quickboard.pass = 0;
    }

    void generate_board(struct bitboard &quickboard, board &matrixboard){
        quickboard.blackboard = matrixboard.get_black_bitboard();
        //DEBUGG
        // std::cout << "get_black_bitboard() return:" << std::endl;
        // BitboardController().show_bit_string(quickboard.blackboard);
        quickboard.whiteboard = matrixboard.get_white_bitboard();

        // std::cout << "get_white_bitboard() return:" << std::endl;
        // BitboardController().show_bit_string(quickboard.whiteboard);

        quickboard.turn = (matrixboard.get_my_tile()==1)?true:false;
        quickboard.pass = matrixboard.get_pass();
        // std::cout << "quickboard.turn: " << quickboard.turn << std::endl;
        // std::cout << "quickboard.pass: " << quickboard.pass << std::endl;

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
                quickboard.blackboard ^= tile_to_put;
            }
            else{
                to_flip_bit_string = get_toflip_tiles(quickboard.whiteboard, 
                    quickboard.blackboard, tile_to_put);
                // std::cout << "--- --- --- --- to_flip_bit_string: " << std::endl;
                // show_bit_string(to_flip_bit_string);
                quickboard.whiteboard ^= tile_to_put;
            }
            quickboard.blackboard ^= to_flip_bit_string;
            
            quickboard.whiteboard ^= to_flip_bit_string;
            
            quickboard.pass = 0;
        }else{
            quickboard.pass ++;
        }
        // std::cout << "--- --- --- --- --- before turn: " << quickboard.turn << quickboard.turn << true << std::endl;
        quickboard.turn = (quickboard.turn)?false:true; // TODO: find reason cannot use "~"
        // std::cout << "--- --- --- --- --- after turn: " << quickboard.turn <<  ~(quickboard.turn) << false << std::endl;
        return;
    }

    u64 get_filled_board(struct bitboard &quickboard){
        return quickboard.whiteboard | quickboard.blackboard;
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
                    &biter?1:2):0)]);
            }
            fputs("\n------------------\n",fp);
        }
        fflush(fp);
    }

    void show_bit_string(u64 moves){
      for(int i=0; i< 8; i++){
          for(int j=0; j < 8; j++){
            u64 biter = One << (63-(i*8+j));
            if(moves&biter){
                std::cout << 1;
            }
            else{
              std::cout << 0;
            }
          }
          std::cout << std::endl;
        }
        std::cout << std::endl << std::endl; 
}

    u64 random_pick_move(u64 &bitstring){
        u64 mask = bitstring;
        int count = __builtin_popcountll(mask);
        if(count == 0)
            return 0;
        std::uniform_int_distribution<int> dist(0, count-1);
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

        // N
        u64 victims = (own << 8) & enemy;
        for (int i=0; i<5; ++i)
            victims |= (victims << 8) & enemy;
        result |= (victims << 8) & empty;
         
        // S
        victims = (own >> 8) & enemy;
        for (int i=0; i<5; ++i)
            victims |= (victims >> 8) & enemy;
        result |= (victims >> 8) & empty;
         
        // W
        victims = ((own & 0x7f7f7f7f7f7f7f7full) << 1) & enemy;
        for (int i=0; i<5; ++i)
            victims |= ((victims & 0x7f7f7f7f7f7f7f7full) << 1) & enemy;
        result |= ((victims & 0x7f7f7f7f7f7f7f7full) << 1) & empty;
         
        // E
        victims = ((own & 0xfefefefefefefefeull) >> 1) & enemy;
        for (int i=0; i<5; ++i)
            victims |= ((victims & 0xfefefefefefefefeull) >> 1) & enemy;
        result |= ((victims & 0xfefefefefefefefeull) >> 1) & empty;
        
        // NW
        victims = (((own & 0x7f7f7f7f7f7f7f7full) << 1) << 8) & enemy;
        for (int i=0; i<5; ++i)
            victims |= (((victims & 0x7f7f7f7f7f7f7f7full) << 1) << 8) & enemy;
        result |= (((victims & 0x7f7f7f7f7f7f7f7full) << 1) << 8) & empty;
         
        // NE
        victims = (((own & 0xfefefefefefefefeull) >> 1) << 8) & enemy;
        for (int i=0; i<5; ++i)
            victims |= (((victims & 0xfefefefefefefefeull) >> 1) << 8) & enemy;
        result |= (((victims & 0xfefefefefefefefeull) >> 1) << 8) & empty;
         
        // SW
        victims = (((own & 0x7f7f7f7f7f7f7f7full) << 1) >> 8) & enemy;
        for (int i=0; i<5; ++i)
            victims |= (((victims & 0x7f7f7f7f7f7f7f7full) << 1) >> 8) & enemy;
        result |= (((victims & 0x7f7f7f7f7f7f7f7full) << 1) >> 8) & empty;
         
        // SE
        victims = (((own & 0xfefefefefefefefeull) >> 1) >> 8) & enemy;
        for (int i=0; i<5; ++i)
            victims |= (((victims & 0xfefefefefefefefeull) >> 1) >> 8) & enemy;
        result |= (((victims & 0xfefefefefefefefeull) >> 1) >> 8) & empty;
     
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
