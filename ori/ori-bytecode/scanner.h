#ifndef ori_scanner_h
#define ori_scanner_h

typedef enum
{
    // Single-character tokens
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_STAR,

    // One or two character tokens
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,

    // Literals
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,

    // Keywords
    TOKEN_AND,
    TOKEN_CLASS,
    TOKEN_ELSE,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_FUNCTION,
    TOKEN_IF,
    TOKEN_LET,
    TOKEN_NULL,
    TOKEN_OR,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_SUPER,
    TOKEN_THIS,
    TOKEN_TRUE,
    TOKEN_WHILE,

    // Special token so compiler knows there was an error when scanning
    TOKEN_ERROR,

    TOKEN_EOF
} TokenType;

typedef struct
{
    TokenType type;
    // Pointer to the start of the lexeme in the source code
    const char* start;
    // Length of the lexeme in the source code
    int length;
    // Line at which the token occurs
    int line;
} Token;

void initScanner(const char* source);
// Scans one token
Token scanToken();

#endif