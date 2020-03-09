using System.Collections.Generic;

namespace OriLanguage
{
    public interface IExpression
    {
        T Accept<T>(IExpressionVisitor<T> visitor);
    }

    public interface IExpressionVisitor<T>
    {
        T VisitBinaryExpression(BinaryExpression expression);
        T VisitGroupingExpression(GroupingExpression expression);
        T VisitLiteralExpression(LiteralExpression expression);
        T VisitUnaryExpression(UnaryExpression expression);
    }

    public struct BinaryExpression : IExpression
    {
        public readonly IExpression left;
        public readonly Token op;
        public readonly IExpression right;

        public BinaryExpression(IExpression left, Token op, IExpression right)
        {
            this.left = left;
            this.op = op;
            this.right = right;
        }

        public T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitBinaryExpression(this);
        }
    }

    public struct GroupingExpression : IExpression
    {
        public readonly IExpression expression;

        public GroupingExpression(IExpression expression)
        {
            this.expression = expression;
        }

        public T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitGroupingExpression(this);
        }
    }

    public struct LiteralExpression : IExpression
    {
        public readonly object value;

        public LiteralExpression(object value)
        {
            this.value = value;
        }

        public T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitLiteralExpression(this);
        }
    }

    public struct UnaryExpression : IExpression
    {
        public readonly Token op;
        public readonly IExpression right;

        public UnaryExpression(Token op, IExpression right)
        {
            this.op = op;
            this.right = right;
        }

        public T Accept<T>(IExpressionVisitor<T> visitor)
        {
            return visitor.VisitUnaryExpression(this);
        }
    }
}
