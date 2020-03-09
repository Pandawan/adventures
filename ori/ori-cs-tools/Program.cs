using System;
using System.Collections.Generic;
using System.Linq;

namespace OriTools
{
    class Program
    {
        static int Main(string[] args)
        {
            // Arguments to pass to the tool
            string[] toolArgs = args.Skip(1).ToArray();

            if (args.Length == 0 || string.IsNullOrEmpty(args[0]))
                Console.WriteLine("Usage: tools <tool_name> [...args]");
            else if (args[0] == "gen_ast")
                GenerateAst.Start(toolArgs);
            else
                Console.WriteLine($"Could not find a tool with name {args[0]}");

            return 0;
        }
    }
}
