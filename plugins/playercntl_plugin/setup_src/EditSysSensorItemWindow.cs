using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace PlayerCntlSetup
{
    public partial class EditSysSensorItemWindow : Form
    {
        FLGameData gameData;
        UIDataSet.SysSensorListRow dataItem;

        public EditSysSensorItemWindow(FLGameData gameData, UIDataSet.SysSensorListRow dataItem)
        {
            InitializeComponent();
            
            this.gameData = gameData;
            this.dataItem = dataItem;
            this.Text = "Edit No Buy Item: " + dataItem.SystemDescription;
            this.numericUpDown1.Value = dataItem.NetworkID;

            hashListBindingSource.DataSource = gameData.DataStore;
            FilterUpdate();
        }

        /// <summary>
        /// Do nothing.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void cancelButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        /// <summary>
        /// Add the selected item to the parent's cargo table.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void okButton_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridView1.SelectedRows)
            {
                GameDataSet.HashListRow dataRow = (GameDataSet.HashListRow)((DataRowView)row.DataBoundItem).Row;
                dataItem.EquipDescription = dataRow.IDSName;
                dataItem.EquipNickName = dataRow.ItemNickName;
                dataItem.NetworkID = (uint) numericUpDown1.Value;
                this.Close();
            }

        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            timer1.Start();
        }

        // <summary>
        /// Show details when row is selected
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void dataGridView1_SelectionChanged(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridView1.SelectedRows)
            {
                richTextBoxInfo.Clear();
                richTextBoxInfo.AppendText("@@@INSERTED_RTF_CODE_HACK@@@");
                string rtf = "";

                if ((string)row.Cells[itemTypeDataGridViewTextBoxColumn.Index].Value == "ships")
                {
                    rtf += FLUtility.FLXmlToRtf((string)row.Cells[IDSInfo1.Index].Value);
                    rtf += "\\pard \\par ";
                    rtf += FLUtility.FLXmlToRtf((string)row.Cells[IDSInfo.Index].Value);
                }
                else
                {
                    string xml = (string)row.Cells[IDSInfo.Index].Value;
                    if (xml.Length == 0)
                        xml = "No information available";
                    rtf += FLUtility.FLXmlToRtf(xml);
                }
                richTextBoxInfo.Rtf = richTextBoxInfo.Rtf.Replace("@@@INSERTED_RTF_CODE_HACK@@@", rtf);
                break;
            }
        }

        /// <summary>
        /// Update the filter applied to the character list data grid view.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void FilterUpdate()
        {
            string filter = "((ItemType = '" + FLGameData.GAMEDATA_AMMO + "')";
            filter += " OR (ItemType = '" + FLGameData.GAMEDATA_ARMOR + "')";
            filter += " OR (ItemType = '" + FLGameData.GAMEDATA_CARGO + "')";
            filter += " OR (ItemType = '" + FLGameData.GAMEDATA_CLOAK + "')";
            filter += " OR (ItemType = '" + FLGameData.GAMEDATA_CM + "')";
            filter += " OR (ItemType = '" + FLGameData.GAMEDATA_GUNS + "')";
            filter += " OR (ItemType = '" + FLGameData.GAMEDATA_MINES + "')";
            filter += " OR (ItemType = '" + FLGameData.GAMEDATA_PROJECTILES + "')";
            filter += " OR (ItemType = '" + FLGameData.GAMEDATA_SCANNERS + "')";
            filter += " OR (ItemType = '" + FLGameData.GAMEDATA_SHIELDS + "')";
            filter += " OR (ItemType = '" + FLGameData.GAMEDATA_THRUSTERS + "')";
            filter += " OR (ItemType = '" + FLGameData.GAMEDATA_TRACTORS + "')";
            filter += " OR (ItemType = '" + FLGameData.GAMEDATA_TURRETS + "')";
            filter += ")";

            if (textBox1.Text.Length > 0)
            {
                string filterText = textBox1.Text;
                if (filter != null)
                    filter += " AND ";
                filter += "((IDSName LIKE '%" + FLUtility.EscapeLikeExpressionString(filterText) + "%')";
                filter += " OR (ItemNickName LIKE '%" + FLUtility.EscapeLikeExpressionString(filterText) + "%')";
                filter += " OR (ItemType LIKE '%" + FLUtility.EscapeLikeExpressionString(filterText) + "%'))";
            }
            hashListBindingSource.Filter = filter;
        }


        /// <summary>
        /// A double click is treat like the OK button.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void acItemGrid_CellDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            okButton_Click(sender, e);
        }

        private void checkBoxShowAllTypes_CheckedChanged(object sender, EventArgs e)
        {
            FilterUpdate();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            FilterUpdate();
        }
    }
}
