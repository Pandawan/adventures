using System;
using System.IO;
using System.Collections.Generic;

namespace OriTools
{
    /// <summary>
    /// Simple script to generate an Expression.cs file with multiple IExpression structs.
    /// </summary>
    public class GenerateAst
    {
        // Simply call "dotnet run -p tools ./ori" from the root of the solution
        public static void Start(string[] args)
        {
            if (args.Length != 2)
            {
                Console.WriteLine("Usage: gen_ast <expression|statement> <output_directory>");
                Environment.Exit(64);
            }

            string outputDir = args[1];

            if (args[0] == "expression")
            {

                DefineAst(outputDir, "Expression", new List<string>() {
                    "BinaryExpression   : IExpression left, Token op, IExpression right",
                    // "CallExpression     : IExpression callee, Token paren, List<IExpression> arguments",
                    "GroupingExpression : IExpression expression",
                    "LiteralExpression  : object value",
                    // "LogicalExpression  : IExpression left, Token op, IExpression right",
                    "UnaryExpression    : Token op, IExpression right",
                    // Ternary doesn't need to know about the ? : tokens
                    // "TernaryExpression  : IExpression conditional, IExpression thenExpression, IExpression elseExpression",
                    // "VariableExpression : Token name",
                    // "AssignExpression   : Token name, IExpression value",
                });
                Console.WriteLine($"Successfully generated file Expressions.cs in {args[1]}");
            }
            /*
            else if (args[0] == "statement")
            {
                DefineAst(outputDir, "Statement", new List<string>() {
                    "ExpressionStatement : IExpression expression",
                    "FunctionStatement   : Token name, List<Token> parameters, List<IStatement> body",
                    "IfStatement         : IExpression condition, IStatement thenBranch, IStatement elseBranch",
                    "PrintStatement      : IExpression expression",
                    "ReturnStatement     : Token keyword, IExpression value",
                    "LetStatement        : Token name, IExpression initializer",
                    "WhileStatement      : IExpression condition, IStatement body",
                    "BlockStatement      : List<IStatement> statements",
                });
                Console.WriteLine($"Successfully generated file Statements.cs in {args[1]}");
            }
            */
            else
            {
                Console.WriteLine("Use 'expression' or 'statement' to specify which file to generate.");
                Environment.Exit(64);
            }
        }

        // Generate Expression file with namespace, interface class, and all Expressions
        private static void DefineAst(string outputDir, string baseName, List<string> types)
        {
            using (StreamWriter outputFile = new StreamWriter(Path.Combine(outputDir, $"{baseName}s.cs")))
            {
                outputFile.WriteLine("using System.Collections.Generic;\n");

                outputFile.WriteLine($"namespace OriLanguage");
                outputFile.WriteLine("{");

                outputFile.WriteLine($"public interface I{baseName}");
                outputFile.WriteLine("{");

                // Add a base accept method
                outputFile.WriteLine($"T Accept<T>(I{baseName}Visitor<T> visitor);");
                outputFile.WriteLine("}");
                outputFile.WriteLine();

                DefineVisitor(outputFile, baseName, types);

                foreach (string type in types)
                {
                    string className = type.Split(":")[0].Trim();
                    string fields = type.Split(":")[1].Trim();
                    DefineType(outputFile, baseName, className, fields);
                }

                outputFile.WriteLine("}");
            }
        }

        // Generate one Expression struct + fields + ctor
        private static void DefineType(StreamWriter outputFile, string baseName, string className, string fields)
        {
            outputFile.WriteLine();
            outputFile.WriteLine($"public struct {className} : I{baseName}");
            outputFile.WriteLine("{");

            string[] fieldList = fields.Split(", ");

            // Fields
            foreach (string field in fieldList)
            {
                outputFile.WriteLine($"public readonly {field};");
            }

            outputFile.WriteLine();

            // Constructor
            outputFile.WriteLine($"public {className} ({fields})");
            outputFile.WriteLine("{");

            foreach (string field in fieldList)
            {
                string name = field.Split(" ")[1];
                outputFile.WriteLine($"this.{name} = {name};");
            }

            outputFile.WriteLine("}");

            // Visitor pattern
            outputFile.WriteLine();
            outputFile.WriteLine($"public T Accept<T>(I{baseName}Visitor<T> visitor)");
            outputFile.WriteLine("{");
            outputFile.WriteLine($"return visitor.Visit{className}(this);");
            outputFile.WriteLine("}");

            outputFile.WriteLine("}");
        }

        // Generate visitor interface to use Expressions
        private static void DefineVisitor(StreamWriter outputFile, string baseName, List<string> types)
        {
            outputFile.WriteLine($"public interface I{baseName}Visitor<T>");
            outputFile.WriteLine("{");

            foreach (string type in types)
            {
                string typeName = type.Split(":")[0].Trim();
                outputFile.WriteLine($"T Visit{typeName} ({typeName} {baseName.ToLower()});");
            }

            outputFile.WriteLine("}");
        }
    }
}