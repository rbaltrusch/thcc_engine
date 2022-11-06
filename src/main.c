/*
Chess engine
R. Baltrusch
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "main.h"

int chess_generate_random_number(const int start, const int end)
{
    return start + rand() / (RAND_MAX / (end - start + 1) + 1);
}

/*
Initialises the supplied board with the standard piece configuration.
It accepts a FEN-notation string; however, it is currently ignored.
*/
chess_error_code chess_init_board(int board[BOARD_SIZE], const char string[])
{
    int initial_board[BOARD_SIZE] = {
        BLACK_ROOK_CASTLE, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING_CASTLE, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK_CASTLE,
        BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN,
        WHITE_ROOK_CASTLE, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN, WHITE_KING_CASTLE, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK_CASTLE};

    for (int i = 0; i < BOARD_SIZE; i++)
    {
        board[i] = initial_board[i];
    }
    return 0;
}

// Logs the supplied message if the supplied log_level is not smaller than the global constant LOG_LEVEL
void chess_log_message(const char string[], int log_level)
{
    if (log_level < LOG_LEVEL)
    {
        return;
    }
    printf("%i: %s", log_level, string);
}

/*
 */
chess_Coordinates chess_convert_to_board_coordinates(int coordinate)
{
    return (chess_Coordinates){coordinate % BOARD_ROW_SIZE, coordinate / BOARD_ROW_SIZE};
}

/*
Converts x and y board coordinates to a number between 0 and 63 (included).

Board coordinates range from 0 to 7, counting from top to bottom, left to right,
with the top left corner (black-side) square A8 being square (x=0, y=0).
*/
int chess_convert_from_board_coordinates(const int x, const int y)
{
    return y * BOARD_ROW_SIZE + x;
}

// Returns 1 if index is out of the range (0, BOARD_SIZE), otherwise 0
bool chess_check_board_index_out_of_range(const int index)
{
    return index < 0 || index >= BOARD_SIZE;
}

//
bool chess_check_board_coordinates_out_of_range(const int x, const int y)
{
    return (x < 0 || x >= BOARD_ROW_SIZE || y < 0 || y >= BOARD_ROW_SIZE);
}

// Returns 1 if move origin or destination are out of range, otherwise 0
bool chess_check_move_out_of_range(const chess_Move *move)
{
    return chess_check_board_index_out_of_range(move->destination) || chess_check_board_index_out_of_range(move->origin);
}

bool chess_is_white_or_empty(const int piece)
{
    return piece < BLACK_PAWN;
}

bool chess_is_white(const int piece)
{
    return piece > EMPTY && piece < BLACK_PAWN;
}

bool chess_is_black(const int piece)
{
    return piece >= BLACK_PAWN;
}

bool chess_is_pawn(const int piece)
{
    return piece == WHITE_PAWN || piece == BLACK_PAWN;
}

int chess_get_final_move_destination(chess_MovesLinkedList *moves, const int default_value)
{
    if (NULL == moves)
        return default_value;

    chess_MovesLinkedListNode *tail = moves->tail;
    if (NULL == tail)
        return default_value;

    chess_Move *move = tail->move;
    return NULL == move ? default_value : move->destination;
}

// Prints the chess board in rows of 8, starting from the top-left square (A8).
chess_error_code chess_print_board(const int board[BOARD_SIZE])
{
    for (int y = 0; y < BOARD_ROW_SIZE; y++)
    {
        for (int x = 0; x < BOARD_ROW_SIZE; x++)
        {
            int coordinate = chess_convert_from_board_coordinates(x, y);
            if (chess_check_board_index_out_of_range(coordinate))
            {
                chess_log_message("Invalid (x, y) board coordinates supplied!", ERROR);
                return 1;
            }

            int piece = board[coordinate];
            if (piece > 9)
                printf("%i ", piece);
            else
                printf("%i  ", piece);
        }
        printf("\n");
    }
    return 0;
}

/*
Updates the board with the supplied move.
*/
chess_error_code chess_update_board_with_move(int board[BOARD_SIZE], chess_Move *move)
{
    if (chess_check_move_out_of_range(move))
    {
        chess_log_message("Invalid move supplied!", ERROR);
        return 1;
    }

    // regular move
    board[move->destination] = board[move->origin];
    board[move->origin] = EMPTY;

    // TODO account for castling
    // TODO account for en-passant
    // TODO account for promotion
    return 0;
}

//
chess_Move *chess_create_move(const int board_index, const int new_board_index)
{
    chess_Move *move = malloc(sizeof(chess_Move));
    move->origin = board_index;
    move->destination = new_board_index;
    return move;
}

/*
Appends move from board_index to new_position and returns pointer to it.
Returns NULL if this is not possible due to a blocking piece or because the new position would be out of bounds.
*/
chess_Move *chess_create_non_capturing_move(const int board[BOARD_SIZE], const int board_index, const int new_board_index)
{
    if (chess_check_board_index_out_of_range(new_board_index))
        return NULL;

    int piece = board[new_board_index];
    if (EMPTY != piece)
        return NULL;

    return chess_create_move(board_index, new_board_index);
}

/*
 */
chess_error_code chess_append_move(chess_MovesLinkedList *moves, chess_Move *move)
{
    if (NULL == move || NULL == moves)
        return 1;

    chess_MovesLinkedListNode *node = malloc(sizeof(chess_MovesLinkedListNode));
    node->move = move;
    node->next_node = NULL;

    if (NULL == moves->head)
    {
        moves->head = node;
        moves->tail = node;
        moves->length = 1;
    }
    else
    {
        moves->tail->next_node = node;
        moves->tail = node;
        (moves->length)++;
    }
    return 0;
}

chess_error_code chess_free_moves(chess_MovesLinkedList *moves)
{
    if (NULL == moves)
        return 0;

    chess_MovesLinkedListNode *current_node = moves->head;
    while (current_node != NULL)
    {
        chess_MovesLinkedListNode *next_node = current_node->next_node;
        free(current_node->move);
        free(current_node);
        current_node = next_node;
    }
}

chess_Move *chess_get_move(chess_MovesLinkedList *moves, const int index)
{
    if (NULL == moves)
        return NULL;

    int current_index = 0;
    chess_MovesLinkedListNode *current_node = moves->head;
    while(current_node != NULL)
    {
        if (index == current_index)
            return current_node->move;
        current_node = current_node->next_node;
        current_index++;
    }
    return NULL;
}

chess_error_code _chess_compute_offset_move(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker, chess_Coordinates offset, bool must_capture)
{
    // TODO: check for discovered check
    chess_Coordinates current = chess_convert_to_board_coordinates(board_index);
    int x = current.x + offset.x;
    int y = current.y + offset.y;
    if (chess_check_board_coordinates_out_of_range(x, y))
    {
        return 0;
    }

    int new_board_index = chess_convert_from_board_coordinates(x, y);
    int piece = board[new_board_index];
    bool is_team = (*(checker))(piece);
    if (!is_team && !(must_capture && EMPTY == piece))
    {
        chess_Move *move = chess_create_move(board_index, new_board_index);
        return chess_append_move(moves, move);
    }
    return 0;
}

chess_error_code chess_compute_offset_array_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker, const chess_Coordinates offsets[INDEX_OFFSETS])
{
    chess_Coordinates current = chess_convert_to_board_coordinates(board_index);
    for (int i = 0; i < INDEX_OFFSETS; i++)
    {
        chess_error_code error_code = _chess_compute_offset_move(board, board_index, moves, checker, offsets[i], false);
        if (error_code)
            return error_code;
    }
    return 0;
}

chess_error_code _chess_determine_and_set_enemy_en_passant_pawns(int board[BOARD_SIZE], const int x, const int y, chess_TeamChecker checker)
{
    if (chess_check_board_coordinates_out_of_range(x, y))
        return 0;

    int index = chess_convert_from_board_coordinates(x, y);
    int piece = board[index];
    bool is_team = (*(checker))(piece);
    if (!is_team && chess_is_pawn(piece))
    {
        board[index] += EN_PASSANT_UPGRADE_INCREMENT;
    }
    return 0;
}

// TODO: use this function when using moves by en passant pawns
chess_error_code chess_determine_and_set_enemy_en_passant_pawns(int board[BOARD_SIZE], const int board_index, chess_TeamChecker checker)
{
    chess_Coordinates current = chess_convert_to_board_coordinates(board_index);
    chess_error_code error_code = _chess_determine_and_set_enemy_en_passant_pawns(board, current.x - 1, current.y, checker);
    error_code += _chess_determine_and_set_enemy_en_passant_pawns(board, current.x + 1, current.y, checker);
    return error_code;
}

chess_error_code chess_compute_black_pawn_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker)
{
    chess_error_code error_code = _chess_compute_offset_move(board, board_index, moves, checker, (chess_Coordinates){-1, BLACK_PAWN_DIRECTION}, true);
    error_code += _chess_compute_offset_move(board, board_index, moves, checker, (chess_Coordinates){1, BLACK_PAWN_DIRECTION}, true);

    // TODO: check for discovered check
    int new_board_index = board_index + BOARD_ROW_SIZE;
    chess_Move *move = chess_create_non_capturing_move(board, board_index, new_board_index);
    error_code += chess_append_move(moves, move);
    if (error_code)
        return error_code;

    if (board_index >= BOARD_BLACK_MAX_PAWN_STARTING_INDEX)
        return 0;

    new_board_index += BOARD_ROW_SIZE;
    move = chess_create_non_capturing_move(board, board_index, new_board_index);
    return chess_append_move(moves, move);
}

chess_error_code _chess_compute_en_passant_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker, const int direction)
{
    // only one en passant move makes sense per turn, so saving last move destination to check for changes
    int final_destination = chess_get_final_move_destination(moves, -1);

    // setting must_capture to false because the target square does not contain an enemy piece when capturing en passant
    chess_error_code error_code = _chess_compute_offset_move(board, board_index, moves, checker, (chess_Coordinates){1, direction}, false);
    if (final_destination != chess_get_final_move_destination(moves, -1))
        return error_code;

    error_code += _chess_compute_offset_move(board, board_index, moves, checker, (chess_Coordinates){-1, direction}, false);
    return error_code;
}

chess_error_code chess_compute_black_en_passant_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker)
{
    return _chess_compute_en_passant_moves(board, board_index, moves, checker, BLACK_PAWN_DIRECTION);
}

chess_error_code chess_compute_white_pawn_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker)
{
    chess_error_code error_code = _chess_compute_offset_move(board, board_index, moves, checker, (chess_Coordinates){-1, WHITE_PAWN_DIRECTION}, true);
    error_code += _chess_compute_offset_move(board, board_index, moves, checker, (chess_Coordinates){1, WHITE_PAWN_DIRECTION}, true);

    // TODO: check for discovered check
    int new_board_index = board_index - BOARD_ROW_SIZE;
    chess_Move *move = chess_create_non_capturing_move(board, board_index, new_board_index);
    error_code += chess_append_move(moves, move);
    if (error_code)
        return error_code;

    if (board_index < BOARD_WHITE_MIN_PAWN_STARTING_INDEX)
        return 0;

    new_board_index -= BOARD_ROW_SIZE;
    move = chess_create_non_capturing_move(board, board_index, new_board_index);
    return chess_append_move(moves, move);
}

chess_error_code chess_compute_white_en_passant_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker)
{
    return _chess_compute_en_passant_moves(board, board_index, moves, checker, WHITE_PAWN_DIRECTION);
}

chess_error_code chess_compute_knight_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker)
{
    return chess_compute_offset_array_moves(board, board_index, moves, checker, KNIGHT_MOVE_INDEX_OFFSETS);
}

chess_error_code chess_compute_backward_line_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker, const int offset)
{
    int current_board_index = board_index;
    while (current_board_index >= offset)
    {
        current_board_index -= offset;
        int piece = board[current_board_index];
        bool is_team = (*(checker))(piece);
        if (is_team)
            break;

        chess_Move *move = chess_create_move(board_index, current_board_index);
        chess_error_code error_code = chess_append_move(moves, move);
        if (error_code)
            return error_code;

        if (piece != EMPTY)
            break;
    }
}

chess_error_code chess_compute_forward_line_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker, const int offset)
{
    int current_board_index = board_index;
    while (current_board_index <= BOARD_SIZE - offset)
    {
        current_board_index += offset;
        int piece = board[current_board_index];
        bool is_team = (*(checker))(piece);
        if (is_team)
            break;

        chess_Move *move = chess_create_move(board_index, current_board_index);
        chess_error_code error_code = chess_append_move(moves, move);
        if (error_code)
            return error_code;

        if (piece != EMPTY)
            break;
    }
}

chess_error_code chess_compute_arbitrary_offset_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker, const chess_Coordinates offset)
{
    chess_Coordinates current = chess_convert_to_board_coordinates(board_index);
    int x = current.x;
    int y = current.y;
    while (true)
    {
        x += offset.x;
        y += offset.y;
        if (chess_check_board_coordinates_out_of_range(x, y))
            break;

        int new_board_index = chess_convert_from_board_coordinates(x, y);
        int piece = board[new_board_index];
        bool is_team = (*(checker))(piece);
        if (is_team)
            break;

        chess_Move *move = chess_create_move(board_index, new_board_index);
        chess_error_code error_code = chess_append_move(moves, move);
        if (error_code)
            return error_code;

        if (piece != EMPTY)
            break;
    }
    return 0;
}

//
chess_error_code chess_compute_straight_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker)
{
    // TODO: check for discovered check
    int error_code = chess_compute_forward_line_moves(board, board_index, moves, checker, BOARD_ROW_SIZE);
    error_code += chess_compute_backward_line_moves(board, board_index, moves, checker, BOARD_ROW_SIZE);
    error_code += chess_compute_arbitrary_offset_moves(board, board_index, moves, checker, (chess_Coordinates){-1, 0});
    error_code += chess_compute_arbitrary_offset_moves(board, board_index, moves, checker, (chess_Coordinates){1, 0});
    return error_code;
}

//
chess_error_code chess_compute_diagonal_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker)
{
    // TODO: check for discovered check
    int error_code;
    for (int i = 0; i < DIAGONAL_MOVES; i++)
    {
        error_code += chess_compute_arbitrary_offset_moves(board, board_index, moves, checker, DIAGONAL_MOVE_INDEX_OFFSETS[i]);
    }
    return error_code;
}

//
chess_error_code chess_compute_king_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker)
{
    // TODO: check for check
    return chess_compute_offset_array_moves(board, board_index, moves, checker, KING_MOVE_INDEX_OFFSETS);
}

bool _chess_check_castle_side(const int board[BOARD_SIZE], const int start, const int end)
{
    for (int i = start; i < end; i++)
    {
        if (board[i] != EMPTY)
            return false;
    }
    return true;
}

chess_error_code chess_compute_black_king_castle_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker)
{
    // TODO: check denied castling

    // queen side
    if (BLACK_ROOK_CASTLE == board[0] && _chess_check_castle_side(board, 1, board_index))
    {
        chess_Move *move = chess_create_move(board_index, board_index - CASTLING_OFFSET);
        chess_error_code error_code = chess_append_move(moves, move);
        if (error_code)
            return error_code;
    }

    // king side
    if (BLACK_ROOK_CASTLE == board[BOARD_ROW_SIZE] && _chess_check_castle_side(board, board_index + 1, BOARD_ROW_SIZE))
    {
        chess_Move *move = chess_create_move(board_index, board_index + CASTLING_OFFSET);
        chess_error_code error_code = chess_append_move(moves, move);
        if (error_code)
            return error_code;
    }
    return 0;
}

chess_error_code chess_compute_white_king_castle_moves(const int board[BOARD_SIZE], const int board_index, chess_MovesLinkedList *moves, chess_TeamChecker checker)
{
    // TODO: check denied castling

    // queen side
    const int queen_side_rook_index = BOARD_SIZE - BOARD_ROW_SIZE;
    if (BLACK_ROOK_CASTLE == board[queen_side_rook_index] && _chess_check_castle_side(board, queen_side_rook_index + 1, board_index))
    {
        chess_Move *move = chess_create_move(board_index, board_index - CASTLING_OFFSET);
        chess_error_code error_code = chess_append_move(moves, move);
        if (error_code)
            return error_code;
    }

    // king side
    if (BLACK_ROOK_CASTLE == board[BOARD_SIZE] && _chess_check_castle_side(board, board_index + 1, BOARD_SIZE))
    {
        chess_Move *move = chess_create_move(board_index, board_index + CASTLING_OFFSET);
        chess_error_code error_code = chess_append_move(moves, move);
        if (error_code)
            return error_code;
    }
    return 0;
}

//
chess_error_code chess_compute_black_moves(const int board[BOARD_SIZE], chess_MovesLinkedList *moves)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        int piece = board[i];
        if (chess_is_white_or_empty(piece))
            continue;

        switch (piece)
        {
        case BLACK_PAWN:
            chess_compute_black_pawn_moves(board, i, moves, chess_is_black);
            break;
        case BLACK_ROOK:
        case BLACK_ROOK_CASTLE:
            chess_compute_straight_moves(board, i, moves, chess_is_black);
            break;
        case BLACK_KNIGHT:
            chess_compute_knight_moves(board, i, moves, chess_is_black);
            break;
        case BLACK_BISHOP:
            chess_compute_diagonal_moves(board, i, moves, chess_is_black);
            break;
        case BLACK_QUEEN:
            chess_compute_diagonal_moves(board, i, moves, chess_is_black);
            chess_compute_straight_moves(board, i, moves, chess_is_black);
            break;
        case BLACK_KING_CASTLE:
            chess_compute_black_king_castle_moves(board, i, moves, chess_is_black);
        case BLACK_KING:
            chess_compute_king_moves(board, i, moves, chess_is_black);
            break;
        case BLACK_PAWN_EN_PASSANT:
            chess_compute_black_en_passant_moves(board, i, moves, chess_is_black);
            chess_compute_black_pawn_moves(board, i, moves, chess_is_black);
            break;
        default:
            break;
        }
    }
    return 0;
}

//
chess_error_code chess_compute_white_moves(const int board[BOARD_SIZE], chess_MovesLinkedList *moves)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        int piece = board[i];
        if (chess_is_black(piece))
            continue;

        switch (piece)
        {
        case WHITE_PAWN:
            chess_compute_white_pawn_moves(board, i, moves, chess_is_white);
            break;
        case WHITE_ROOK:
        case WHITE_ROOK_CASTLE:
            chess_compute_straight_moves(board, i, moves, chess_is_white);
            break;
        case WHITE_KNIGHT:
            chess_compute_knight_moves(board, i, moves, chess_is_white);
            break;
        case WHITE_BISHOP:
            chess_compute_diagonal_moves(board, i, moves, chess_is_white);
            break;
        case WHITE_QUEEN:
            chess_compute_diagonal_moves(board, i, moves, chess_is_white);
            chess_compute_straight_moves(board, i, moves, chess_is_white);
            break;
        case WHITE_KING_CASTLE:
            chess_compute_white_king_castle_moves(board, i, moves, chess_is_white);
        case WHITE_KING:
            chess_compute_king_moves(board, i, moves, chess_is_white);
            break;
        case WHITE_PAWN_EN_PASSANT:
            chess_compute_white_en_passant_moves(board, i, moves, chess_is_white);
            chess_compute_white_pawn_moves(board, i, moves, chess_is_white);
            break;
        default:
            break;
        }
    }
    return 0;
}

//
void log_moves(chess_MovesLinkedList *moves)
{
    if (NULL == moves)
        return;

    chess_MovesLinkedListNode *node = moves->head;
    while (node != NULL)
    {
        chess_Move *move = node->move;
        printf("Move from %i to %i\n", move->origin, move->destination);
        node = node->next_node;
    }
}

int main()
{
    int board[BOARD_SIZE] = {0};
    chess_init_board(board, "");
    chess_print_board(board);
    printf("\n");

    time_t initial_time = time(NULL);
    int total_moves = 0;
    bool is_white_turn = true;
    for (int i = 0; i < 100; i++)
    {
        chess_MovesLinkedList *moves = &((chess_MovesLinkedList){NULL});
        if (is_white_turn)
        {
            chess_compute_white_moves(board, moves);
        }
        else
        {
            chess_compute_black_moves(board, moves);
        }
        int chosen_move_index = chess_generate_random_number(0, moves->length);
        printf("%i: Chose move %i\n", i, chosen_move_index);
        chess_Move *move = chess_get_move(moves, chosen_move_index);
        if (NULL == move)
            return 1;

        chess_update_board_with_move(board, move);
        total_moves += moves->length;
        chess_free_moves(moves);
        is_white_turn = !is_white_turn;
        chess_print_board(board);
        printf("\n");
    }
    // log_moves(moves);
    printf("Finished successfully, computing a total of %i moves in %d seconds", total_moves, time(NULL) - initial_time);
    return 0;
}
