namespace OriLanguage
{
    public enum TokenType
    {
        // Single-character tokens.                      
        LeftParen, RightParen, LeftBrace, RightBrace, LeftBracket, RightBracket,
        Comma, Dot, Minus, Plus, Semicolon, Slash, Star, Modulus,

        // One or two character tokens.                  
        Bang, BangEqual,
        Equal, EqualEqual,
        Greater, GreaterEqual,
        Less, LessEqual,

        // Literals.                                     
        Identifier, String, Number,

        // Keywords.                                     
        And, Class, Else, False, Fun, For, If, Null, Or,
        Print, Return, super, This, True, Var, While, // TODO: Break ?

        EOF
    }
}