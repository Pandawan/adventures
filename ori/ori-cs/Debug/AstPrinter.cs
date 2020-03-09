using System;
using System.Text;

namespace OriLanguage.Debug {
    public class AstPrinter : IExpressionVisitor<string> {

        /*
        public static void Main(string[] args) {
            IExpression expression = new BinaryExpression(
                new UnaryExpression(
                    new Token(TokenType.Minus, "-", null, 1),
                    new LiteralExpression(123)
                ),
                new Token(TokenType.Star, "*", null, 1),
                new GroupingExpression(
                    new LiteralExpression(45.67)
                )
            );

            Console.WriteLine(new AstPrinter().Print(expression));
        }
        */

        string Print(IExpression expression) {
            return expression.Accept(this);
        }

        public string VisitBinaryExpression(BinaryExpression expression) {
            return Parenthesize(expression.op.lexeme, expression.left, expression.right);
        }

        public string VisitGroupingExpression(GroupingExpression expression) {
            return Parenthesize("group", expression.expression);
        }

        public string VisitLiteralExpression(LiteralExpression expression) {
            if (expression.value == null) return "null";
            return expression.value.ToString();
        }

        public string VisitUnaryExpression(UnaryExpression expression) {
            return Parenthesize(expression.op.lexeme, expression.right);
        }

        private string Parenthesize(string name, params IExpression[] expressions){
            StringBuilder builder = new StringBuilder();

            builder.Append("(").Append(name);
            foreach(IExpression expression in expressions) {
                builder
                    .Append(" ")
                    .Append(expression.Accept(this));
            }

            builder.Append(")");

            return builder.ToString();
        }
    }
}