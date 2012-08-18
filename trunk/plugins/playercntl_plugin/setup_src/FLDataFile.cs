using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

/*  The following notes on bin file encoding were written by Christopher Wellons <ccw129@psu.edu>
    and were taken from http://www.nullprogram.com/projects/bini/

    The bini decode implementaton in the following code is my own (and so is probably buggy!)
  
    A BINI file has two segments: a data segment and a string segment, which contains the string table.
    +------------+
    |    data    |
    |------------|
    |   string   |
    |    table   |
    +------------+
    The string table is an argz vector: a contiguous block of memory with strings separated by
     null-characters (/0). The data section points to these strings. All values are stored as little endian.

    A BINI file begins with a 12-byte header made up of 3 4-byte segments,

    header {
      dword "BINI"
      dword version
      dword str_table
    }

    The first 4 bytes identify the file as a BINI file with those exact 4 ASCII letters. The second
    4 bytes are always equal to the number 1. This is believed to be a version number. The last 4 bytes
    is the string table offset from the beginning of the file.

    The first 4 bytes after the header is the first section. A section contains two 2-byte values,

    section {
      word string_offset
      word number_of_entries
    }

    The string offset is the offset from the beginning of the string table. This string is the name of
    that section. The second word is the number of entries in this section. Note that the number of 
    sections is not listed anywhere, so this information can only be found by iterating though the
    entire data segment.

    Following this section information is a 3-byte entry,

    entry {
      word string_offset
      byte number of values
    }

    This is data is setup just like sections with the string offset being the name of the entry,
    and following it is the same number of values as indicated,

    value {
      byte  type
      dword data
    }

    A value is 5 bytes. The first byte describes the type, of which there are three,

         1 - integer
         2 - float
         3 - a string table offset

    The data dword is of the indicated type. The next entry after all of the values for this entry,
    the next entry (if there is one). After the last entry, we may either find ourselves at the string
    table, meaning that the parsing is complete, or we are at another new section, and we start over again. 
*/

namespace PlayerCntlSetup
{
    public class FLDataFile
    {
        public class Setting
        {
            public Setting(string sectionName, string settingName, string desc, object[] values)
            {
                this._sectionName = sectionName;
                this._settingName = settingName;
                this._desc = desc;
                this._values = values;
            }

            private string _desc = "";
            private string _sectionName = "";
            private string _settingName = "";
            private object[] _values = new object[0];

            public string desc { get { return _desc; } }
            public string sectionName { get { return _sectionName; } }
            public string settingName { get { return _settingName; } }
            public object[] values { get { return _values; } set { _values = value; }  }

            public uint UInt(int index) { return Convert.ToUInt32(values[index]); }
            public string Str(int index) { return Convert.ToString(values[index]); }
            public string UniStr(int index) { return FLUtility.DecodeUnicodeHex(Convert.ToString(values[index])); }
            public float Float(int index) { return Convert.ToSingle(values[index], System.Globalization.NumberFormatInfo.InvariantInfo); }
            public int NumValues() { return values.Length; }
        };

        public class Section
        {
            public string sectionName = "";
            public List<Setting> settings = new List<Setting>();

            /// <summary>
            /// Returns the first value for the specified setting in this section.
            /// </summary>
            /// <param name="settingName">Key name.</param>
            public Setting GetSetting(string settingName)
            {
                foreach (Setting set in this.settings)
                    if (set.settingName.ToLowerInvariant() == settingName.ToLowerInvariant())
                        return set;

                throw new FLDataFileException("setting " + settingName + " not found in section " + sectionName);
            }

             /// <summary>
            /// Returns true if the setting exists
            /// </summary>
            /// <param name="sectionName">Section name.</param>
            /// <param name="settingName">Key name.</param>
            public bool SettingExists(string settingName)
            {
                foreach (Setting set in this.settings)
                    if (set.settingName.ToLowerInvariant() == settingName.ToLowerInvariant())
                        return true;
                return false;
            }
        };

        /// <summary>
        /// True if no commas are expected for a setting and the whole line should be treated
        /// as a single value.
        /// </summary>
        private bool multiFields = true;

        /// <summary>
        /// The entries in the ini file
        /// </summary>
        public List<Section> sections = new List<Section>();

        /// <summary>
        /// The path of the loaded ini file
        /// </summary>
        public string filePath;

        /// <summary>
        /// True if the file was encrypted when it was loaded
        /// </summary>
        public bool wasEncrypted = false;

        /// <summary>
        /// The ini file contents - decrypted if necessary
        /// </summary>
        private string contents;

        /// <summary>
        /// The super duper microsoft encryption key
        /// </summary>
        private static byte[] gene = { (byte)'G', (byte)'e', (byte)'n', (byte)'e' };

        /// <summary>
        /// Create an empty Freelancer ini file.
        /// </summary>
        /// <param name="multiFields">if true commas delimit fields</param>
        public FLDataFile(bool multiFields)
        {
            this.multiFields = multiFields;
        }

        /// <summary>
        /// Open and parse a Freelancer ini file into an setting list.
        /// </summary>
        /// <param name="filePath">The file to load.</param>
        /// <param name="multiFields">if true commas delimit fields</param>
        public FLDataFile(string filePath, bool multiFields)
        {
            this.multiFields = multiFields;
            this.filePath = filePath;
            byte[] buf;
            using (FileStream fs = new FileStream(filePath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
            {
                buf = new byte[fs.Length];
                fs.Read(buf, 0, (int)fs.Length);
                fs.Close();
            }
            Parse(buf, Path.GetFileName(filePath));
        }

        /// <summary>
        /// Parse a Freelancer ini file in the specified array
        /// </summary>
        /// <param name="buf">The array to parse.</param>
        /// <param name="filePath">This file is not loaded but retained for diagnostics.</param>
        /// <param name="multiFields">if true commas delimit fields</param>
        public FLDataFile(byte[] buf, string fileName, bool multiFields)
        {
            this.filePath = fileName;
            Parse(buf, fileName);
        }

        /// <summary>
        /// Parse a Freelancer ini file in the specified array
        /// </summary>
        /// <param name="buf">The array to parse.</param>
        private void Parse(byte[] buf, string fileName)
        {
            // If this is an encrypted FL ini file then decypt it 
            // and overwrite the original buffer.
            if (buf.Length >= 4 && buf[0] == 'F' && buf[1] == 'L' && buf[2] == 'S' && buf[3] == '1')
            {
                byte[] dbuf = new byte[buf.Length - 4];
                for (int i = 4; i < buf.Length; i++)
                {
                    int k = (gene[i % 4] + (i - 4)) % 256;
                    dbuf[i - 4] = (byte)(buf[i] ^ (k | 0x80));
                }
                buf = dbuf;
                wasEncrypted = true;
            }

            StreamReader sr = new StreamReader(new MemoryStream(buf));
            contents += sr.ReadToEnd();
            sr.Close();

            // If the file is empty then this is an error
            if (buf.Length == 0)
                throw new FLDataFileException("File is empty");

            // If this is a bini file then decode it
            if (buf.Length >= 12 && buf[0] == 'B' && buf[1] == 'I' && buf[2] == 'N' && buf[3] == 'I')
            {
                LoadBinaryIni(buf, fileName);
            }
            else
            {
                LoadTextIni(buf, fileName);
            }
        }

        void LoadBinaryIni(byte[] buf, string fileName)
        {
            int p = 4;
            int version = System.BitConverter.ToInt32(buf, p); p+=4;
            int strTableOffset = System.BitConverter.ToInt32(buf, p); p+=4;

            while (p < buf.Length && p < strTableOffset)
            {
                int sectionStrOffset = System.BitConverter.ToInt16(buf, p); p+=2;
                int sectionNumEntries = System.BitConverter.ToInt16(buf, p); p+=2;
                string sectionName = BufToString(buf, strTableOffset + sectionStrOffset);

                Section section = new Section();
                section.sectionName = sectionName;
                sections.Add(section);

                while (sectionNumEntries-- > 0)
                {
                    int entryStrOffset = System.BitConverter.ToInt16(buf, p); p+=2;
                    int entryNumValues = buf[p++];

                    string settingName = BufToString(buf, strTableOffset + entryStrOffset);
                    string desc = fileName + ":0x" + p.ToString("x") + " '" + settingName+"'";
                    object[] values = new object[entryNumValues];

                    for (int currentValue = 0; currentValue < entryNumValues; currentValue++)
                    {
                        int valueType = buf[p++];
                        int value = System.BitConverter.ToInt32(buf, p); p+=4;
                        switch (valueType)
                        {
                            case 1: // Integer
                                values[currentValue] = (uint)value;
                                break;
                            case 2: // Float
                                values[currentValue] = System.BitConverter.ToSingle(buf, p - 4);
                                break;
                            case 3: // String
                                values[currentValue] = BufToString(buf, strTableOffset + value);
                                break;
                            default:
                                throw new FLDataFileException("Unexpected value type at offset=" + (p - 1));
                        }
                    }

                    Setting setting = new Setting(sectionName, settingName, desc, values);
                    section.settings.Add(setting);
                }
            }
        }

        string BufToString(byte[] buf, int offset)
        {
            // Find the null byte in the str
            int strLen = 0;
            for (int i=offset; i<buf.Length && buf[i]!=0; i++, strLen++)
                ;
            return System.Text.Encoding.ASCII.GetString(buf, offset, strLen);
        }

        void LoadTextIni(byte[] buf, string fileName)
        {
            Section sec = null;

            try
            {
                int lineNumber = 0;
                StreamReader sr = new StreamReader(new MemoryStream(buf));
                string strLine = sr.ReadLine(); lineNumber++;
                while (strLine != null)
                {
                    strLine = strLine.Trim();

                    if (strLine != "")
                    {
                        if (strLine.StartsWith("[") && strLine.EndsWith("]"))
                        {
                            string sectionName = strLine.Substring(1, strLine.Length - 2);
                            sec = new Section();
                            sec.sectionName = sectionName;
                            sections.Add(sec);
                        }
                        else
                        {
                            // Create a dummy section for files with out a [] section.
                            if (sec == null)
                            {
                                sec = new Section();
                                sec.sectionName = "ROOT";
                                sections.Add(sec);
                            }

                            // Strip comments at the end of the line.
                            int commentStartIdx = strLine.IndexOf(';');
                            if (commentStartIdx != -1)
                                strLine = strLine.Substring(0, commentStartIdx);

                            // Parse the damn line.
                            if (strLine != "")
                            {
                                string[] keyPair = strLine.Split(new char[] { '=' }, 2);
                                string settingName = keyPair[0].Trim();
                                string desc = fileName+":"+lineNumber+" '" + strLine +"'";

                                Setting set;
                                if (multiFields)
                                {
                                    string[] values = new string[0];
                                    if (keyPair.Length > 1)
                                        values = keyPair[1].Trim().Split(new char[] { ',' });

                                    object[] settingValues = new object[values.Length];
                                    for (int i = 0; i < values.Length; i++)
                                        settingValues[i] = values[i].Trim();

                                    set = new Setting(sec.sectionName, settingName, desc, settingValues);
                                }
                                else
                                {
                                    object[] settingValues = new object[0];
                                    if (keyPair.Length > 1)
                                        settingValues = new string[] { keyPair[1].Trim() };
                                    set = new Setting(sec.sectionName, settingName, desc, settingValues);
                                }
                                sec.settings.Add(set);
                            }
                        }
                    }

                    strLine = sr.ReadLine(); lineNumber++;
                }
                sr.Close();
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        /// <summary>
        /// Returns true if the section exists
        /// </summary>
        /// <param name="sectionName">Section name.</param>
        public bool SectionExists(string sectionName)
        {
            foreach (Section sec in sections)
                if (sec.sectionName.ToLowerInvariant() == sectionName.ToLowerInvariant())
                    return true;
            return false;
        }

        /// <summary>
        /// Returns true if the section/setting exists
        /// </summary>
        /// <param name="sectionName">Section name.</param>
        /// <param name="settingName">Key name.</param>
        public bool SettingExists(string sectionName, string settingName)
        {
            foreach (Section sec in sections)
                if (sec.sectionName.ToLowerInvariant() == sectionName.ToLowerInvariant())
                    return sec.SettingExists(settingName);
            return false;
        }

        /// <summary>
        /// Returns the value array for the given section/key pair.
        /// </summary>
        /// <param name="sectionName">Section name.</param>
        /// <param name="settingName">Key name.</param>
        public FLDataFile.Setting GetSetting(string sectionName, string settingName)
        {
            foreach (Section sec in sections)
                if (sec.sectionName.ToLowerInvariant() == sectionName.ToLowerInvariant())
                    return sec.GetSetting(settingName);

            throw new FLDataFileException("section " + sectionName + " not found in " + Path.GetFileName(filePath));
        }

        /// <summary>
        /// Returns the value array for the given section/key pair.
        /// </summary>
        /// <param name="sectionName">Section name.</param>
        /// <param name="settingName">Key name.</param>
        public bool GetSettingBool(string sectionName, string settingName, bool defaultValue)
        {
            try
            {
                foreach (Section sec in sections)
                {
                    if (sec.sectionName.ToLowerInvariant() == sectionName.ToLowerInvariant())
                    {
                        string value = sec.GetSetting(settingName).Str(0).Trim();
                        if (value == "1")
                            return true;
                        if (value == "yes")
                            return true;
                        if (value == "True")
                            return true;
                        return false;
                    }
                }
            }
            catch
            {
            }
            return defaultValue;
        }


        /// <summary>
        /// Returns the value array for the given section/key pair.
        /// </summary>
        /// <param name="sectionName">Section name.</param>
        /// <param name="settingName">Key name.</param>
        public string GetSettingStr(string sectionName, string settingName, string defaultValue)
        {
            try
            {
                foreach (Section sec in sections)
                    if (sec.sectionName.ToLowerInvariant() == sectionName.ToLowerInvariant())
                        return sec.GetSetting(settingName).Str(0);
            }
            catch
            {
            }
            return defaultValue;
        }


        /// <summary>
        /// Adds or replaces a setting to the table to be saved. If the new setting value is
        /// different to the current setting value or the key doesn't exist then return true
        /// to indicate that the settings should be saved. Otherwise return false to indicate
        /// that the ini file contents have not changed.
        /// </summary>
        /// <param name="sectionName">Section to add under.</param>
        /// <param name="settingName">Key name to add.</param>
        /// <param name="settingValue">Value of key.</param>
        public bool AddSetting(string sectionName, string settingName, object[] settingValues)
        {
            foreach (Section sec in sections)
            {
                if (sec.sectionName.ToLowerInvariant() == sectionName.ToLowerInvariant())
                {
                    // Find the setting in this section
                    foreach (Setting set in sec.settings)
                    {
                        if (set.settingName.ToLowerInvariant() == settingName.ToLowerInvariant())
                        {
                            set.values = settingValues;
                            return true;
                        }
                    }

                    // Not found so add a new entry to this section.
                    sec.settings.Add(new Setting(sec.sectionName, settingName, "unknown", settingValues));
                    return true;
                }
            }

            // The entry section wasn't found, add a new section.
            Section ns = new Section();
            ns.sectionName = sectionName;
            ns.settings.Add(new Setting(sectionName, settingName, "unknown", settingValues));
            sections.Add(ns);
            return true;
        }


        /// <summary>
        /// Adds a setting to the table to be saved. Does not check for uniqueness so that
        /// multiple entries with the same keys are allowed.
        /// </summary>
        /// <param name="sectionName">Section to add under.</param>
        /// <param name="settingName">Key name to add.</param>
        /// <param name="settingValue">Value of key.</param>
        public void AddSettingNotUnique(string sectionName, string settingName, object[] settingValues)
        {
            // If the section exists add this entry to the existing section
            foreach (Section sec in sections)
            {
                if (sec.sectionName.ToLowerInvariant() == sectionName.ToLowerInvariant())
                {
                    sec.settings.Add(new Setting(sec.sectionName, settingName, "unknown", settingValues));
                    return;
                }
            }

            // The section wasn't found, add a new section.
            Section ns = new Section();
            ns.sectionName = sectionName;
            ns.settings.Add(new Setting(sectionName, settingName, "unknown", settingValues));
            sections.Add(ns);

        }

        /// <summary>
        /// Remove a setting.
        /// </summary>
        /// <param name="sectionName">Section to delete from.</param>
        /// <param name="settingName">Key name to delete.</param>
        /// <returns>Return true if a setting was deleted</returns>
        public bool DeleteSetting(string sectionName, string settingName)
        {
            foreach (Section sec in sections)
            {
                if (sec.sectionName.ToLowerInvariant() == sectionName.ToLowerInvariant())
                {
                    // Find the entry in this section
                    foreach (Setting set in sec.settings)
                    {
                        if (set.settingName.ToLowerInvariant() == settingName.ToLowerInvariant())
                        {
                            sec.settings.Remove(set);
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        /// <summary>
        /// Delete a setting by reference. This is useful if you have multiple
        /// settings with the same name.
        /// </summary>
        /// <param name="sectionName">The setting to delete.</param>
        /// <returns>Return true if the setting was deleted, otherwise false.</returns>
        public bool DeleteSetting(Setting setting)
        {
            foreach (Section sec in sections)
            {
                foreach (Setting set in sec.settings)
                {
                    if (set==setting)
                    {
                        sec.settings.Remove(set);
                        return true;
                    }
                }
            }
            return false;
        }


        /// <summary>
        /// Remove a section.
        /// </summary>
        /// <param name="sectionName">Section to delete<param>
        /// <returns>Return true if a section was deleted</returns>
        public bool DeleteSection(string sectionName)
        {
            foreach (Section sec in sections)
            {
                if (sec.sectionName.ToLowerInvariant() == sectionName.ToLowerInvariant())
                {
                    sections.Remove(sec);
                    return true;
                }
            }
            return false;
        }


        /// <summary>
        /// Return a list of all items with the specified section name and setting name
        /// </summary>
        /// <param name="sectionName">The section name.</param>
        /// <param name="settingName">The setting name.</param>
        /// <returns></returns>
        public List<Setting> GetSettings(string sectionName, string settingName)
        {
            List<Setting> result = new List<Setting>();
            foreach (Section sec in sections)
            {
                if (sec.sectionName.ToLowerInvariant() == sectionName.ToLowerInvariant())
                {
                    foreach (Setting set in sec.settings)
                    {
                        if (settingName == null)
                        {
                            result.Add(set);
                        }
                        else if (set.settingName.ToLowerInvariant() == settingName.ToLowerInvariant())
                        {
                            result.Add(set);
                        }
                    }
                }
            }
            return result;
        }

        /// <summary>
        /// Return a list of all items with the specified section name
        /// </summary>
        /// <param name="sectionName">The section name.</param>
        /// <returns></returns>
        public List<Setting> GetSettings(string sectionName)
        {
            List<Setting> result = new List<Setting>();
            foreach (Section sec in sections)
            {
                if (sec.sectionName.ToLowerInvariant() == sectionName.ToLowerInvariant())
                {
                    foreach (Setting set in sec.settings)
                    {
                        result.Add(set);
                    }
                }
            }
            return result;
        }

        /// <summary>
        /// Save settings to the file. Note that this doesn't support bini save
        /// but will encrypt the file if requested.
        /// </summary>
        /// <param name="filePath">The path to save the file too.</param>
        /// <param name="encrypt">If true then encrypt the file.</param>
        public void SaveSettings(string filePath, bool encrypt)
        {
            // Build the ini file in a string, section by section
            StringBuilder strToSave = new StringBuilder();
            foreach (Section sec in sections)
            {
                strToSave.AppendLine("[" + sec.sectionName + "]");

                foreach (Setting setting in sec.settings)
                {
                    string line = setting.settingName;
                    if (setting.values.Length > 0)
                    {
                        line += " = ";
                        for (int i = 0; i < setting.values.Length; i++)
                        {
                            if (i != 0)
                                line += ", ";
                            if (setting.values[i] is float)
                                line += Convert.ToString((float)setting.values[i], System.Globalization.NumberFormatInfo.InvariantInfo);
                            else if (setting.values[i] is uint)
                                line += Convert.ToString((uint)setting.values[i], System.Globalization.NumberFormatInfo.InvariantInfo);
                            else if (setting.values[i] is int)
                                line += Convert.ToString((int)setting.values[i], System.Globalization.NumberFormatInfo.InvariantInfo);
                            else if (setting.values[i] is bool)
                                line += ((bool)setting.values[i])?"yes":"no";
                            else
                                line += setting.values[i];
                        }
                    }
                    strToSave.AppendLine(line);
                }
                strToSave.AppendLine("");
            }

            try
            {
                // Encrypt the ini file if necessary
                byte[] buf = new System.Text.ASCIIEncoding().GetBytes(strToSave.ToString());
                if (encrypt)
                {
                    byte[] nBuf = new byte[buf.Length + 4];
                    nBuf[0] = (byte)'F';
                    nBuf[1] = (byte)'L';
                    nBuf[2] = (byte)'S';
                    nBuf[3] = (byte)'1';
                    for (int i = 0; i < buf.Length; i++)
                    {
                        int k = (gene[i % 4] + i) % 256;
                        nBuf[i + 4] = (byte)(buf[i] ^ (k | 0x80));
                    }
                    buf = nBuf;
                }

                // Write it back to the original location
                using (FileStream fs = new FileStream(filePath, FileMode.Create, FileAccess.Write, FileShare.Read))
                {
                    fs.Write(buf, 0, buf.Length);
                    fs.Close();
                }
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        /// <summary>
        /// Returns the ini file contents as a string.
        public string GetIniFileContents()
        {
            return contents;
        }
    }
}
