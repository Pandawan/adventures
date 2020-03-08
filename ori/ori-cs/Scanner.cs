using System;
using System.Collections.Generic;

namespace OriLanguage
{
    class Scanner
    {
        private static readonly Dictionary<string, TokenType> keywords = new Dictionary<string, TokenType>(){
            {"and",    TokenType.And },
            {"class",  TokenType.Class },
            {"else",   TokenType.Else },
            {"false",  TokenType.False },
            {"for",    TokenType.For },
            {"fun",    TokenType.Fun },
            {"if",     TokenType.If },
            {"nil",    TokenType.Null },
            {"or",     TokenType.Or },
            {"print",  TokenType.Print },
            {"return", TokenType.Return },
            {"super",  TokenType.super },
            {"this",   TokenType.This },
            {"true",   TokenType.True },
            {"var",    TokenType.Var },
            {"while",  TokenType.While },
        };

        private readonly string m_Source;
        private readonly List<Token> m_Tokens;
        /// <summary>
        /// The first character in the current lexeme being scanned.
        /// </summary>
        private int m_Start;
        /// <summary>
        /// The character currently being scanned/considered.
        /// </summary>
        private int m_Current;
        private int m_Line;

        public Scanner(string source)
        {
            this.m_Source = source;
            this.m_Tokens = new List<Token>();
            this.m_Start = 0;
            this.m_Current = 0;
            this.m_Line = 1;
        }

        public List<Token> ScanTokens()
        {
            while (!IsAtEnd())
            {
                // We are at the beginning of the next lexeme.
                m_Start = m_Current;
                ScanToken();
            }

            m_Tokens.Add(new Token(TokenType.EOF, "", null, m_Line));
            return m_Tokens;
        }

        private void ScanToken()
        {
            char c = Advance();
            switch (c)
            {
                // Braces
                case '(': AddToken(TokenType.LeftParen); break;
                case ')': AddToken(TokenType.RightParen); break;
                case '{': AddToken(TokenType.LeftBrace); break;
                case '}': AddToken(TokenType.RightBrace); break;
                case '[': AddToken(TokenType.LeftBrace); break;
                case ']': AddToken(TokenType.RightBrace); break;

                // Syntax
                case ',': AddToken(TokenType.Comma); break;
                case '.': AddToken(TokenType.Dot); break;
                case ';': AddToken(TokenType.Semicolon); break;

                // Math
                case '-': AddToken(TokenType.Minus); break;
                case '+': AddToken(TokenType.Plus); break;
                case '*': AddToken(TokenType.Star); break;
                case '%': AddToken(TokenType.Modulus); break;

                // Boolean
                case '!': AddToken(Match('=') ? TokenType.BangEqual : TokenType.Bang); break;
                case '=': AddToken(Match('=') ? TokenType.EqualEqual : TokenType.Equal); break;
                case '<': AddToken(Match('=') ? TokenType.LessEqual : TokenType.Less); break;
                case '>': AddToken(Match('=') ? TokenType.GreaterEqual : TokenType.Greater); break;
                // TODO: Bitwise would be implemented the same as above with AND &&, and OR, etc.
                // This would require removing the && and || in the default

                case '&':
                    {
                        if (Match('&'))
                        {
                            AddToken(TokenType.And);
                        }
                        else
                        {
                            Ori.Error(m_Line, "Bitwise operators are not yet implemented.");
                        }
                        break;
                    }

                case '|':
                    {
                        if (Match('|'))
                        {
                            AddToken(TokenType.Or);
                        }
                        else
                        {
                            Ori.Error(m_Line, "Bitwise operators are not yet implemented.");
                        }
                        break;
                    }


                case '"': ReadString(); break;

                case '/':
                    {
                        // A comment goes until the end of a line
                        if (Match('/'))
                        {
                            while (Peek() != '\n' && !IsAtEnd()) Advance();
                        }
                        else if (Match('*'))
                        {
                            // Keep on reading comment until it reaches */
                            while (!(Advance() == '*' && Match('/'))) ;
                        }
                        else
                        {
                            AddToken(TokenType.Slash);
                        }
                        break;
                    }

                case ' ':
                case '\r':
                case '\t':
                    // Ignore whitespace
                    break;

                case '\n':
                    m_Line++;
                    break;

                default:
                    {
                        if (IsDigit(c))
                        {
                            ReadNumber();
                        }
                        else if (IsAlpha(c))
                        {
                            ReadIdentifier();
                        }
                        else
                        {
                            Ori.Error(m_Line, "Unexpected character.");
                        }
                        break;
                    }
            }
        }

        private void ReadIdentifier()
        {
            while (IsAlphaNumeric(Peek())) Advance();

            // See if the identifier is a reserved word
            string text = Substring(m_Start, m_Current);


            TokenType type = keywords.ContainsKey(text)
                ? keywords[text]
                : TokenType.Identifier;

            AddToken(type);
        }

        private void ReadNumber()
        {
            while (IsDigit(Peek())) Advance();

            // Look for a decimal part.
            if (Peek() == '.' && IsDigit(PeekNext()))
            {
                // Consume the "."
                Advance();

                // Consume every digit after it
                while (IsDigit(Peek())) Advance();
            }

            AddToken(TokenType.Number, Double.Parse(Substring(m_Start, m_Current)));
        }

        private void ReadString()
        {
            while (Peek() != '"' && !IsAtEnd())
            {
                // Supports multiline strings
                if (Peek() == '\n') m_Line++;
                Advance();
            }

            // Unterminated string
            if (IsAtEnd())
            {
                Ori.Error(m_Line, "Unterminated string.");
                return;
            }

            // The closing "
            Advance();

            // Trim the surrounding quotes
            string value = Substring(m_Start + 1, m_Current - 1);
            // TODO: If want to support escape sequences, do it here
            AddToken(TokenType.String, value);
        }

        /// <summary>
        /// Consumes the current character if it matches the expected one given.
        /// </summary>
        /// <param name="expected">The character to consume.</param>
        /// <returns>True if it successfully consumed the character.</returns>
        private bool Match(char expected)
        {
            if (IsAtEnd()) return false;
            if (m_Source[m_Current] != expected) return false;

            m_Current++;
            return true;
        }

        /// <summary>
        /// Get the current character without consuming it.
        /// </summary>
        private char Peek()
        {
            if (IsAtEnd()) return '\0';
            return m_Source[m_Current];
        }

        /// <summary>
        /// Get the next character without consuming it.
        /// </summary>
        private char PeekNext()
        {
            if (m_Current + 1 >= m_Source.Length) return '\0';
            return m_Source[m_Current + 1];
        }

        /// <summary>
        /// Get the current character and consume it.
        /// </summary>
        private char Advance()
        {
            m_Current++;
            return m_Source[m_Current - 1];
        }

        private bool IsAlpha(char c)
        {
            return (c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                        c == '_';
        }

        private bool IsAlphaNumeric(char c)
        {
            return IsAlpha(c) || IsDigit(c);
        }

        private bool IsDigit(char c)
        {
            return c >= '0' && c <= '9';
        }

        private bool IsAtEnd()
        {
            return m_Current >= m_Source.Length;
        }

        private void AddToken(TokenType type)
        {
            AddToken(type, null);
        }

        private void AddToken(TokenType type, object literal)
        {
            string text = Substring(m_Start, m_Current);
            m_Tokens.Add(new Token(type, text, literal, m_Line));
        }

        /// <summary>
        /// Creates a substring of our source from a start
        /// and ending position.
        /// </summary>
        private string Substring(int start, int end)
        {
            int length = end - start;
            return m_Source.Substring(start, length);
        }
    }
}