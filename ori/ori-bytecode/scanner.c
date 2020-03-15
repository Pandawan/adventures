#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct
{
    // Pointer to the begining of the current lexeme being scanned
    const char* start;
    // Pointer to the current character being looked at
    const char* current;
    int line;
} Scanner;

// Like for VM, keep a global instance so we don't have to pass it around
// TODO: To allow for embedding, make everything pass around scanner
Scanner scanner;

void initScanner(const char* source)
{
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool isAtEnd()
{
    // Null terminated string
    return *scanner.current == '\0';
}

static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c == '_');
}

// Advance scanner to next character
static char advance()
{
    scanner.current++;
    return scanner.current[-1];
}

// Returns the current character without consuming it
static char peek()
{
    return *scanner.current;
}

// Returns the next character without consuming it
static char peekNext()
{
    if (isAtEnd())
        return '\0';
    return scanner.current[1];
}

// Attempts to match the given character, consuming it if found
// Returns true if it correctly matched and consumed the character
static bool match(char expected)
{
    if (isAtEnd())
        return false;
    if (*scanner.current != expected)
        return false;

    scanner.current++;
    return true;
}

static Token makeToken(TokenType type)
{
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
}

static Token errorToken(const char* message)
{
    Token token;
    token.type = TOKEN_ERROR;
    // Point lexeme to error message rather than source code
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;

    return token;
}

// Skip all whitespaces and comments until it encounters a "meaningful" character
static void skipWhitespaceAndComments()
{
    for (;;)
    {
        char c = peek();
        switch (c)
        {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;

            // Bump line number if newline
            case '\n':
                scanner.line++;
                advance();
                break;

            case '/':
            {
                if (peekNext() == '/')
                {
                    // Comment goes until the end of the line
                    while (peek() != '\n' && !isAtEnd())
                        advance();
                }
                // Don't want to consume the / if the next character wasn't another /
                else
                {
                    return;
                }
                break;
            }
                // TODO: /* comments */

            default:
                return;
        }
    }
}

// Check if the next few characters match the given "rest" string
// Returns the given token type if true, otherwise returns identifier
static TokenType checkKeyword(int start, int length, const char* rest, TokenType type)
{
    if (scanner.current - scanner.start == start + length && memcmp(scanner.start + start, rest, length) == 0)
    {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

// Scan the next characters and determines the type of the token (identifier, keyword, etc.)
static TokenType getIdentifierType()
{
    // Finite state machine to try and match the keywords
    switch (scanner.start[0])
    {
        case 'a':
            return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'c':
            return checkKeyword(1, 4, "lass", TOKEN_CLASS);
        case 'e':
            return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
        {
            // Check that there is a second letter after f
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                    case 'a':
                        return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o':
                        return checkKeyword(2, 1, "r", TOKEN_FOR);
                    case 'u':
                        return checkKeyword(2, 6, "nction", TOKEN_FUNCTION);
                }
            }
            break;
        }
        case 'i':
            return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'l':
            return checkKeyword(1, 2, "et", TOKEN_LET);
        case 'n':
            return checkKeyword(1, 2, "ull", TOKEN_NULL);
        case 'o':
            return checkKeyword(1, 1, "r", TOKEN_OR);
        case 'p':
            return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r':
            return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's':
            return checkKeyword(1, 4, "uper", TOKEN_SUPER);
        case 't':
        {
            // Check that there is a second letter after t
            if (scanner.current - scanner.start > 1)
            {
                switch (scanner.start[1])
                {
                    case 'h':
                        return checkKeyword(2, 2, "is", TOKEN_THIS);
                    case 'r':
                        return checkKeyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        }

        case 'w':
            return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}

static Token readIdentifier()
{
    // Identifiers can be alphanumeric and _
    while (isAlpha(peek()) || isDigit(peek()))
        advance();

    return makeToken(getIdentifierType());
}

static Token readNumber()
{
    while (isDigit(peek()))
        advance();

    // Look for decimal part
    if (peek() == '.' && isDigit(peekNext()))
    {
        // Consume the .
        advance();

        while (isDigit(peek()))
            advance();
    }

    return makeToken(TOKEN_NUMBER);
}

static Token readString()
{
    // TODO: String interpolation with ${}

    // Keep advancing until it finds a " or reaches the end
    while (peek() != '"' && !isAtEnd())
    {
        // Supports multiline strings
        if (peek() == '\n')
            scanner.line++;
        advance();
    }

    if (isAtEnd())
        return errorToken("Unterminated string.");

    // Skip closing quote
    advance();

    // Not passing the actual string value here,
    // it can be found with start and length in source code
    // This is for simplicity, it could be done
    // but would require dynamic typing of stored value in Token struct
    return makeToken(TOKEN_STRING);
}

Token scanToken()
{
    // Since no loop to ignore whitespace & comments, need to take care of it early
    skipWhitespaceAndComments();

    scanner.start = scanner.current;

    if (isAtEnd())
        return makeToken(TOKEN_EOF);

    char c = advance();

    // Read a keyword or identifier
    if (isAlpha(c))
        return readIdentifier();
    if (isDigit(c))
        return readNumber();

    switch (c)
    {
        case '(':
            return makeToken(TOKEN_LEFT_PAREN);
        case ')':
            return makeToken(TOKEN_RIGHT_PAREN);
        case '{':
            return makeToken(TOKEN_LEFT_BRACE);
        case '}':
            return makeToken(TOKEN_RIGHT_BRACE);
        case ';':
            return makeToken(TOKEN_SEMICOLON);
        case ',':
            return makeToken(TOKEN_COMMA);
        case '.':
            return makeToken(TOKEN_DOT);
        case '-':
            return makeToken(TOKEN_MINUS);
        case '+':
            return makeToken(TOKEN_PLUS);
        case '/':
            return makeToken(TOKEN_SLASH);
        case '*':
            return makeToken(TOKEN_STAR);

        case '!':
            return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

            // TODO: BITWISE &, |, etc.
        case '&':
        {
            if (match('&'))
                return makeToken(TOKEN_AND);
        }
        case '|':
        {
            if (match('|'))
                return makeToken(TOKEN_OR);
        }

        case '"':
            return readString();
    }

    return errorToken("Unexpected character.");
}