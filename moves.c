/*
    Sjeng - a chess variants playing program
    Copyright (C) 2000 Gian-Carlo Pascutto

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    File: moves.c                                        
    Purpose: functions used to generate & make moves

*/

/*
 * 14x14 - 196 cells
 *    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 000-013
 *    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 014-027
 *    0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, // 028-041
 *    0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, // 042-055
 *    0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, // 056-069
 *    0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, // 070-083
 *    0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, // 084-097
 *    0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, // 098-111
 *    0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, // 112-125
 *    0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, // 126-139
 *    0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, // 140-153
 *    0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, // 154-167
 *    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 168-181
 *    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // 182-195
 */

#include "sjeng.h"
#include "extvars.h"
#include "protos.h"
#include "squares.h"

unsigned long total_moves;
unsigned long total_movegens;

int numb_moves;
static move_s *genfor;

int fcaptures;
int gfrom;

int kingcap; /* break if we capture the king */

bool check_legal (move_s moves[], int m, int incheck) { //x
    /* determines if a move made was legal.  Checks to see if the player who
       just moved castled through check, or is in check.  If the move made
       was illegal, returns FALSE, otherwise, returns TRUE. */

    int castled = moves[m].castled;
    int from = moves[m].from;
    int target = moves[m].target;
    int l;

    if (Variant == Suicide) return TRUE;

    /* check for castling moves: */
    if (castled) {
        /* white kingside castling: */
        if (castled == wck) {
            if (is_attacked (F1, 0)) return FALSE; //x
            if (is_attacked (G1, 0)) return FALSE; //x
            if (is_attacked (H1, 0)) return FALSE; //x
            return TRUE;
        }
        /* white queenside castling: */
        if (castled == wcq) {
            if (is_attacked (F1, 0)) return FALSE; //x
            if (is_attacked (E1, 0)) return FALSE; //x
            if (is_attacked (D1, 0)) return FALSE; //x
            return TRUE;
        }
        /* black kingside castling: */
        if (castled == bck) {
            if (is_attacked (F10, 1)) return FALSE; //x
            if (is_attacked (G10, 1)) return FALSE; //x
            if (is_attacked (H10, 1)) return FALSE; //x
            return TRUE;
        }
        /* black queenside castling: */
        if (castled == bcq) {
            if (is_attacked (F10, 1)) return FALSE; //x
            if (is_attacked (E10, 1)) return FALSE; //x
            if (is_attacked (D10, 1)) return FALSE; //x
            return TRUE;
        }
    }

    /* otherwise, just check on the kings: */
    /* black king: */

    /* the code in here checks whether a move could
     * have put the king in check, if he was not in
     * check before, if not, an early exit is taken */

    else if (white_to_move&1) {
        if (!incheck) {
            if (moves[m].from == 0) return TRUE;

            switch (moves[m].promoted ? bpawn : board[target]) {
                case bpawn:
                    /* pawn moves, it can discover a rank or diagonal check 
                     * a capture can also discover a file check */
                    if (moves[m].captured != npiece) {
                        if (file(from) != file(bking_loc) 
                                && rank(from) != rank(bking_loc)
                                && diagl(from) != diagl(bking_loc)
                                && diagr(from) != diagr(bking_loc))
                            return TRUE;
                    } else {
                        if (rank(from) != rank(bking_loc)
                                && diagl(from) != diagl(bking_loc)
                                && diagr(from) != diagr(bking_loc))
                            return TRUE;
                    }
                    break;
                case bknight:
                    /* discovers all */
                    if (file(from) != file(bking_loc) 
                            && rank(from) != rank(bking_loc)
                            && diagl(from) != diagl(bking_loc)
                            && diagr(from) != diagr(bking_loc))
                        return TRUE;
                    break;
                case bbishop:
                    /* always discovers file and rank
                     * always discovers one diagonal */
                    if (file(from) != file(bking_loc) 
                            && rank(from) != rank(bking_loc))
                    {
                        if (diagl(from) == diagl(target)) {
                            /* stays on diag, can only uncover check on
                             * other diag */
                            if (diagr(from) != diagr(bking_loc))
                                return TRUE;
                        } else {
                            if (diagl(from) != diagl(bking_loc))
                                return TRUE;
                        }
                    }
                    break;
                case brook:
                    /* discovers diagonal always */
                    /* one file or rank discovered */
                    if (diagr(from) != diagr(bking_loc)
                            && diagl(from) != diagl(bking_loc))
                    {
                        /* rank move ? */
                        if(rank(from) == rank(target)) {
                            if (file(from) != file(bking_loc))
                                return TRUE;
                        } else {
                            /* file move */
                            if (rank(from) != rank(bking_loc))
                                return TRUE;
                        }
                    }
                    break;
                case bqueen:
                    /* find out what move it was: ldiag/rdiag/file/rank*/
                    if (file(from) == file(target)) {
                        if (diagr(from) != diagr(bking_loc)
                                && diagl(from) != diagl(bking_loc)
                                && rank(from) != rank(bking_loc))
                            return TRUE;	
                    } else if (rank(from) == rank(target)) {
                        if (diagr(from) != diagr(bking_loc)
                                && file(from) != file(bking_loc)
                                && diagl(from) != diagl(bking_loc))
                            return TRUE;	
                    } else if (diagl(from) == diagl(target)) {
                        if (diagr(from) != diagr(bking_loc)
                                && file(from) != file(bking_loc)
                                && rank(from) != rank(bking_loc))
                            return TRUE;	
                    } else if (diagr(from) == diagr(target)) {
                        if (diagl(from) != diagl(bking_loc)
                                && file(from) != file(bking_loc)
                                && rank(from) != rank(bking_loc))
                            return TRUE;	
                    }
                    break;
                default:
                    break;
            }

            /* we got so far, we know there can only be some
             * kind of possible discovering */
            /* find out what */
            /* we do not need to check for pawn, king or knightattacks,
             * as they cannot be discovered*/

            if (board[target] != bking) {
                if (file(from) == file(bking_loc)) {
                    if (bking_loc > from) {
                        for (l = bking_loc-14; board[l] == npiece; l-=14); //x
                        if (board[l] == wrook || board[l] == wqueen) return FALSE;
                    } else {
                        for (l = bking_loc+14; board[l] == npiece; l+=14); //x
                        if (board[l] == wrook || board[l] == wqueen) return FALSE;
                    }
                } else if (rank(from) == rank(bking_loc)) {
                    if (bking_loc > from) {
                        for (l = bking_loc-1; board[l] == npiece; l-=1);
                        if (board[l] == wrook || board[l] == wqueen) return FALSE;
                    } else {
                        for (l = bking_loc+1; board[l] == npiece; l+=1);
                        if (board[l] == wrook || board[l] == wqueen) return FALSE;
                    }
                } else if (diagl(from) == diagl(bking_loc)) {
                    if (bking_loc > from) {
                        for (l = bking_loc-15; board[l] == npiece; l-=15); //x
                        if (board[l] == wbishop || board[l] == wqueen) return FALSE;
                    } else {
                        for (l = bking_loc+15; board[l] == npiece; l+=15); //x
                        if (board[l] == wbishop || board[l] == wqueen) return FALSE;
                    }
                } else if (diagr(from) == diagr(bking_loc)) {
                    if (bking_loc > from) {
                        for (l = bking_loc-13; board[l] == npiece; l-=13); //x
                        if (board[l] == wbishop || board[l] == wqueen) return FALSE;
                    } else {
                        for (l = bking_loc+13; board[l] == npiece; l+=13); //x
                        if (board[l] == wbishop || board[l] == wqueen) return FALSE;
                    }
                }
                return TRUE;
            }
        }

        if (is_attacked (bking_loc, 1)) {
            return FALSE;
        } else return TRUE;
    } else {
        /* white king: */

        if (!incheck) {
            if (moves[m].from == 0) return TRUE;

            switch (moves[m].promoted ? wpawn : board[target]) {
                case wpawn:
                    /* pawn moves, it can discover a rank or diagonal check 
                     * a capture can also discover a file check */
                    if (moves[m].captured != npiece)
                    {
                        if (file(from) != file(wking_loc) 
                                && rank(from) != rank(wking_loc)
                                && diagl(from) != diagl(wking_loc)
                                && diagr(from) != diagr(wking_loc))
                            return TRUE;
                    } else {
                        if (rank(from) != rank(wking_loc)
                                && diagl(from) != diagl(wking_loc)
                                && diagr(from) != diagr(wking_loc))
                            return TRUE;
                    }
                    break;
                case wknight:
                    /* discovers all */
                    if (file(from) != file(wking_loc) 
                            && rank(from) != rank(wking_loc)
                            && diagl(from) != diagl(wking_loc)
                            && diagr(from) != diagr(wking_loc))
                        return TRUE;
                    break;
                case wbishop:
                    /* always discovers file and rank
                     * always discovers one diagonal */
                    if (file(from) != file(wking_loc) 
                            && rank(from) != rank(wking_loc))
                    {
                        if (diagl(from) == diagl(target)) {
                            /* stays on diag, can only uncover check on
                             * other diag */
                            if (diagr(from) != diagr(wking_loc))
                                return TRUE;
                        } else {
                            if (diagl(from) != diagl(wking_loc))
                                return TRUE;
                        }
                    }
                    break;
                case wrook:
                    /* discovers diagonal always */
                    /* one file or rank discovered */
                    if (diagr(from) != diagr(wking_loc)
                            && diagl(from) != diagl(wking_loc))
                    {
                        /* rank move ? */
                        if(rank(from) == rank(target)) {
                            if (file(from) != file(wking_loc))
                                return TRUE;
                        } else {
                            /* file move */
                            if (rank(from) != rank(wking_loc))
                                return TRUE;
                        }
                    }
                    break;
                case wqueen:
                    /* find out what move it was: ldiag/rdiag/file/rank*/
                    if (file(from) == file(moves[m].target)) {
                        if (diagr(from) != diagr(wking_loc)
                                && diagl(from) != diagl(wking_loc)
                                && rank(from) != rank(wking_loc))
                            return TRUE;	
                    } else if (rank(from) == rank(target)) {
                        if (diagr(from) != diagr(wking_loc)
                                && file(from) != file(wking_loc)
                                && diagl(from) != diagl(wking_loc))
                            return TRUE;	
                    } else if (diagl(from) == diagl(target)) {
                        if (diagr(from) != diagr(wking_loc)
                                && file(from) != file(wking_loc)
                                && rank(from) != rank(wking_loc))
                            return TRUE;	
                    } else if (diagr(from) == diagr(target)) {
                        if (diagl(from) != diagl(wking_loc)
                                && file(from) != file(wking_loc)
                                && rank(from) != rank(wking_loc))
                            return TRUE;	
                    }
                    break;
                default:
                    break;
            }

            if (board[target] != wking) {
                if (file(from) == file(wking_loc)) {
                    if (wking_loc > from) {
                        for (l = wking_loc-14; board[l] == npiece; l-=14); //x
                        if (board[l] == brook || board[l] == bqueen) return FALSE;
                    } else {
                        for (l = wking_loc+14; board[l] == npiece; l+=14); //x
                        if (board[l] == brook || board[l] == bqueen) return FALSE;
                    }
                } else if (rank(from) == rank(wking_loc)) {
                    if (wking_loc > from) {
                        for (l = wking_loc-1; board[l] == npiece; l-=1); //x
                        if (board[l] == brook || board[l] == bqueen) return FALSE;
                    } else {
                        for (l = wking_loc+1; board[l] == npiece; l+=1); //x
                        if (board[l] == brook || board[l] == bqueen) return FALSE;
                    }
                } else if (diagl(from) == diagl(wking_loc)) {
                    if (wking_loc > from) {
                        for (l = wking_loc-15; board[l] == npiece; l-=15); //x
                        if (board[l] == bbishop || board[l] == bqueen) return FALSE;
                    } else {
                        for (l = wking_loc+15; board[l] == npiece; l+=15); //x
                        if (board[l] == bbishop || board[l] == bqueen) return FALSE;
                    }
                } else if (diagr(from) == diagr(wking_loc)) {
                    if (wking_loc > from) {
                        for (l = wking_loc-13; board[l] == npiece; l-=13); //x
                        if (board[l] == bbishop || board[l] == bqueen) return FALSE;
                    } else {
                        for (l = wking_loc+13; board[l] == npiece; l+=13); //x
                        if (board[l] == bbishop || board[l] == bqueen) return FALSE;
                    }
                }
                return TRUE;
            }
        }

        if (is_attacked (wking_loc, 0)) {
            return FALSE;
        } else return TRUE;
    }

    /* should never get here .. but just so it will compile :P */
    return FALSE;
}


#define push_slide(t) if (board[(t)] != frame) push_slidE((t))
#define push_knight(t) if (board[(t)] != frame) push_knighT((t))

void gen (move_s moves[]) {
    /* generate pseudo-legal moves, and place them in the moves array */

    int from, a, j, i;

    kingcap = FALSE;

    numb_moves = 0;
    genfor = &moves[0];

    if (Variant == Suicide) {
        captures = FALSE;
        fcaptures = FALSE;
    };

restart:

    /* generate white moves, if it is white to move: */
    if (white_to_move) {
        for (a = 1, j = 1;
                (a <= piece_count) && 
                 (((Variant != Suicide) && !kingcap) || ((Variant == Suicide) && (fcaptures == captures)))
                ; j++) {

            i = pieces[j];

            if (i < 1)
                continue;
            else
                a++;

            from = i;
            gfrom = i;

            switch (board[from]) {
                case (wpawn):
                    /* pawn moves up one square: */
                    if (board[from+14] == npiece) { //x
                        /* only promotions when captures == TRUE */
                        if (rank (from) == 9 && ((Variant != Suicide) && (Variant != Losers))) { //x
                            push_pawn (from+14, FALSE); //x
                        } else if (!captures) {
                            push_pawn (from+14, FALSE); //x

                            /* pawn moving up two squares on its first move: */
                            if (rank(from) == 2 && board[from+28] == npiece) //x
                                push_pawn_simple (from+28); //x
                        }
                    }
                    /* pawn capturing diagonally: */
                    if ((board[from+15]&1) == 0 && board[from+15] != frame) //x
                        push_pawn (from+15, FALSE); //x
                    /* pawn captruing diagonally: */
                    if ((board[from+13]&1) == 0 && board[from+13] != frame) //x
                        push_pawn (from+13, FALSE); //x
                    /* ep move: */
                    if (ep_square == from+15) //x
                        push_pawn (from+15, TRUE); //x
                    /* ep move: */
                    else if (ep_square == from+13) //x
                        push_pawn (from+13, TRUE); //x
                    break;
                case (wknight):
                    /* use the knight offsets: */
                    push_knight (from-29); //x
                    push_knight (from-27); //x
                    push_knight (from-16); //x
                    push_knight (from-12); //x
                    push_knight (from+12); //x
                    push_knight (from+16); //x
                    push_knight (from+27); //x
                    push_knight (from+29); //x
                    break;
                case (wbishop):
                    /* use the bishop offsets: */
                    push_slide (from-15); //x
                    push_slide (from-13); //x
                    push_slide (from+13); //x
                    push_slide (from+15); //x
                    break;
                case (wrook):
                    /* use the rook offsets: */
                    push_slide (from-14); //x
                    push_slide (from-1);
                    push_slide (from+1);
                    push_slide (from+14); //x
                    break;
                case (wqueen):
                    /* use the queen offsets: */
                    push_slide (from-15); //x
                    push_slide (from-14); //x
                    push_slide (from-13); //x
                    push_slide (from-1);
                    push_slide (from+1);
                    push_slide (from+13); //x
                    push_slide (from+14); //x
                    push_slide (from+15); //x
                    break;
                case (wking):
                    /* use the king offsets for 'normal' moves: */
                    push_king (from-15); //x
                    push_king (from-14); //x
                    push_king (from-13); //x
                    push_king (from-1);
                    push_king (from+1);
                    push_king (from+13); //x
                    push_king (from+14); //x
                    push_king (from+15); //x
                    /* castling moves: */
                    // 30->35 white king now there
                    if (from == F1 && !moved[F1] && !captures && (Variant != Suicide || Giveaway == TRUE)) { //x
                        /* kingside: */
                        if (!moved[I1] && board[I1] == wrook) //x
                            if (board[G1] == npiece && board[H1] == npiece) //x
                                push_king_castle (from+2, wck);
                        /* queenside: */
                        if (!moved[B1] && board[B1] == wrook) //x
                            if (board[C1] == npiece && board[D1] == npiece && board[E1] == npiece) //x
                                push_king_castle (from-2, wcq);
                    }
                    break;
                default:
                    break;
            }
        }
    } else {
        /* generate black moves, if it is black to move: */
        //printf ("DEBUG: gen (): Black to move\n");
        for (a = 1, j = 1;
                (a <= piece_count) && 
                 (((Variant != Suicide) && !kingcap) || ((Variant == Suicide) && (fcaptures == captures)))
                ; j++) {

            i = pieces[j];

            if (i < 1)
                continue;
            else
                a++;

            from = i; 
            gfrom = i;

            switch (board[from]) {
                case (bpawn):
                    /* pawn moves up one square: */
                    if (board[from-14] == npiece) { //x
                        /* only promotions when captures == TRUE */
                        if (rank (from) == 2 && ((Variant != Suicide) && (Variant != Losers))) {
                            push_pawn (from-14, FALSE); //x
                        } else if (!captures) {
                            push_pawn (from-14, FALSE); //x

                            /* pawn moving up two squares on its first move: */
                            if (rank(from) == 9 && board[from-28] == npiece) //x
                                push_pawn_simple (from-28); //x
                        }
                    }
                    /* pawn capturing diagonally: */
                    if ((board[from-15]&1) == 1 && board[from-15] != npiece) //x
                        push_pawn (from-15, FALSE); //x
                    /* pawn capturing diagonally: */
                    if ((board[from-13]&1) == 1 && board[from-13] != npiece) //x
                        push_pawn (from-13, FALSE); //x
                    /* ep move: */
                    if (ep_square == from-15) //x
                        push_pawn (from-15, TRUE); //x
                    /* ep move: */
                    else if (ep_square == from-13) //x
                        push_pawn (from-13, TRUE); //x
                    break;
                case (bknight):
                    /* use the knight offsets: */
                    push_knight (from-29); //x
                    push_knight (from-27); //x
                    push_knight (from-16); //x
                    push_knight (from-12); //x
                    push_knight (from+12); //x
                    push_knight (from+16); //x
                    push_knight (from+27); //x
                    push_knight (from+29); //x
                    break;
                case (bbishop):
                    /* use the bishop offsets: */
                    push_slide (from-15); //x
                    push_slide (from-13); //x
                    push_slide (from+13); //x
                    push_slide (from+15); //x
                    break;
                case (brook):
                    /* use the rook offsets: */
                    push_slide (from-14); //x
                    push_slide (from-1);
                    push_slide (from+1);
                    push_slide (from+14); //x
                    break;
                case (bqueen):
                    /* use the queen offsets: */
                    push_slide (from-15); //x
                    push_slide (from-14); //x
                    push_slide (from-13); //x
                    push_slide (from-1);
                    push_slide (from+1);
                    push_slide (from+13); //x
                    push_slide (from+14); //x
                    push_slide (from+15); //x
                    break;
                case (bking):
                    /* use the king offsets for 'normal' moves: */
                    push_king (from-15); //x
                    push_king (from-14); //x
                    push_king (from-13); //x
                    push_king (from-1);
                    push_king (from+1);
                    push_king (from+13); //x
                    push_king (from+14); //x
                    push_king (from+15); //x
                    /* castling moves: */
                    // 114->161 black king now there
                    if (from == F10 && !moved[F10] && !captures && (Variant != Suicide || Giveaway == TRUE)) { //x
                        /* kingside: */
                        if (!moved[I10] && board[I10] == brook) //x
                            if (board[G10] == npiece && board[H10] == npiece) //x
                                push_king_castle (from+2, bck);
                        /* queenside: */
                        if (!moved[B10] && board[B10] == brook) //x
                            if (board[C10] == npiece && board[D10] == npiece && board[E10] == npiece) //x
                                push_king_castle (from-2, bcq);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    if (((Variant == Crazyhouse) || (Variant == Bughouse)) && !captures && !kingcap) {
        if (white_to_move && 
                (holding[WHITE][wpawn] || holding[WHITE][wknight]
                 || holding[WHITE][wbishop] || holding[WHITE][wqueen]
                 || holding[WHITE][wrook]))
        {
            // from=26 from<118
            for (from = 30; from < 166; from++) { //x
                gfrom = from;

                switch (board[from]) {
                    case (frame):
                        from += 3;
                        continue;
                    case (npiece):
                        if(holding[WHITE][wpawn]) {  
                            if ((rank(from) != 10) && (rank(from) != 1)) { //x
                                try_drop(wpawn);
                            }
                        }   
                        if(holding[WHITE][wknight]) {
                            try_drop(wknight);
                        }
                        if(holding[WHITE][wbishop]) {
                            try_drop(wbishop);
                        }
                        if(holding[WHITE][wrook]) {
                            try_drop(wrook);
                        }	
                        if(holding[WHITE][wqueen]) {
                            try_drop(wqueen);
                        }
                }
            }
        }      
        else if (!white_to_move && 
                (holding[BLACK][bpawn] || holding[BLACK][bknight]
                 || holding[BLACK][bbishop] || holding[BLACK][bqueen]
                 || holding[BLACK][brook]))
        {
            // from=26 from<118
            for (from = 30; from < 166; from++) { //x
                gfrom = from;

                switch (board[from]) {
                    case (frame):
                        from += 3;
                        continue;
                    case (npiece):
                        if(holding[BLACK][bpawn]) {  
                            if ((rank(from) != 10) && (rank(from) != 1)) { //x
                                try_drop(bpawn);
                            }
                        }   
                        if(holding[BLACK][bknight]) {
                            try_drop(bknight);
                        }
                        if(holding[BLACK][bbishop]) {
                            try_drop(bbishop);
                        }
                        if(holding[BLACK][brook]) {
                            try_drop(brook);
                        }	
                        if(holding[BLACK][bqueen]) {
                            try_drop(bqueen);
                        }
                };
            };
        }
    }

    if ((Variant == Suicide) && fcaptures == TRUE && captures == FALSE) {
        captures = TRUE;
        numb_moves = 0;
        goto restart;
    }

    if (Variant == Suicide) kingcap = FALSE;
}


bool in_check (void) {
    /* return true if the side to move is in check: */

    if (Variant == Suicide) return FALSE;

    if (white_to_move == 1) {
        if (is_attacked (wking_loc, 0)) { return TRUE; }
    } else {
        if (is_attacked (bking_loc, 1)) { return TRUE; }
    }

    return FALSE;
}


bool f_in_check(move_s moves[], int m)
{
    int target = moves[m].target;
    int from = moves[m].from;
    int l;
    static const int knight_o[8] = {12, -12, 16, -16, 27, -27, 29, -29}; //x

    if (Variant == Suicide) return FALSE;

    if (white_to_move == 1) {
        /* is white king attacked */
        /* we are certain the king is not in check already,
         * as we would capture him in our ply */
        /* thus, we need to check if our move could possibly
         * put the king in check */
        /* this can either be a direct check, or a discover */

        switch (board[target]) {
            case bpawn:
                if (board[target-13] == wking || board[target-15] == wking) return TRUE; //x
                break;
            case bbishop:
                if (diagl(target) == diagl(wking_loc)) {
                    /* possible left diag check */
                    if (wking_loc < target) {
                        for (l = wking_loc+15; board[l] == npiece; l +=15); //x
                        if (l == target) return TRUE;
                    }
                    else
                    {
                        for (l = wking_loc-15; board[l] == npiece; l -=15); //x
                        if (l == target) return TRUE;
                    }
                } else if (diagr(target) == diagr(wking_loc)) {
                    if (wking_loc < target) {
                        for (l = wking_loc+13; board[l] == npiece; l +=13); //x
                        if (l == target) return TRUE;
                    } else {
                        for (l = wking_loc-13; board[l] == npiece; l -=13); //x
                        if (l == target) return TRUE;
                    }
                }
                break;
            case brook:
                if (file(target) == file(wking_loc)) {
                    if (wking_loc < target) {
                        for (l = wking_loc+14; board[l] == npiece; l +=14); //x
                        if (l == target) return TRUE;
                    } else {
                        for (l = wking_loc-14; board[l] == npiece; l -=14); //x
                        if (l == target) return TRUE;
                    }
                } else if (rank(target) == rank(wking_loc)) {
                    if (wking_loc < target) {
                        for (l = wking_loc+1; board[l] == npiece; l++);
                        if (l == target) return TRUE;
                    } else {
                        for (l = wking_loc-1; board[l] == npiece; l--);
                        if (l == target) return TRUE;
                    }
                }
                break;
            case bknight:
                for (l = 0; l < 8; l++) 
                    if ((wking_loc + knight_o[l]) == target) return TRUE;
                break;
            case bqueen:
                if (file(target) == file(wking_loc)) {
                    if (wking_loc < target) {
                        for (l = wking_loc+14; board[l] == npiece; l +=14); //x
                        if (l == target) return TRUE;
                    } else {
                        for (l = wking_loc-14; board[l] == npiece; l -=14); //x
                        if (l == target) return TRUE;
                    }
                } else if (rank(target) == rank(wking_loc)) {
                    if (wking_loc < target) {
                        for (l = wking_loc+1; board[l] == npiece; l +=1);
                        if (l == target) return TRUE;
                    } else {
                        for (l = wking_loc-1; board[l] == npiece; l -=1);
                        if (l == target) return TRUE;
                    }
                } else if (diagl(target) == diagl(wking_loc)) {
                    if (wking_loc < target) {
                        for (l = wking_loc+15; board[l] == npiece; l +=15); //x
                        if (l == target) return TRUE;
                    } else {
                        for (l = wking_loc-15; board[l] == npiece; l -=15); //x
                        if (l == target) return TRUE;
                    }
                } else if (diagr(target) == diagr(wking_loc)) {
                    if (wking_loc < target) {
                        for (l = wking_loc+13; board[l] == npiece; l +=13); //x
                        if (l == target) return TRUE;
                    } else {  
                        for (l = wking_loc-13; board[l] == npiece; l -=13); //x
                        if (l == target) return TRUE;
                    }
                }
                break;
            case bking:
                /* can only discover checks */
                /* castling is tricky */
                if (moves[m].castled) {
                    if (is_attacked (wking_loc, 0)) 
                        return TRUE;
                    else
                        return FALSE;
                }
                break;
        }

        /* drop move can never discover check */
        if (from == 0) return FALSE;

        /* this checks for discovered checks */
        if (rank(from) == rank(wking_loc)) {
            if (wking_loc > from) {
                for (l = wking_loc-1; board[l] == npiece; l--);    
                if (board[l] == brook || board[l] == bqueen) return TRUE;
            } else {
                for (l = wking_loc+1; board[l] == npiece; l++);
                if (board[l] == brook || board[l] == bqueen) return TRUE;
            }
        } else if (file(from) == file(wking_loc)) {
            if (wking_loc > from) {
                for (l = wking_loc-14; board[l] == npiece; l-=14);     //x
                if (board[l] == brook || board[l] == bqueen) return TRUE;
            } else {
                for (l = wking_loc+14; board[l] == npiece; l+=14); //x
                if (board[l] == brook || board[l] == bqueen) return TRUE;
            }
        } else if (diagl(from) == diagl(wking_loc)) {
            if (wking_loc > from) {
                for (l = wking_loc-15; board[l] == npiece; l-=15);     //x
                if (board[l] == bbishop || board[l] == bqueen) return TRUE;
            } else {
                for (l = wking_loc+15; board[l] == npiece; l+=15); //x
                if (board[l] == bbishop || board[l] == bqueen) return TRUE;
            }
        } else if (diagr(from) == diagr(wking_loc)) {
            if (wking_loc > from) {
                for (l = wking_loc-13; board[l] == npiece; l-=13);     //x
                if (board[l] == bbishop || board[l] == bqueen) return TRUE;
            } else {
                for (l = wking_loc+13; board[l] == npiece; l+=13); //x
                if (board[l] == bbishop || board[l] == bqueen) return TRUE;
            }
        }    

        return FALSE;
    } else {
        /* is black king attacked */
        switch (board[target]) {
            case wpawn:
                if (board[target+13] == bking || board[target+15] == bking) return TRUE; //x
                break;
            case wbishop:
                if (diagl(target) == diagl(bking_loc)) {
                    /* possible left diag check */
                    if (bking_loc < target) {
                        for (l = bking_loc+15; board[l] == npiece; l +=15); //x
                        if (l == target) return TRUE;
                    } else {
                        for (l = bking_loc-15; board[l] == npiece; l -=15); //x
                        if (l == target) return TRUE;
                    }
                } else if (diagr(target) == diagr(bking_loc)) {
                    if (bking_loc < target) {
                        for (l = bking_loc+13; board[l] == npiece; l +=13); //x
                        if (l == target) return TRUE;
                    } else {
                        for (l = bking_loc-13; board[l] == npiece; l -=13); //x
                        if (l == target) return TRUE;
                    }
                }
                break;
            case wrook:
                if (file(target) == file(bking_loc)) {
                    if (bking_loc < target) {
                        for (l = bking_loc+14; board[l] == npiece; l +=14); //x
                        if (l == target) return TRUE;
                    } else {
                        for (l = bking_loc-14; board[l] == npiece; l -=14); //x
                        if (l == target) return TRUE;
                    }
                } else if (rank(target) == rank(bking_loc)) {
                    if (bking_loc < target) {
                        for (l = bking_loc+1; board[l] == npiece; l++);
                        if (l == target) return TRUE;
                    } else {
                        for (l = bking_loc-1; board[l] == npiece; l--);
                        if (l == target) return TRUE;
                    }
                }
                break;
            case wknight:
                for (l = 0; l < 8; l++) 
                    if ((bking_loc + knight_o[l]) == target) return TRUE;
                break;
            case wqueen:
                if (file(target) == file(bking_loc)) {
                    if (bking_loc < target) {
                        for (l = bking_loc+14; board[l] == npiece; l +=14); //x
                        if (l == target) return TRUE;
                    } else {
                        for (l = bking_loc-14; board[l] == npiece; l -=14); //x
                        if (l == target) return TRUE;
                    }
                } else if (rank(target) == rank(bking_loc)) {
                    if (bking_loc < target) {
                        for (l = bking_loc+1; board[l] == npiece; l +=1);
                        if (l == target) return TRUE;
                    } else {
                        for (l = bking_loc-1; board[l] == npiece; l -=1);
                        if (l == target) return TRUE;
                    }
                } else if (diagl(target) == diagl(bking_loc)) {
                    if (bking_loc < target) {
                        for (l = bking_loc+15; board[l] == npiece; l +=15); //x
                        if (l == target) return TRUE;
                    } else {
                        for (l = bking_loc-15; board[l] == npiece; l -=15); //x
                        if (l == target) return TRUE;
                    }
                } else if (diagr(target) == diagr(bking_loc)) {
                    if (bking_loc < target) {
                        for (l = bking_loc+13; board[l] == npiece; l +=13); //x
                        if (l == target) return TRUE;
                    } else {
                        for (l = bking_loc-13; board[l] == npiece; l -=13); //x
                        if (l == target) return TRUE;
                    }
                }
                break;
            case wking:
                /* can only discover checks */
                if (moves[m].castled) { 
                    if (is_attacked (bking_loc, 1)) 
                        return TRUE;
                    else
                        return FALSE;
                }
                break;
        }

        if (from == 0) return FALSE;

        /* this checks for discovered checks */
        if (rank(from) == rank(bking_loc)) {
            if (bking_loc > from) {
                for (l = bking_loc-1; board[l] == npiece; l--);    
                if (board[l] == wrook || board[l] == wqueen) return TRUE;
            } else {
                for (l = bking_loc+1; board[l] == npiece; l++);
                if (board[l] == wrook || board[l] == wqueen) return TRUE;
            }
        } else if (file(from) == file(bking_loc)) {
            if (bking_loc > from) {
                for (l = bking_loc-14; board[l] == npiece; l-=14);     //x
                if (board[l] == wrook || board[l] == wqueen) return TRUE;
            } else {
                for (l = bking_loc+14; board[l] == npiece; l+=14); //x
                if (board[l] == wrook || board[l] == wqueen) return TRUE;
            }
        } else if (diagl(from) == diagl(bking_loc)) {
            if (bking_loc > from) {
                for (l = bking_loc-15; board[l] == npiece; l-=15);     //x
                if (board[l] == wbishop || board[l] == wqueen) return TRUE;
            } else {
                for (l = bking_loc+15; board[l] == npiece; l+=15); //x
                if (board[l] == wbishop || board[l] == wqueen) return TRUE;
            }
        } else if (diagr(from) == diagr(bking_loc)) {
            if (bking_loc > from) {
                for (l = bking_loc-13; board[l] == npiece; l-=13);     //x
                if (board[l] == wbishop || board[l] == wqueen) return TRUE;
            } else {
                for (l = bking_loc+13; board[l] == npiece; l+=13); //x
                if (board[l] == wbishop || board[l] == wqueen) return TRUE;
            }
        }

        return FALSE;
    }
}


int extended_in_check(void) {
    register int sq;
    static const int knight_o[8] = {10, -10, 14, -14, 23, -23, 25, -25}; //x

    if (Variant == Suicide) return 0;

    if (white_to_move == 1) {
        sq = board[wking_loc-14]; //x
        if (sq == brook || sq == bqueen) return 2;
        sq = board[wking_loc-1];
        if (sq == brook || sq == bqueen) return 2;
        sq = board[wking_loc+1];
        if (sq == brook || sq == bqueen) return 2;
        sq = board[wking_loc+14]; //x
        if (sq == brook || sq == bqueen) return 2;
        sq = board[wking_loc+15]; //x
        if (sq == bbishop || sq == bqueen || sq == bpawn) return 2;
        sq = board[wking_loc+13]; //x
        if (sq == bbishop || sq == bqueen || sq == bpawn) return 2;
        sq = board[wking_loc-13]; //x
        if (sq == bbishop || sq == bqueen) return 2;
        sq = board[wking_loc-13]; //x
        if (sq == bbishop || sq == bqueen) return 2;
        for (sq = 0; sq < 8; sq++) {
            if (board[wking_loc + knight_o[sq]] == bknight) return 2;
        }
        if (is_attacked (wking_loc, 0)) {
            if (Variant == Normal || Variant == Losers) return 2;
            else return 1;
        }
    } else {
        sq = board[bking_loc-14]; //x
        if (sq == wrook || sq == wqueen) return 2;
        sq = board[bking_loc-1];
        if (sq == wrook || sq == wqueen) return 2;
        sq = board[bking_loc+1];
        if (sq == wrook || sq == wqueen) return 2;
        sq = board[bking_loc+14]; //x
        if (sq == wrook || sq == wqueen) return 2;
        sq = board[bking_loc-15]; //x
        if (sq == wbishop || sq == wqueen || sq == wpawn) return 2;
        sq = board[bking_loc-13]; //x
        if (sq == wbishop || sq == wqueen || sq == wpawn) return 2;
        sq = board[bking_loc+13]; //x
        if (sq == wbishop || sq == wqueen) return 2;
        sq = board[bking_loc+15]; //x
        if (sq == wbishop || sq == wqueen) return 2;
        for (sq = 0; sq < 8; sq++) {
            if (board[bking_loc + knight_o[sq]] == wknight) return 2;
        }
        if (is_attacked (bking_loc, 1)) {
            if (Variant == Normal || Variant == Losers) return 2;
            else return 1;
        }
    }

    return 0;
};


void make (move_s moves[], int i) {
    /* make a move */

    /* rather than writing out from[i].from, from[i].target, etc. all over
       the place, just make a copy of them here: */
    int ep, from, target, captured, promoted, castled, find_slot;
    ep = moves[i].ep;
    from = moves[i].from;
    target = moves[i].target;
    captured = moves[i].captured;
    promoted = moves[i].promoted;
    castled = moves[i].castled;

    //if (i == 0) {
    //    printf ("dbg: make(): i = %d ep = %d from = %d target = %d captured = %d promoted = %d castled = %d\n", i, ep, from, target, captured, promoted, castled);
    //}

    //if ((moves[i].target == 0) || ((moves[i].from != 0) && ((board[moves[i].from] == npiece) || board[moves[i].from] == frame)))
    //   DIE;

    /* clear the en passant rights: */
    path_x[ply].epsq = ep_square;

    ep_square = 0;

    /* update the 50 move info: */
    path_x[ply].fifty = fifty;

    /* ignore piece drops...50move draw wont happen anyway */
    if (board[from] == wpawn || board[from] == bpawn || board[target] != npiece) {
        fifty = 0;
    } else {
        fifty++;
    }

    if (from == 0) { 
        /* drop move */
        /* Drop moves are handled fully seperate because we exepect to encouter
           lots of them and we try to skip as many checks as possible.
           Note that the critical path for drop moves is very short.
           Also, we have to handle pieces[] and squares[] specially   */

        /* new piece on board */
        piece_count++;

        /* find first empty slot in pieces[] */
        for(find_slot = 1; (pieces[find_slot] != 0); find_slot++)
            assert(find_slot < 99); //x

        /* add to piece array, set piece-square pointer */
        pieces[find_slot] = target;

        path_x[ply].was_promoted = is_promoted[find_slot];
        is_promoted[find_slot] = 0;

        /* set square->piece pointer */
        squares[target] = find_slot;
        //  moved[target] = 1;

        //if (promoted <= frame || promoted >= npiece)
        // 	DIE;

        assert(promoted > frame && promoted < npiece);

        DropremoveHolding(promoted, ToMove);

        /* piece went off holding but onto board */
        AddMaterial(promoted);

        /* put our piece on the board */
        board[target] = promoted;

        Hash(promoted,target);

        white_to_move ^= 1;
        ply++;

        return;
    } else {
        path_x[ply].was_promoted = is_promoted[squares[target]];

        /* update the "general" pieces[] / squares[] info (special moves need
           special handling later): */
        path_x[ply].cap_num = squares[target];
        pieces[squares[target]] = 0;
        pieces[squares[from]] = target;
        squares[target] = squares[from];
        squares[from] = 0;

        /* update the piece count & add Holdings */
        if (!ep) {
            switch (board[target]) {
                case (npiece): break;
                default:
                       if (Variant == Bughouse || Variant == Crazyhouse)
                       {
                           if (path_x[ply].was_promoted)
                           {
                               addHolding(SwitchPromoted(board[target]), ToMove);
                           }
                           else
                           { 
                               addHolding(SwitchColor(board[target]), ToMove);
                           }
                       }

                       RemoveMaterial(board[target]);

                       /* remove captured piece */
                       Hash(board[target], target);

                       piece_count--;
                       break;
            }
        }

        /* white pawn moves: */
        if (board[from] == wpawn) {
            /* look for a promotion move: */
            if (promoted) {
                board[target] = promoted;
                board[from] = npiece;
                moved[target]++;
                moved[from]++;
                white_to_move ^= 1;

                is_promoted[squares[target]] = 1;

                /* remove pawn */
                Hash(wpawn, from);
                /* add new stuff */
                Hash(promoted, target);

                RemoveMaterial(wpawn);
                AddMaterial(promoted);

                ply++;

                return;
            }

            /* look for an en passant move: */
            if (ep) {
                /* remove pawn */
                Hash(wpawn, from);
                /* remove ep pawn */
                Hash(bpawn, target-14); //x
                /* add target pawn */
                Hash(wpawn, target);

                RemoveMaterial(bpawn);

                board[target] = wpawn;
                board[from] = npiece;

                addHolding(wpawn, WHITE);
                piece_count--;

                board[target-14] = npiece; //x
                moved[target]++;
                moved[from]++;
                moved[target-14]++; //x
                white_to_move ^= 1;
                path_x[ply].cap_num = squares[target-14]; //x

                pieces[squares[target-14]] = 0; //x
                squares[target-14] = 0; //x

                ply++;

                return;
            }

            /* otherwise, we have a "regular" pawn move: */
            /* first check to see if we've moved a pawn up 2 squares: */
            if (target == from+28) //x
                ep_square = from+14; //x

            Hash(wpawn, from);
            Hash(wpawn, target);

            board[target] = wpawn;
            board[from] = npiece;
            moved[target]++;
            moved[from]++;
            white_to_move ^= 1;

            ply++;

            return;
        }

        /* black pawn moves: */
        if (board[from] == bpawn) {
            /* look for a promotion move: */
            if (promoted) {
                board[target] = promoted;
                board[from] = npiece;
                moved[target]++;
                moved[from]++;
                white_to_move ^= 1;

                is_promoted[squares[target]] = 1;

                /* remove pawn */
                Hash(bpawn, from);
                /* add new stuff */
                Hash(promoted, target);

                RemoveMaterial(bpawn);
                AddMaterial(promoted);

                ply++;

                return;
            }

            /* look for an en passant move: */
            if (ep) {

                /* remove pawn */
                Hash(bpawn, from);
                /* remove ep pawn */
                Hash(wpawn, target+14); //x
                /* add target pawn */
                Hash(bpawn, target);

                RemoveMaterial(wpawn);

                board[target] = bpawn;
                board[from] = npiece;

                addHolding(bpawn, BLACK);
                piece_count--;

                board[target+14] = npiece; //x
                moved[target]++;
                moved[from]++;
                moved[target+14]++; //x
                white_to_move ^= 1;
                path_x[ply].cap_num = squares[target+14]; //x
                pieces[squares[target+14]] = 0; //x
                squares[target+14] = 0; //x

                ply++;

                return;
            }

            /* otherwise, we have a "regular" pawn move: */
            /* first check to see if we've moved a pawn down 2 squares: */
            if (target == from-28) //x
                ep_square = from-14; //x

            board[target] = bpawn;
            board[from] = npiece;
            moved[target]++;
            moved[from]++;
            white_to_move ^= 1;

            Hash(bpawn, from);
            Hash(bpawn, target);

            ply++;

            return;
        }

        /* piece moves, other than the king: */
        if (board[from] != wking && board[from] != bking) {
            Hash(board[from], from);
            Hash(board[from], target);

            board[target] = board[from];
            board[from] = npiece;
            moved[target]++;
            moved[from]++;
            white_to_move ^= 1;

            ply++;

            return;
        }

        /* otherwise, we have a king move of some kind: */
        /* White king moves first: */
        if (board[from] == wking) {
            /* record the new white king location: */
            wking_loc = target;

            /* perform the white king's move: */
            board[target] = wking;
            board[from] = npiece;
            moved[target]++;
            moved[from]++;
            white_to_move ^= 1;

            Hash(wking, from);
            Hash(wking, target);

            /* check for castling: */
            /* check for white kingside castling: */
            // wking was at 30 (8x8)
            // wking is now at 35 (10x10)
            if (castled == wck) {
                board[I1] = npiece; //x
                board[G1] = wrook; //x
                moved[I1]++; //x
                moved[G1]++; //x
                white_castled = wck;
                pieces[squares[I1]] = G1; //x
                squares[G1] = squares[I1]; //x
                squares[I1] = 0; //x

                Hash(wrook, I1); //x
                Hash(wrook, G1); //x

                ply++;

                return;
            }

            /* check for white queenside castling: */
            else if (castled == wcq) {
                board[B1] = npiece; //x
                board[E1] = wrook; //x
                moved[B1]++; //x
                moved[E1]++; //x
                white_castled = wcq;
                pieces[squares[B1]] = E1; //x
                squares[E1] = squares[B1]; //x
                squares[B1] = 0; //x

                Hash(wrook, B1); //x
                Hash(wrook, E1); //x

                ply++;

                return;
            }

            ply++;

            return;
        } else {
            /* now we have only black king moves left: */
            /* record the new black king location: */
            bking_loc = target;

            /* perform the black king's move: */
            board[target] = bking;
            board[from] = npiece;
            moved[target]++;
            moved[from]++;
            white_to_move ^= 1;

            Hash(bking, from);
            Hash(bking, target);

            if (castled == bck) {
                /* check for castling: */
                /* check for black kingside castling: */
                // bking was at 114 (8x8)
                // bking is at 161 (10x10)
                board[I10] = npiece; //x
                board[G10] = brook; //x
                moved[I10]++; //x
                moved[G10]++; //x
                black_castled = bck;
                pieces[squares[I10]] = G10; //x
                squares[G10] = squares[I10]; //x
                squares[I10] = 0; //x

                Hash(brook, I10); //x
                Hash(brook, G10); //x

                ply++;

                return;
            } else if (castled == bcq) {
                /* check for black queenside castling: */
                // bking was at 114 (8x8)
                // bking is at 161 (10x10)
                board[B10] = npiece; //x
                board[E10] = brook; //x
                moved[B10]++; //x
                moved[E10]++; //x
                black_castled = bcq;
                pieces[squares[B10]] = E10; //x
                squares[E10] = squares[B10]; //x
                squares[B10] = 0; //x

                Hash(brook, B10); //x
                Hash(brook, E10); //x

                ply++;

                return;
            }
        }
        ply++;

        return;
    }
}

void add_move(int Ptarget, int Ppromoted) {
  genfor[numb_moves].from = gfrom;
  genfor[numb_moves].target = Ptarget;
  genfor[numb_moves].captured = npiece;
  genfor[numb_moves].castled = no_castle;
  genfor[numb_moves].promoted = Ppromoted;
  genfor[numb_moves].ep = FALSE;
  numb_moves++;

  return;	
}

void add_capture(int Ptarget,
		 int Pcaptured,
		 int Ppromoted,
		 int Pep)
{
    if ((Variant != Suicide) && (Pcaptured == wking || Pcaptured == bking))
    {
        kingcap = TRUE;
        return;
    }
    else
        if (Pcaptured != npiece) fcaptures = TRUE; 

    genfor[numb_moves].from = gfrom;
    genfor[numb_moves].target = Ptarget;
    genfor[numb_moves].captured = Pcaptured;
    genfor[numb_moves].castled = no_castle;
    genfor[numb_moves].promoted = Ppromoted;
    genfor[numb_moves].ep = Pep;
    numb_moves++;

    return;
}


void try_drop (int ptype) {
  genfor[numb_moves].from = 0;
  genfor[numb_moves].target = gfrom;
  genfor[numb_moves].captured = npiece;
  genfor[numb_moves].castled = no_castle;
  genfor[numb_moves].promoted = ptype;
  genfor[numb_moves].ep = FALSE;
  numb_moves++;
  
  return;  
}


void push_king_castle (int Ptarget, int Pcastle_type) {
  genfor[numb_moves].from = gfrom;
  genfor[numb_moves].target = Ptarget;
  genfor[numb_moves].captured = npiece;
  genfor[numb_moves].castled = Pcastle_type;
  genfor[numb_moves].promoted = 0;
  genfor[numb_moves].ep = FALSE;
  numb_moves++;
  
  return;
}

void push_king (int target) {
    /* add king moves to the moves array */

    /* first see if the move will take the king off the board: */
    if (board[target] == frame)
        return;

    /* check to see if we have a non capture when in qsearch: */
    if (board[target] == npiece && captures)
        return;

    /* non-capture, 'normal' king moves: */
    if (board[target] == npiece) {
        add_move(target, 0);
        return;
    }

    /* 'normal' capture moves by the king: */
    else if ((board[target]&1) != (board[gfrom]&1)) {
        add_capture(target, board[target], 0, FALSE);
        return;
    }

    /* no more possible moves for the king, so return: */
    return;
}


void push_knighT (int target) {
    /* add knight moves to the moves array */

    /* check to see if we have a non capture when in qsearch: */
    if (board[target] == npiece && captures)
        return;

    /* check for a non-capture knight move: */
    if (board[target] == npiece) {
        add_move(target, 0);
        return;
    }

    /* check for a capture knight move: */
    else if ((board[target]&1) != (board[gfrom]&1)) {
        add_capture(target, board[target], 0, FALSE);
        return;
    }

    /* no more possible moves left for the knight, so return: */
    return;
}


void push_pawn (int target, bool is_ep) {
    /* add pawn moves to the moves array */

    int captured_piece;

    /* check to see if it's an ep move: */
    if (is_ep) {
        if (board[gfrom] == wpawn) {
            add_capture(target, bpawn, 0, TRUE);
            return;
        }
        else {
            add_capture(target, wpawn, 0, TRUE);
            return;
        }
    }

    /* record which piece we are taking, so we don't have to compute it over
       and over again: */
    captured_piece = board[target];

    /* look for a white promotion move: */
    if (board[gfrom] == wpawn && rank(gfrom) == 9) { //x
        add_capture(target, captured_piece, wqueen, FALSE);
        add_capture(target, captured_piece, wrook, FALSE);
        add_capture(target, captured_piece, wbishop, FALSE);
        add_capture(target, captured_piece, wknight, FALSE);
        if (Variant == Suicide)
            add_capture(target, captured_piece, wking, FALSE);
        /* we've finished generating all the promotions: */
        return;
    }

    /* look for a black promotion move: */
    else if (board[gfrom] == bpawn && rank(gfrom) == 2) {
        add_capture(target, captured_piece, bqueen, FALSE);
        add_capture(target, captured_piece, brook, FALSE);
        add_capture(target, captured_piece, bbishop, FALSE);
        add_capture(target, captured_piece, bknight, FALSE);
        if (Variant == Suicide)
            add_capture(target, captured_piece, bking, FALSE);
        /* we've finished generating all the promotions: */
        return;
    }

    /* otherwise, we have a normal pawn move: */
    else {
        add_capture(target, captured_piece, 0, FALSE);
        return;
    }

    /* the function should never get here, but just for completeness: */
    return;
}

void push_pawn_simple (int target) {
    /* add pawn moves to the moves array */

    add_move(target, 0);
    return;
}

void push_slidE (int target) {
    /* add moves for sliding pieces to the moves array */

    int offset;
    int mycolor;

    /* check to see if we have gone off the board first: */
    //  if (board[target] == frame)
    //   return;

    /* init variables: */
    offset = target - gfrom;
    mycolor = board[gfrom]&1;

    /* loop until we hit the edge of the board, or another piece: */
    do {
        /* case when the target is an empty square: */
        if (board[target] == npiece) {
            // FIXME: WTF is this? how can one capture an empty square?
            if (!captures) {
                add_move(target, 0);
            }
            target += offset;
        } else if ((board[target]&1) != mycolor) {
            /* case when an enemy piece is hit: */
            add_capture(target, board[target], 0, FALSE);
            break;
        }

        /* otherwise, we have hit a friendly piece (or edge of board): */
        else
            break;
    } while (board[target] != frame);

    /* we have finished generating all of the sliding moves, so return: */
    return;
}


void unmake (move_s moves[], int i) {
    /* un-make a move */

    /* rather than writing out from[i].from, from[i].target, etc. all over
       the place, just make a copy of them here: */
    int ep, from, target, captured, promoted, castled;
    ep = moves[i].ep;
    from = moves[i].from;
    target = moves[i].target;
    captured = moves[i].captured;
    promoted = moves[i].promoted;
    castled = moves[i].castled;

    //if ((moves[i].target == 0) || ((moves[i].target != 0) && (board[moves[i].target] == npiece)))
    //   DIE;

    ply--;

    //printf("%d ", ply);

    ep_square = path_x[ply].epsq;

    /* update the 50 move info: */
    fifty = path_x[ply].fifty;

    /* drop move */ 
    if (from == 0) {
        /* Drop moves are hanled fully seperate because we exepect to encouter
           lots of them and we try to skip as many checks as possible.
           Note that the critical path for drop moves is very short.
           Also, we have to handle pieces[] and squares[] specially   */
        /* remove from piece array, unset piece-square pointer */

        pieces[squares[target]] = 0;
        is_promoted[squares[target]] = path_x[ply].was_promoted;

        /* unset square->piece pointer */
        squares[target] = 0;
        //     moved[target] = 0;

        piece_count--;
        assert(promoted < npiece && promoted > frame);
        DropaddHolding(promoted, NotToMove);
        RemoveMaterial(promoted);

        /* restore board, either no piece or ep square */
        board[target] = captured;
        Hash(promoted,target);
        white_to_move ^= 1;
        return;
    } else {
        /* update the "general" pieces[] / squares[] info (special moves need
           special handling later): */

        squares[from] = squares[target];
        squares[target] = path_x[ply].cap_num;
        pieces[squares[target]] = target;
        pieces[squares[from]] = from;

        is_promoted[squares[target]] = path_x[ply].was_promoted;

        /* update the piece count for determining opening/middlegame/endgame stage */
        if (!ep)
        {
            switch (captured) {
                case (npiece): break;
                default:
                       if (Variant == Bughouse || Variant == Crazyhouse)
                       {
                           if (is_promoted[squares[target]]) {
                               removeHolding(SwitchPromoted(captured), NotToMove); 
                           } else { 
                               removeHolding(SwitchColor(captured), NotToMove);
                           } 
                       }

                       Hash(captured, target);
                       AddMaterial(captured);
                       piece_count++; 
                       break;
            }
        }

        /* white pawn moves: */
        if (board[target] == wpawn) {
            /* look for an en passant move: */
            if (ep) {

                Hash(wpawn, target);
                Hash(wpawn, from);
                Hash(bpawn, target-14); //x

                board[target] = npiece;
                board[from] = wpawn;

                AddMaterial(bpawn);

                removeHolding(wpawn, WHITE);
                piece_count++;

                board[target-14] = bpawn; //x
                moved[target]--;
                moved[from]--;
                moved[target-14]--; //x
                white_to_move ^= 1;
                squares[target-14] = path_x[ply].cap_num; //x
                pieces[path_x[ply].cap_num] = target-14; //x
                squares[target] = 0;
                return;
            }

            /* otherwise, we have a "regular" pawn move: */
            Hash(wpawn, from);
            Hash(wpawn, target);

            board[target] = captured;
            board[from] = wpawn;
            moved[target]--;
            moved[from]--;
            white_to_move ^= 1;
            return;

        }

        /* black pawn moves: */
        if (board[target] == bpawn) {
            /* look for an en passant move: */
            if (ep) {

                Hash(bpawn, target);
                Hash(bpawn, from);
                Hash(wpawn, target+14); //x

                board[target] = npiece;
                board[from] = bpawn;

                AddMaterial(wpawn);

                removeHolding(bpawn, BLACK);
                piece_count++;

                board[target+14] = wpawn; //x
                moved[target]--;
                moved[from]--;
                moved[target+14]--; //x
                white_to_move ^= 1;
                squares[target+14] = path_x[ply].cap_num; //x
                pieces[path_x[ply].cap_num] = target+14; //x
                squares[target] = 0;
                return;
            }

            Hash(bpawn, from);
            Hash(bpawn, target);

            /* otherwise, we have a "regular" pawn move: */
            board[target] = captured;
            board[from] = bpawn;
            moved[target]--;
            moved[from]--;
            white_to_move ^= 1;
            return;

        }

        /* piece moves, other than the king: */
        if (board[target] != wking && board[target] != bking && !promoted) {
            board[from] = board[target];
            board[target] = captured;
            moved[target]--;
            moved[from]--;
            white_to_move ^= 1;

            Hash(board[from], target);
            Hash(board[from], from);

            return;
        }

        /* look for a promotion move: */
        if (promoted) {
            /* white promotions: */
            if (board[target]%2) {
                board[target] = captured;
                board[from] = wpawn;
                moved[target]--;
                moved[from]--;
                white_to_move ^= 1;

                Hash(wpawn, from);
                Hash(promoted, target);

                RemoveMaterial(promoted);
                AddMaterial(wpawn);

                return;
            }

            /* black promotions: */
            board[target] = captured;
            board[from] = bpawn;
            moved[target]--;
            moved[from]--;
            white_to_move ^= 1;

            Hash(bpawn, from);
            Hash(promoted, target);

            RemoveMaterial(promoted);
            AddMaterial(bpawn);

            return;
        }

        /* otherwise, we have a king move of some kind: */
        /* White king moves first: */
        if (board[target] == wking) {
            /* record the new white king location: */
            wking_loc = from;

            /* perform the white king's move: */
            board[target] = captured;
            board[from] = wking;
            moved[target]--;
            moved[from]--;
            white_to_move ^= 1;

            Hash(wking, from);
            Hash(wking, target);

            /* check for castling: */
            /* check for white kingside castling: */
            if (castled == wck) {
                board[I1] = wrook; //x
                board[G1] = npiece; //x
                moved[I1]--; //x
                moved[G1]--; //x
                white_castled = no_castle;
                squares[I1] = squares[G1]; //x
                squares[G1] = 0; //x
                pieces[squares[I1]] = I1; //x

                Hash(wrook, I1); //x
                Hash(wrook, G1); //x

                return;
            }

            /* check for white queenside castling: */
            else if (castled == wcq) {
                board[B1] = wrook; //x
                board[E1] = npiece; //x
                moved[B1]--; //x
                moved[E1]--; //x
                white_castled = no_castle;
                squares[B1] = squares[E1]; //x
                squares[E1] = 0; //x
                pieces[squares[B1]] = B1; //x

                Hash(wrook, E1); //x
                Hash(wrook, B1); //x

                return;
            }

            return;
        }

        /* now we have only black king moves left: */
        else {
            /* record the new black king location: */
            bking_loc = from;

            /* perform the black king's move: */
            board[target] = captured;
            board[from] = bking;
            moved[target]--;
            moved[from]--;
            white_to_move ^= 1;

            Hash(bking, from);
            Hash(bking, target);

            /* check for castling: */
            /* check for black kingside castling: */
            if (castled == bck) {
                board[I10] = brook; //x
                board[G10] = npiece; //x
                moved[I10]--; //x
                moved[G10]--; //x
                black_castled = no_castle;
                squares[I10] = squares[G10]; //x
                squares[G10] = 0; //x
                pieces[squares[I10]] = I10; //x

                Hash(brook, I10); //x
                Hash(brook, G10); //x

                return;
            }

            /* check for black queenside castling: */
            // bking was at 114 (8x8)
            // bking is at 161 (10x10)
            else if (castled == bcq) {
                board[B10] = brook; //x
                board[E10] = npiece; //x
                moved[B10]--; //x
                moved[E10]--; //x
                black_castled = no_castle;
                squares[B10] = squares[E10]; //x
                squares[E10] = 0; //x
                pieces[squares[B10]] = B10; //x

                Hash(brook, B10); //x
                Hash(brook, E10); //x

                return;
            }
        }
    }
    return;
}

