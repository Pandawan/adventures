using System;
using System.IO;
using System.Collections.Generic;

namespace OriLanguage
{
    class Ori
    {
        private static bool hadError = false;

        public static int Main(string[] args)
        {
            if (args.Length > 1)
            {
                Console.WriteLine("Usage ori-cs [script]");
                return 64;
            }
            else if (args.Length == 1)
            {
                return RunFile(args[0]);
            }
            else
            {
                return RunPrompt();
            }
        }

        private static int RunFile(string path)
        {
            if (!File.Exists(path))
            {
                Console.Error.WriteLine($"Could not find file at path {path}");
            }

            string text = File.ReadAllText(path);
            Run(text);

            // If error, return with error code
            if (hadError) return 65;

            return 0;
        }

        private static int RunPrompt()
        {
            while (true)
            {
                Console.Write("> ");
                Run(Console.ReadLine());

                // Errors shouldn't kill entire process in REPL
                hadError = false;
            }
        }

        private static void Run(string source)
        {
            Scanner scanner = new Scanner(source);
            List<Token> tokens = scanner.ScanTokens();

            // For now just print the tokens
            foreach (Token token in tokens)
            {
                Console.WriteLine(token);
            }
        }

        public static void Error(int line, string message)
        {
            Report(line, "", message);
        }

        // TODO: Better error reporting with line & column, perhaps even showing the offending line
        private static void Report(int line, string where, string message)
        {
            Console.Error.WriteLine($"[line {line}] Error{where}: {message}");
            hadError = true;
        }
    }
}
