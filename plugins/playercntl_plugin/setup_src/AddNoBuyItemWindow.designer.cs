namespace PlayerCntlSetup
{
    partial class AddNoBuyItemWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AddShipPimperBaseWindow));
            this.cancelButton = new System.Windows.Forms.Button();
            this.okButton = new System.Windows.Forms.Button();
            this.dataGridView1 = new System.Windows.Forms.DataGridView();
            this.iDSNameDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.itemNickNameDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.itemTypeDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.itemHashDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.IDSInfo = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.IDSInfo1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.IDSInfo2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.IDSInfo3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.hashListBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.gameDataSet = new PlayerCntlSetup.GameDataSet();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.richTextBoxInfo = new System.Windows.Forms.RichTextBox();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.hashListBindingSource)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.gameDataSet)).BeginInit();
            this.SuspendLayout();
            // 
            // cancelButton
            // 
            this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(433, 357);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 0;
            this.cancelButton.Text = "Close";
            this.cancelButton.UseVisualStyleBackColor = true;
            this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // okButton
            // 
            this.okButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.okButton.Location = new System.Drawing.Point(352, 357);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(75, 23);
            this.okButton.TabIndex = 1;
            this.okButton.Text = "Add";
            this.okButton.UseVisualStyleBackColor = true;
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // dataGridView1
            // 
            this.dataGridView1.AllowUserToAddRows = false;
            this.dataGridView1.AllowUserToDeleteRows = false;
            this.dataGridView1.AllowUserToResizeRows = false;
            this.dataGridView1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridView1.AutoGenerateColumns = false;
            this.dataGridView1.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridView1.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.iDSNameDataGridViewTextBoxColumn,
            this.itemNickNameDataGridViewTextBoxColumn,
            this.itemTypeDataGridViewTextBoxColumn,
            this.itemHashDataGridViewTextBoxColumn,
            this.IDSInfo,
            this.IDSInfo1,
            this.IDSInfo2,
            this.IDSInfo3});
            this.dataGridView1.DataSource = this.hashListBindingSource;
            this.dataGridView1.Location = new System.Drawing.Point(14, 12);
            this.dataGridView1.Name = "dataGridView1";
            this.dataGridView1.ReadOnly = true;
            this.dataGridView1.RowHeadersVisible = false;
            this.dataGridView1.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridView1.Size = new System.Drawing.Size(332, 339);
            this.dataGridView1.TabIndex = 7;
            this.dataGridView1.CellDoubleClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.acItemGrid_CellDoubleClick);
            this.dataGridView1.SelectionChanged += new System.EventHandler(this.dataGridView1_SelectionChanged);
            // 
            // iDSNameDataGridViewTextBoxColumn
            // 
            this.iDSNameDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.iDSNameDataGridViewTextBoxColumn.DataPropertyName = "IDSName";
            this.iDSNameDataGridViewTextBoxColumn.HeaderText = "Description";
            this.iDSNameDataGridViewTextBoxColumn.Name = "iDSNameDataGridViewTextBoxColumn";
            this.iDSNameDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // itemNickNameDataGridViewTextBoxColumn
            // 
            this.itemNickNameDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.itemNickNameDataGridViewTextBoxColumn.DataPropertyName = "ItemNickName";
            this.itemNickNameDataGridViewTextBoxColumn.HeaderText = "NickName";
            this.itemNickNameDataGridViewTextBoxColumn.Name = "itemNickNameDataGridViewTextBoxColumn";
            this.itemNickNameDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // itemTypeDataGridViewTextBoxColumn
            // 
            this.itemTypeDataGridViewTextBoxColumn.DataPropertyName = "ItemType";
            this.itemTypeDataGridViewTextBoxColumn.HeaderText = "ItemType";
            this.itemTypeDataGridViewTextBoxColumn.Name = "itemTypeDataGridViewTextBoxColumn";
            this.itemTypeDataGridViewTextBoxColumn.ReadOnly = true;
            this.itemTypeDataGridViewTextBoxColumn.Visible = false;
            // 
            // itemHashDataGridViewTextBoxColumn
            // 
            this.itemHashDataGridViewTextBoxColumn.DataPropertyName = "ItemHash";
            this.itemHashDataGridViewTextBoxColumn.HeaderText = "ItemHash";
            this.itemHashDataGridViewTextBoxColumn.Name = "itemHashDataGridViewTextBoxColumn";
            this.itemHashDataGridViewTextBoxColumn.ReadOnly = true;
            this.itemHashDataGridViewTextBoxColumn.Visible = false;
            // 
            // IDSInfo
            // 
            this.IDSInfo.DataPropertyName = "IDSInfo";
            this.IDSInfo.HeaderText = "IDSInfo";
            this.IDSInfo.Name = "IDSInfo";
            this.IDSInfo.ReadOnly = true;
            this.IDSInfo.Visible = false;
            // 
            // IDSInfo1
            // 
            this.IDSInfo1.DataPropertyName = "IDSInfo1";
            this.IDSInfo1.HeaderText = "IDSInfo1";
            this.IDSInfo1.Name = "IDSInfo1";
            this.IDSInfo1.ReadOnly = true;
            this.IDSInfo1.Visible = false;
            // 
            // IDSInfo2
            // 
            this.IDSInfo2.DataPropertyName = "IDSInfo2";
            this.IDSInfo2.HeaderText = "IDSInfo2";
            this.IDSInfo2.Name = "IDSInfo2";
            this.IDSInfo2.ReadOnly = true;
            this.IDSInfo2.Visible = false;
            // 
            // IDSInfo3
            // 
            this.IDSInfo3.DataPropertyName = "IDSInfo3";
            this.IDSInfo3.HeaderText = "IDSInfo3";
            this.IDSInfo3.Name = "IDSInfo3";
            this.IDSInfo3.ReadOnly = true;
            this.IDSInfo3.Visible = false;
            // 
            // hashListBindingSource
            // 
            this.hashListBindingSource.DataMember = "HashList";
            this.hashListBindingSource.DataSource = this.gameDataSet;
            // 
            // gameDataSet
            // 
            this.gameDataSet.DataSetName = "GameDataSet";
            this.gameDataSet.SchemaSerializationMode = System.Data.SchemaSerializationMode.IncludeSchema;
            // 
            // textBox1
            // 
            this.textBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.textBox1.Location = new System.Drawing.Point(52, 359);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(202, 20);
            this.textBox1.TabIndex = 10;
            this.textBox1.TextChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // label3
            // 
            this.label3.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(17, 362);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(29, 13);
            this.label3.TabIndex = 9;
            this.label3.Text = "Filter";
            // 
            // richTextBoxInfo
            // 
            this.richTextBoxInfo.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxInfo.Location = new System.Drawing.Point(352, 12);
            this.richTextBoxInfo.Name = "richTextBoxInfo";
            this.richTextBoxInfo.Size = new System.Drawing.Size(156, 339);
            this.richTextBoxInfo.TabIndex = 11;
            this.richTextBoxInfo.Text = "";
            // 
            // timer1
            // 
            this.timer1.Interval = 300;
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // AddShipPimperBaseWindow
            // 
            this.AcceptButton = this.okButton;
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(520, 392);
            this.Controls.Add(this.richTextBoxInfo);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.dataGridView1);
            this.Controls.Add(this.okButton);
            this.Controls.Add(this.cancelButton);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "AddShipPimperBaseWindow";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Add Base";
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.hashListBindingSource)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.gameDataSet)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.DataGridView dataGridView1;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.RichTextBox richTextBoxInfo;
        private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.BindingSource hashListBindingSource;
        private GameDataSet gameDataSet;
        private System.Windows.Forms.DataGridViewTextBoxColumn iDSNameDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn itemNickNameDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn itemTypeDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn itemHashDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn IDSInfo;
        private System.Windows.Forms.DataGridViewTextBoxColumn IDSInfo1;
        private System.Windows.Forms.DataGridViewTextBoxColumn IDSInfo2;
        private System.Windows.Forms.DataGridViewTextBoxColumn IDSInfo3;
    }
}