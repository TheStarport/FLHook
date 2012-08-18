using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace PlayerCntlSetup
{
    // CSVReader by Andreas Knab
    // http://knab.ws/blog/index.php?/archives/10-CSV-file-parser-and-writer-in-C-Part-2.html
    // Each record is one line (with exceptions)
    // Fields are separated with commas
    // Leading and trailing space-characters adjacent to comma field separators are ignored
    // Fields with embedded commas must be delimited with double-quote characters
    // Fields that contain double quote characters must be surounded by double-quotes, and the embedded double-quotes must each be represented by a pair of consecutive double quotes.
    // A field that contains embedded line-breaks must be surounded by double-quotes
    // Fields with leading or trailing spaces must be delimited with double-quote characters
    // Fields may always be delimited with double quotes
    // The first record in a CSV file may be a header record containing column (field) names
    public class CsvStream
    {
        private TextReader stream;

        public CsvStream(TextReader s)
        {
            stream = s;
        }

        public List<string> GetNextRow()
        {
            List<string> row = new List<string>();
            while (true)
            {
                string item = GetNextItem();
                if (item == null)
                    return row.Count == 0 ? null : row;
                row.Add(item);
            }
        }

        private bool EOS = false;
        private bool EOL = false;

        private string GetNextItem()
        {
            if (EOL)
            {
                // previous item was last in line, start new line
                EOL = false;
                return null;
            }

            bool quoted = false;
            bool predata = true;
            bool postdata = false;
            StringBuilder item = new StringBuilder();

            while (true)
            {
                char c = GetNextChar(true);
                if (EOS)
                    return item.Length > 0 ? item.ToString() : null;

                if ((postdata || !quoted) && c == ',')
                    // end of item, return
                    return item.ToString();

                if ((predata || postdata || !quoted) && (c == '\x0A' || c == '\x0D'))
                {
                    // we are at the end of the line, eat newline characters and exit
                    EOL = true;
                    if (c == '\x0D' && GetNextChar(false) == '\x0A')
                        // new line sequence is 0D0A
                        GetNextChar(true);
                    return item.ToString();
                }

                if (predata && c == ' ')
                    // whitespace preceeding data, discard
                    continue;

                if (predata && c == '"')
                {
                    // quoted data is starting
                    quoted = true;
                    predata = false;
                    continue;
                }

                if (predata)
                {
                    // data is starting without quotes
                    predata = false;
                    item.Append(c);
                    continue;
                }

                if (c == '"' && quoted)
                {
                    if (GetNextChar(false) == '"')
                        // double quotes within quoted string means add a quote       
                        item.Append(GetNextChar(true));
                    else
                        // end-quote reached
                        postdata = true;
                    continue;
                }

                // all cases covered, character must be data
                item.Append(c);
            }
        }

        private char[] buffer = new char[4096];
        private int pos = 0;
        private int length = 0;

        private char GetNextChar(bool eat)
        {
            if (pos >= length)
            {
                length = stream.ReadBlock(buffer, 0, buffer.Length);
                if (length == 0)
                {
                    EOS = true;
                    return '\0';
                }
                pos = 0;
            }
            if (eat)
                return buffer[pos++];
            else
                return buffer[pos];
        }
    }
}
