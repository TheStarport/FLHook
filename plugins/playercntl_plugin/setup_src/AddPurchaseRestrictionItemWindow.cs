using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace PlayerCntlSetup
{
    public partial class AddPurchaseRestrictionItemWindow : Form
    {
        FLGameData gameData;
        UIDataSet.ShipPurchaseRestrictionItemsDataTable dataTable;

        public AddPurchaseRestrictionItemWindow(FLGameData gameData, UIDataSet.ShipPurchaseRestrictionItemsDataTable dataTable)
        {
            this.gameData = gameData;
            this.dataTable = dataTable;

            InitializeComponent();
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
                dataTable.AddShipPurchaseRestrictionItemsRow(gameData.GetItemDescByHash(dataRow.ItemHash), dataRow.ItemNickName, "", "");
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
            string filter = "((ItemType = '" + FLGameData.GAMEDATA_SHIPS + "')";
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
