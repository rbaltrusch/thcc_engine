#define EMPTY 0
#define WHITE_PAWN 1
#define WHITE_PAWN_EN_PASSANT 2
#define WHITE_ROOK 3
#define WHITE_ROOK_CASTLE 4
#define WHITE_KNIGHT 5
#define WHITE_BISHOP 6
#define WHITE_QUEEN 7
#define WHITE_KING 8
#define WHITE_KING_CASTLE 9
#define BLACK_PAWN 10
#define BLACK_PAWN_EN_PASSANT 11
#define BLACK_ROOK 12
#define BLACK_ROOK_CASTLE 13
#define BLACK_KNIGHT 14
#define BLACK_BISHOP 15
#define BLACK_QUEEN 16
#define BLACK_KING 17
#define BLACK_KING_CASTLE 18

#define BOARD_ROW_SIZE 8
#define BOARD_SIZE 64
#define BOARD_BLACK_MAX_PAWN_STARTING_INDEX 16
#define BOARD_WHITE_MIN_PAWN_STARTING_INDEX 48

#define EN_PASSANT_UPGRADE_INCREMENT 1
#define BLACK_PAWN_DIRECTION 1
#define WHITE_PAWN_DIRECTION -1
#define CASTLING_OFFSET 2

#define INDEX_OFFSETS 8
#define DIAGONAL_MOVES 4

#define BITS_PER_PIECE 5

#define FEN_MAX_STRING_LENGTH 90

#define DEBUG 3
#define INFO 2
#define WARNING 1
#define ERROR 0
#define LOG_LEVEL 3

typedef int chess_error_code;
typedef chess_error_code (*chess_BoardUpdater)(int board[BOARD_SIZE]);
typedef bool (*chess_TeamChecker)(const int piece);
typedef double (*chess_EvaluationFunction)(const int board[BOARD_SIZE]);

typedef enum
{
    chess_WHITE,
    chess_BLACK
} chess_Team;

typedef struct
{
    int x;
    int y;
} chess_Coordinates;

typedef struct
{
    int origin;
    int destination;
} chess_Move;

typedef struct chess_MovesLinkedListNode_
{
    chess_Move *move;
    struct chess_MovesLinkedListNode_ *next_node;
} chess_MovesLinkedListNode;

typedef struct
{
    chess_MovesLinkedListNode *head;
    chess_MovesLinkedListNode *tail;
    int length;
} chess_MovesLinkedList;

const chess_Coordinates KNIGHT_MOVE_INDEX_OFFSETS[INDEX_OFFSETS] = {{-2, -1}, {-2, 1}, {2, -1}, {2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}};
const chess_Coordinates DIAGONAL_MOVE_INDEX_OFFSETS[DIAGONAL_MOVES] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
const chess_Coordinates KING_MOVE_INDEX_OFFSETS[INDEX_OFFSETS] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

int chess_compute_zobrist_hash(int board[BOARD_SIZE]);
int chess_update_zobrist_hash(int board[BOARD_SIZE], int index);
