#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct
{
    Token current;
    Token previous;
    // Whether or not the parser has encountered an error
    bool hadError;
    // Whether or not the parser is in panic mode and should skip tokens and resynchronize
    bool panicMode;
} Parser;

typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR,         // or ||
    PREC_AND,        // and &&
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();

typedef struct
{
    // Function to compile a prefix expression starting with a token of that type
    ParseFn prefix;
    // Function to compile an infix expression whose left operand is followed by a token of that type
    ParseFn infix;
    Precedence precedence;
} ParseRule;

// Like VM, keep on single instance
// TODO: Might want to pass it around to support embedding
Parser parser;

Chunk* compilingChunk;

static Chunk* getCurrentChunk()
{
    return compilingChunk;
}

static void errorAt(Token* token, const char* message)
{
    // Ignore any error if it's already panicking
    if (parser.panicMode)
        return;
    // Set panic mode
    parser.panicMode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR)
    {
        // Nothing
    }
    else
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

// Report an error on the previous token
static void error(const char* message)
{
    errorAt(&parser.previous, message);
}

// Report an error at the curren token
static void errorAtCurrent(const char* message)
{
    errorAt(&parser.current, message);
}

// Reads the next token
static void advance()
{
    parser.previous = parser.current;

    for (;;)
    {
        parser.current = scanToken();
        // If get an error, want to keep scanning until it reaches valid code OR EOF
        if (parser.current.type != TOKEN_ERROR)
            break;

        errorAtCurrent(parser.current.start);
    }
}

// Reads the next token validating that the token has the given expected type
static void consume(TokenType type, const char* message)
{
    if (parser.current.type == type)
    {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static void emitByte(uint8_t byte)
{
    writeChunk(getCurrentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn()
{
    emitByte(OP_RETURN);
}

// Create a constant from the given value and add it to the current chunk
static uint8_t makeConstant(Value value)
{
    int constant = addConstant(getCurrentChunk(), value);
    // TODO: This only allows for 256 different constants in one chunk.
    // Add an OP_CONSTANT_16 which allows for two-byte index operand
    if (constant > UINT8_MAX)
    {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

static void emitConstant(Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler()
{
    // For now, return is used to end expressions and print their values
    emitReturn();

#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
    {
        disassembleChunk(getCurrentChunk(), "code");
    }
#endif
}

static void compileExpression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

// TODO: Add support for ternary operator ? : This would probably require modifying parsePrecedence & parseRules

static void compileBinary()
{
    // At this point, the left operand has already been consumed

    // Remember the operator
    TokenType operatorType = parser.previous.type;

    // Compile the right operand
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    // Emit the operator instruction
    switch (operatorType)
    {
        case TOKEN_BANG_EQUAL:
            emitBytes(OP_EQUAL, OP_NOT);
            break;
        case TOKEN_EQUAL_EQUAL:
            emitByte(OP_EQUAL);
            break;
        case TOKEN_GREATER:
            emitByte(OP_GREATER);
            break;
        case TOKEN_GREATER_EQUAL:
            emitBytes(OP_LESS, OP_NOT); // (a >= b) == !(a < b)
            break;
        case TOKEN_LESS:
            emitByte(OP_LESS);
            break;
        case TOKEN_LESS_EQUAL:
            emitBytes(OP_GREATER, OP_NOT); // (a <= b) == !(a > b)
            break;
        case TOKEN_PLUS:
            emitByte(OP_ADD);
            break;
        case TOKEN_MINUS:
            emitByte(OP_SUBTRACT);
            break;
        case TOKEN_STAR:
            emitByte(OP_MULTIPLY);
            break;
        case TOKEN_SLASH:
            emitByte(OP_DIVIDE);
            break;
        default:
            return; // Unreachable
    }
}

static void compileLiteral()
{
    // Keyword token has already been consumed by parsePrecedence
    // All that's needed is to emit the correct instruction for the literal
    switch (parser.previous.type)
    {
        case TOKEN_FALSE:
            emitByte(OP_FALSE);
            break;
        case TOKEN_NULL:
            emitByte(OP_NULL);
            break;
        case TOKEN_TRUE:
            emitByte(OP_TRUE);
            break;
    }
}

static void compileGrouping()
{
    // Assume ( has already been consumed
    compileExpression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void compileNumber()
{
    // Assume the token for number literal has already been consumed and is in previous
    // Convert that string lexeme to a double
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void compileString()
{
    // Convert content of string into a constant value (removing the quotes)
    // TODO: Add support for escape sequences
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
                                    parser.previous.length - 2)));
}

static void compileUnary()
{
    TokenType operatorType = parser.previous.type;

    // Compile the operand
    // (use PREC_UNARY to allow for nested unary like !!doubleNegative)
    parsePrecedence(PREC_UNARY);

    // TODO: In multiline unary, the error shows up on the wrong line
    // print -
    //   true;
    // The error will show up on line 2, because OP_NEGATE is emitted AFTER the operand
    // This could be fixed by storing the token's line before compiling the operand, and pass that into emitByte

    // Emit the operator instruction
    switch (operatorType)
    {
        case TOKEN_BANG:
            emitByte(OP_NOT);
            break;
        case TOKEN_MINUS:
            emitByte(OP_NEGATE);
            break;
        default:
            return; // Unreachable
    }
}

// Rules powering the Pratt parser
ParseRule rules[] = {
    {compileGrouping, NULL, PREC_NONE},       // TOKEN_LEFT_PAREN
    {NULL, NULL, PREC_NONE},                  // TOKEN_RIGHT_PAREN
    {NULL, NULL, PREC_NONE},                  // TOKEN_LEFT_BRACE
    {NULL, NULL, PREC_NONE},                  // TOKEN_RIGHT_BRACE
    {NULL, NULL, PREC_NONE},                  // TOKEN_COMMA
    {NULL, NULL, PREC_NONE},                  // TOKEN_DOT
    {compileUnary, compileBinary, PREC_TERM}, // TOKEN_MINUS
    {NULL, compileBinary, PREC_TERM},         // TOKEN_PLUS
    {NULL, NULL, PREC_NONE},                  // TOKEN_SEMICOLON
    {NULL, compileBinary, PREC_FACTOR},       // TOKEN_SLASH
    {NULL, compileBinary, PREC_FACTOR},       // TOKEN_STAR
    {compileUnary, NULL, PREC_NONE},          // TOKEN_BANG
    {NULL, compileBinary, PREC_EQUALITY},     // TOKEN_BANG_EQUAL
    {NULL, NULL, PREC_NONE},                  // TOKEN_EQUAL
    {NULL, compileBinary, PREC_EQUALITY},     // TOKEN_EQUAL_EQUAL
    {NULL, compileBinary, PREC_COMPARISON},   // TOKEN_GREATER
    {NULL, compileBinary, PREC_COMPARISON},   // TOKEN_GREATER_EQUAL
    {NULL, compileBinary, PREC_COMPARISON},   // TOKEN_LESS
    {NULL, compileBinary, PREC_COMPARISON},   // TOKEN_LESS_EQUAL
    {NULL, NULL, PREC_NONE},                  // TOKEN_IDENTIFIER
    {compileString, NULL, PREC_NONE},         // TOKEN_STRING
    {compileNumber, NULL, PREC_NONE},         // TOKEN_NUMBER
    {NULL, NULL, PREC_NONE},                  // TOKEN_AND
    {NULL, NULL, PREC_NONE},                  // TOKEN_CLASS
    {NULL, NULL, PREC_NONE},                  // TOKEN_ELSE
    {compileLiteral, NULL, PREC_NONE},        // TOKEN_FALSE
    {NULL, NULL, PREC_NONE},                  // TOKEN_FOR
    {NULL, NULL, PREC_NONE},                  // TOKEN_FUNCTION
    {NULL, NULL, PREC_NONE},                  // TOKEN_IF
    {NULL, NULL, PREC_NONE},                  // TOKEN_LET
    {compileLiteral, NULL, PREC_NONE},        // TOKEN_NULL
    {NULL, NULL, PREC_NONE},                  // TOKEN_OR
    {NULL, NULL, PREC_NONE},                  // TOKEN_PRINT
    {NULL, NULL, PREC_NONE},                  // TOKEN_RETURN
    {NULL, NULL, PREC_NONE},                  // TOKEN_SUPER
    {NULL, NULL, PREC_NONE},                  // TOKEN_THIS
    {compileLiteral, NULL, PREC_NONE},        // TOKEN_TRUE
    {NULL, NULL, PREC_NONE},                  // TOKEN_WHILE
    {NULL, NULL, PREC_NONE},                  // TOKEN_ERROR
    {NULL, NULL, PREC_NONE},                  // TOKEN_EOF
};

// Starts at the current token and parses any expression at the given precedence level or higher
static void parsePrecedence(Precedence precedence)
{
    // Read the next token
    advance();

    // Handle prefix
    // Look up corresponding parse rule
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    // If no prefix parser is found, it must be a syntax error
    if (prefixRule == NULL)
    {
        error("Expect expression.");
        return;
    }
    // Call the parsing rule
    prefixRule();

    // Handle infix
    while (precedence <= getRule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }
}

static ParseRule* getRule(TokenType type)
{
    return &rules[type];
}

static void compileExpression()
{
    // Parese the lowest precedence level, allowing for ALL higher levels to also be parsed
    parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const char* source, Chunk* chunk)
{
    initScanner(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    advance();
    compileExpression();
    consume(TOKEN_EOF, "Expected end of expression.");
    endCompiler();

    return !parser.hadError;
}
