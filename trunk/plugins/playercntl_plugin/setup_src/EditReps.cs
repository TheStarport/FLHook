using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace PlayerCntlSetup
{
    public partial class EditReps : Form
    {
        // The adjustment mode. If the player's reputation for scRepGroup
        // is greater than fRep then make the reputation equal to fRep
        const uint MODE_REP_LESSTHAN = 0;

        // The adjustment mode. If the player's reputation for scRepGroup
        // is less than fRep then make the reputation equal to fRep
        const uint MODE_REP_GREATERTHAN = 1;

        // Don't change anything/ignore this reputation group.
        const uint MODE_REP_NO_CHANGE = 2;

        // Don't change anything/ignore this reputation group.
        const uint MODE_REP_STATIC = 3;

        // The configuration we're editing.
        FLDataFile cfgFile;

        // The item we're setting up faction reps for.
        string itemNickName;

        public EditReps(FLGameData gameData, string itemNickName, FLDataFile cfgFile)
        {
            InitializeComponent();

            this.itemNickName = itemNickName;
            this.cfgFile = cfgFile;
            this.Text = "Edit Reputations: " + gameData.GetItemDescByNickNameX(itemNickName);

            GameDataSet.HashListRow[] factions = (GameDataSet.HashListRow[])gameData.DataStore.HashList.Select("ItemType = '" + FLGameData.GAMEDATA_FACTIONS + "'");
            foreach (GameDataSet.HashListRow faction in factions)
            {
                uIDataSet.RepFixerItemFactions.AddRepFixerItemFactionsRow(faction.IDSName, faction.ItemNickName, "");
            }

            foreach (FLDataFile.Setting set in cfgFile.GetSettings(itemNickName))
            {
                string factionNick = set.settingName;

                string repStr = "-";
                if (set.NumValues() > 0)
                {
                    string[] values = set.Str(0).Split(',');
                    if (values.Length == 2)
                    {
                        int mode;
                        if (!Int32.TryParse(values[1], out mode))
                            repStr = "ERR";
                        else if (mode == MODE_REP_GREATERTHAN)
                            repStr = ">";
                        else if (mode == MODE_REP_STATIC)
                            repStr = "=";
                        else
                            repStr = "<";

                        float rep;
                        if (!Single.TryParse(values[0], out rep))
                            repStr = "ERR";
                        else if (rep > 1.0 || rep < -1.0)
                            repStr = "ERR";
                        else
                            repStr += rep.ToString();
                    }
                    else if (values.Length == 1)
                    {
                        float rep;
                        if (!Single.TryParse(values[0], out rep))
                            repStr = "ERR";
                        else if (rep > 1.0 || rep < -1.0)
                            repStr = "ERR";
                        repStr = "<" + rep.ToString();
                    }
                }

                foreach (UIDataSet.RepFixerItemFactionsRow faction in uIDataSet.RepFixerItemFactions.Rows)
                {
                    if (faction.ItemNickName == factionNick)
                    {
                        faction.Reputation = repStr;
                        break;
                    }
                }
            }
        }

        private void EditReps_Load(object sender, EventArgs e)
        {
            dataGridView1.Sort(dataGridView1.Columns[iDSNameDataGridViewTextBoxColumn.Index], ListSortDirection.Ascending);           
        }

        private void dataGridView1_CellEndEdit(object sender, DataGridViewCellEventArgs e)
        {
            string repStr = dataGridView1.Rows[e.RowIndex].Cells[e.ColumnIndex].Value.ToString();

            if (repStr == "-")
                return;

            float rep;
            if (!Single.TryParse(repStr.Substring(1), out rep))
                dataGridView1.Rows[e.RowIndex].Cells[e.ColumnIndex].Value = "ERR";

            else if (rep > 1.0 || rep < -1.0)
                dataGridView1.Rows[e.RowIndex].Cells[e.ColumnIndex].Value = "ERR";

            if (!repStr.StartsWith("<") && !repStr.StartsWith(">") && !repStr.StartsWith("="))
                dataGridView1.Rows[e.RowIndex].Cells[e.ColumnIndex].Value = "ERR";
        }


        /// <summary>
        /// Change the rep to the next state on a 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void dataGridView1_CellClick(object sender, DataGridViewCellMouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right && e.ColumnIndex == 2)
            {
                for (int i = 0; i < e.Clicks; i++)
                {
                    for (int clicks = e.Clicks; clicks > 0; clicks--)
                    {
                        string repStr = dataGridView1.Rows[e.RowIndex].Cells[e.ColumnIndex].Value.ToString();

                        if (repStr == ">-0.7")
                        {
                            repStr = ">0";
                        }
                        else if (repStr == ">0")
                        {
                            repStr = ">0.7";
                        }
                        else if (repStr == ">0.7")
                        {
                            repStr = "-";
                        }
                        else if (repStr == "-")
                        {
                            repStr = "<-0.7";
                        }
                        else if (repStr == "<-0.7")
                        {
                            repStr = "<0";
                        }
                        else if (repStr == "<0")
                        {
                            repStr = "<0.7";
                        }
                        else
                        {
                            repStr = ">-0.7";
                        }

                        dataGridView1.Rows[e.RowIndex].Cells[e.ColumnIndex].Value = repStr;
                    }
                }
            }
        }

        private void buttonOK_Click(object sender, EventArgs e)
        {
            cfgFile.DeleteSection(itemNickName);
            foreach (UIDataSet.RepFixerItemFactionsRow faction in uIDataSet.RepFixerItemFactions.Rows)
            {
                string repStr = faction.Reputation;
                if (repStr == "-")
                {
                    cfgFile.AddSetting(itemNickName, faction.ItemNickName, new object[] { });
                }
                else if (repStr.StartsWith("<"))
                {
                    cfgFile.AddSetting(itemNickName, faction.ItemNickName, new object[] {
                        String.Format("{0:0.00}, {1}", Single.Parse(repStr.Substring(1)), MODE_REP_LESSTHAN) });
                }
                else if (repStr.StartsWith(">"))
                {
                    cfgFile.AddSetting(itemNickName, faction.ItemNickName, new object[] { 
                        String.Format("{0:0.00}, {1}", Single.Parse(repStr.Substring(1)), MODE_REP_GREATERTHAN) });
                }
                else if (repStr.StartsWith("="))
                {
                    cfgFile.AddSetting(itemNickName, faction.ItemNickName, new object[] { 
                        String.Format("{0:0.00}, {1}", Single.Parse(repStr.Substring(1)), MODE_REP_STATIC) });
                }
            }
            this.Close();
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void dataGridView1_SelectionChanged(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridView1.SelectedRows)
            {
                UIDataSet.RepFixerItemFactionsRow item = (UIDataSet.RepFixerItemFactionsRow)((DataRowView)row.DataBoundItem).Row;
                float rep = 0;
                try
                {
                    if (!Single.TryParse(item.Reputation, out rep))
                        Single.TryParse(item.Reputation.Substring(1), out rep);
                }
                catch { }
                this.trackBar1.ValueChanged -= new System.EventHandler(this.trackBar1_ValueChanged);
                trackBar1.Value = (int)(rep * 100f);
                this.trackBar1.ValueChanged += new System.EventHandler(this.trackBar1_ValueChanged);

            }
        }

        private void trackBar1_ValueChanged(object sender, EventArgs e)
        {
            float rep = (trackBar1.Value / 100.0f);
            foreach (DataGridViewRow row in dataGridView1.SelectedRows)
            {
                UIDataSet.RepFixerItemFactionsRow item = (UIDataSet.RepFixerItemFactionsRow)((DataRowView)row.DataBoundItem).Row;
                item.Reputation = String.Format("={0:0.##}", rep);
            }
        }

    }
}
