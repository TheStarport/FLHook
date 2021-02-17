namespace PlayerCntlSetup
{
    partial class Form1
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
            PlayerCntlSetup.GameDataSet gameDataSet;
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.groupBox7 = new System.Windows.Forms.GroupBox();
            this.checkBoxEnableDo = new System.Windows.Forms.CheckBox();
            this.checkBoxEnableMe = new System.Windows.Forms.CheckBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.textBoxSpinImpulseMultiplier = new System.Windows.Forms.TextBox();
            this.S = new System.Windows.Forms.Label();
            this.textBoxSpinProtectMass = new System.Windows.Forms.TextBox();
            this.label41 = new System.Windows.Forms.Label();
            this.groupBox6 = new System.Windows.Forms.GroupBox();
            this.textBoxHullDropFactor = new System.Windows.Forms.TextBox();
            this.label21 = new System.Windows.Forms.Label();
            this.textBoxHullDrop2NickName = new System.Windows.Forms.TextBox();
            this.label18 = new System.Windows.Forms.Label();
            this.textBoxHullDrop1NickName = new System.Windows.Forms.TextBox();
            this.label16 = new System.Windows.Forms.Label();
            this.label17 = new System.Windows.Forms.Label();
            this.textBoxCargoDropContainer = new System.Windows.Forms.TextBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.textBoxDisconnectingPlayersRange = new System.Windows.Forms.TextBox();
            this.label34 = new System.Windows.Forms.Label();
            this.label19 = new System.Windows.Forms.Label();
            this.textBoxDisconnectMsg = new System.Windows.Forms.TextBox();
            this.checkBoxReportDisconnectingPlayers = new System.Windows.Forms.CheckBox();
            this.checkBoxKillDisconnectingPlayers = new System.Windows.Forms.CheckBox();
            this.checkBoxLootDisconnectingPlayers = new System.Windows.Forms.CheckBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.textBoxEscortedShipDamageFactor = new System.Windows.Forms.TextBox();
            this.label43 = new System.Windows.Forms.Label();
            this.label42 = new System.Windows.Forms.Label();
            this.textBoxEscortedShipMinimumMass = new System.Windows.Forms.TextBox();
            this.checkBoxEnableEscort = new System.Windows.Forms.CheckBox();
            this.label25 = new System.Windows.Forms.Label();
            this.textBoxStuckMsg = new System.Windows.Forms.TextBox();
            this.label24 = new System.Windows.Forms.Label();
            this.label23 = new System.Windows.Forms.Label();
            this.textBoxSmiteMusic = new System.Windows.Forms.TextBox();
            this.textBoxCoinMsg = new System.Windows.Forms.TextBox();
            this.textBoxDiceMsg = new System.Windows.Forms.TextBox();
            this.label22 = new System.Windows.Forms.Label();
            this.textBoxPimpShipCost = new System.Windows.Forms.TextBox();
            this.label20 = new System.Windows.Forms.Label();
            this.textBoxDropRepCost = new System.Windows.Forms.TextBox();
            this.checkBoxEnableDropRepAndMisc = new System.Windows.Forms.CheckBox();
            this.label15 = new System.Windows.Forms.Label();
            this.checkBoxEnablePimpShip = new System.Windows.Forms.CheckBox();
            this.tabPage5 = new System.Windows.Forms.TabPage();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.checkBoxEnableGiveCash = new System.Windows.Forms.CheckBox();
            this.button3 = new System.Windows.Forms.Button();
            this.textBoxBlockedSystem = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.textBoxMinTime = new System.Windows.Forms.TextBox();
            this.textBoxMinTransfer = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.checkBoxAsciiCharnameOnly = new System.Windows.Forms.CheckBox();
            this.label46 = new System.Windows.Forms.Label();
            this.textBoxCharnameTagCost = new System.Windows.Forms.TextBox();
            this.label45 = new System.Windows.Forms.Label();
            this.checkBoxCharnameTag = new System.Windows.Forms.CheckBox();
            this.checkBoxEnableMoveChar = new System.Windows.Forms.CheckBox();
            this.label12 = new System.Windows.Forms.Label();
            this.textBoxRenameTimeLimit = new System.Windows.Forms.TextBox();
            this.label14 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.textBoxMoveCost = new System.Windows.Forms.TextBox();
            this.label11 = new System.Windows.Forms.Label();
            this.checkBoxEnableRenameMe = new System.Windows.Forms.CheckBox();
            this.label10 = new System.Windows.Forms.Label();
            this.textBoxRenameCost = new System.Windows.Forms.TextBox();
            this.label13 = new System.Windows.Forms.Label();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.buttonDelItem = new System.Windows.Forms.Button();
            this.buttonAddItem = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.dataGridViewShipPimperEquipList = new System.Windows.Forms.DataGridView();
            this.iDSNameDataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.itemNickNameDataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.itemHashDataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.iDSInfoDataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.iDSInfo1DataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.shipPimperItemListBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.uIDataSet = new PlayerCntlSetup.UIDataSet();
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.buttonDelBase = new System.Windows.Forms.Button();
            this.buttonAddBase = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.dataGridViewShipPimperBases = new System.Windows.Forms.DataGridView();
            this.iDSNameDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.itemNickNameDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.itemHashDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.iDSInfoDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.iDSInfo1DataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.shipPimperBaseListBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.buttonRepDelItem = new System.Windows.Forms.Button();
            this.buttonRepEditItem = new System.Windows.Forms.Button();
            this.buttonRepAddItem = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.dataGridViewRepFixerItems = new System.Windows.Forms.DataGridView();
            this.iDSNameDataGridViewTextBoxColumn2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.itemNickNameDataGridViewTextBoxColumn2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.itemHashDataGridViewTextBoxColumn2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.iDSInfoDataGridViewTextBoxColumn2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.iDSInfo1DataGridViewTextBoxColumn2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.repFixerItemListBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.checkBoxEnableRepFixUpdates = new System.Windows.Forms.CheckBox();
            this.checkBoxLogRepFixUpdates = new System.Windows.Forms.CheckBox();
            this.checkBoxItemMustBeMounted = new System.Windows.Forms.CheckBox();
            this.tabPage6 = new System.Windows.Forms.TabPage();
            this.checkBoxCheckIDRestrictions = new System.Windows.Forms.CheckBox();
            this.buttonImportShipPurchaseRestrictions = new System.Windows.Forms.Button();
            this.buttonPurchaseRestrictionDelItem = new System.Windows.Forms.Button();
            this.buttonPurchaseRestrictionEdittem = new System.Windows.Forms.Button();
            this.buttonPurchaseRestrictionAddItem = new System.Windows.Forms.Button();
            this.label26 = new System.Windows.Forms.Label();
            this.dataGridViewPurchaseRestrictions = new System.Windows.Forms.DataGridView();
            this.descriptionDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.itemNickNameDataGridViewTextBoxColumn3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.controlDescriptionDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.controlItemNickNameDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.shipPurchaseRestrictionItemsBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.checkBoxEnforceIDRestrictions = new System.Windows.Forms.CheckBox();
            this.tabPage11 = new System.Windows.Forms.TabPage();
            this.buttonEquipPurchaseRestrictionDel = new System.Windows.Forms.Button();
            this.buttonEquipPurchaseRestrictionEdit = new System.Windows.Forms.Button();
            this.buttonEquipPurchaseRestrictionAdd = new System.Windows.Forms.Button();
            this.label39 = new System.Windows.Forms.Label();
            this.dataGridViewEquipPurchaseRestriction = new System.Windows.Forms.DataGridView();
            this.descriptionDataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.itemNickNameDataGridViewTextBoxColumn4 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.controlDescriptionDataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.controlItemNickNameDataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.equipPurchaseRestrictionItemsBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.tabPage10 = new System.Windows.Forms.TabPage();
            this.buttonDelNoBuyItem = new System.Windows.Forms.Button();
            this.buttonEditNoBuyItem = new System.Windows.Forms.Button();
            this.buttonAddNoBuyItem = new System.Windows.Forms.Button();
            this.label37 = new System.Windows.Forms.Label();
            this.dataGridViewNoBuyList = new System.Windows.Forms.DataGridView();
            this.baseDescriptionDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.baseNickNameDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.goodDescriptionDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.goodNickNameDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.baseGoodNoBuyListBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.tabPage7 = new System.Windows.Forms.TabPage();
            this.richTextBoxSwearWords = new System.Windows.Forms.RichTextBox();
            this.label44 = new System.Windows.Forms.Label();
            this.richTextBoxHelpLines = new System.Windows.Forms.RichTextBox();
            this.textBoxCmdEchoStyle = new System.Windows.Forms.TextBox();
            this.label28 = new System.Windows.Forms.Label();
            this.checkBoxCmdHide = new System.Windows.Forms.CheckBox();
            this.checkBoxCmdEcho = new System.Windows.Forms.CheckBox();
            this.label27 = new System.Windows.Forms.Label();
            this.checkBoxCustomHelp = new System.Windows.Forms.CheckBox();
            this.tabPage8 = new System.Windows.Forms.TabPage();
            this.label32 = new System.Windows.Forms.Label();
            this.textBoxStandardBannerDelay = new System.Windows.Forms.TextBox();
            this.buttonBannerDelItem = new System.Windows.Forms.Button();
            this.buttonBannerEditItem = new System.Windows.Forms.Button();
            this.buttonBannerAddItem = new System.Windows.Forms.Button();
            this.dataGridViewStandardBanners = new System.Windows.Forms.DataGridView();
            this.bannerTextDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.standardBannerListBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.label31 = new System.Windows.Forms.Label();
            this.tabPage9 = new System.Windows.Forms.TabPage();
            this.richTextBox1 = new System.Windows.Forms.RichTextBox();
            this.richTextBoxGreetingBannerLines = new System.Windows.Forms.RichTextBox();
            this.label29 = new System.Windows.Forms.Label();
            this.richTextBoxSpecialBannerLines = new System.Windows.Forms.RichTextBox();
            this.label33 = new System.Windows.Forms.Label();
            this.textBoxSpecialBannerDelay = new System.Windows.Forms.TextBox();
            this.label30 = new System.Windows.Forms.Label();
            this.tabPage12 = new System.Windows.Forms.TabPage();
            this.buttonSysSensorDelItem = new System.Windows.Forms.Button();
            this.buttonSysSensorEditItem = new System.Windows.Forms.Button();
            this.buttonSysSensorAddItem = new System.Windows.Forms.Button();
            this.label40 = new System.Windows.Forms.Label();
            this.dataGridViewSysSensor = new System.Windows.Forms.DataGridView();
            this.systemDescriptionDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.systemNickNameDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.equipDescriptionDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.equipNickNameDataGridViewTextBoxColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.NetworkID = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.sysSensorListBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this.tabPage13 = new System.Windows.Forms.TabPage();
            this.richTextBoxSounds = new System.Windows.Forms.RichTextBox();
            this.label47 = new System.Windows.Forms.Label();
            this.checkBoxEnableLoginSound = new System.Windows.Forms.CheckBox();
            this.tabPage14 = new System.Windows.Forms.TabPage();
            this.richTextBoxBodies = new System.Windows.Forms.RichTextBox();
            this.label49 = new System.Windows.Forms.Label();
            this.richTextBoxHeads = new System.Windows.Forms.RichTextBox();
            this.label48 = new System.Windows.Forms.Label();
            this.checkBoxEnableWardrobe = new System.Windows.Forms.CheckBox();
            this.tabPage15 = new System.Windows.Forms.TabPage();
            this.textBoxRestartMaxRank = new System.Windows.Forms.TextBox();
            this.checkBoxEnableRestart = new System.Windows.Forms.CheckBox();
            this.textBoxRestartMaxCash = new System.Windows.Forms.TextBox();
            this.label36 = new System.Windows.Forms.Label();
            this.label35 = new System.Windows.Forms.Label();
            this.richTextBoxRestarts = new System.Windows.Forms.RichTextBox();
            this.label50 = new System.Windows.Forms.Label();
            this.checkBoxEnableRestartCost = new System.Windows.Forms.CheckBox();
            this.buttonClose = new System.Windows.Forms.Button();
            this.buttonSave = new System.Windows.Forms.Button();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.button4 = new System.Windows.Forms.Button();
            this.label38 = new System.Windows.Forms.Label();
            this.dataGridView2 = new System.Windows.Forms.DataGridView();
            this.dataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn4 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            gameDataSet = new PlayerCntlSetup.GameDataSet();
            ((System.ComponentModel.ISupportInitialize)(gameDataSet)).BeginInit();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.groupBox7.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox6.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.tabPage5.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.tabPage2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewShipPimperEquipList)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.shipPimperItemListBindingSource)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.uIDataSet)).BeginInit();
            this.tabPage4.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewShipPimperBases)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.shipPimperBaseListBindingSource)).BeginInit();
            this.tabPage3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewRepFixerItems)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.repFixerItemListBindingSource)).BeginInit();
            this.tabPage6.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewPurchaseRestrictions)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.shipPurchaseRestrictionItemsBindingSource)).BeginInit();
            this.tabPage11.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewEquipPurchaseRestriction)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.equipPurchaseRestrictionItemsBindingSource)).BeginInit();
            this.tabPage10.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewNoBuyList)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.baseGoodNoBuyListBindingSource)).BeginInit();
            this.tabPage7.SuspendLayout();
            this.tabPage8.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewStandardBanners)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.standardBannerListBindingSource)).BeginInit();
            this.tabPage9.SuspendLayout();
            this.tabPage12.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewSysSensor)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.sysSensorListBindingSource)).BeginInit();
            this.tabPage13.SuspendLayout();
            this.tabPage14.SuspendLayout();
            this.tabPage15.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView2)).BeginInit();
            this.SuspendLayout();
            // 
            // gameDataSet
            // 
            gameDataSet.DataSetName = "GameDataSet";
            gameDataSet.SchemaSerializationMode = System.Data.SchemaSerializationMode.IncludeSchema;
            // 
            // tabControl1
            // 
            this.tabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage5);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage4);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Controls.Add(this.tabPage6);
            this.tabControl1.Controls.Add(this.tabPage11);
            this.tabControl1.Controls.Add(this.tabPage10);
            this.tabControl1.Controls.Add(this.tabPage7);
            this.tabControl1.Controls.Add(this.tabPage8);
            this.tabControl1.Controls.Add(this.tabPage9);
            this.tabControl1.Controls.Add(this.tabPage12);
            this.tabControl1.Controls.Add(this.tabPage13);
            this.tabControl1.Controls.Add(this.tabPage14);
            this.tabControl1.Controls.Add(this.tabPage15);
            this.tabControl1.Location = new System.Drawing.Point(12, 12);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(708, 442);
            this.tabControl1.TabIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.groupBox7);
            this.tabPage1.Controls.Add(this.groupBox2);
            this.tabPage1.Controls.Add(this.groupBox6);
            this.tabPage1.Controls.Add(this.groupBox3);
            this.tabPage1.Controls.Add(this.groupBox1);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(700, 416);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "General";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // groupBox7
            // 
            this.groupBox7.Controls.Add(this.checkBoxEnableDo);
            this.groupBox7.Controls.Add(this.checkBoxEnableMe);
            this.groupBox7.Location = new System.Drawing.Point(10, 368);
            this.groupBox7.Name = "groupBox7";
            this.groupBox7.Size = new System.Drawing.Size(337, 42);
            this.groupBox7.TabIndex = 23;
            this.groupBox7.TabStop = false;
            this.groupBox7.Text = "/me and /do";
            // 
            // checkBoxEnableDo
            // 
            this.checkBoxEnableDo.AutoSize = true;
            this.checkBoxEnableDo.Location = new System.Drawing.Point(93, 19);
            this.checkBoxEnableDo.Name = "checkBoxEnableDo";
            this.checkBoxEnableDo.Size = new System.Drawing.Size(79, 17);
            this.checkBoxEnableDo.TabIndex = 12;
            this.checkBoxEnableDo.Text = "Enable /do";
            this.checkBoxEnableDo.UseVisualStyleBackColor = true;
            // 
            // checkBoxEnableMe
            // 
            this.checkBoxEnableMe.AutoSize = true;
            this.checkBoxEnableMe.Location = new System.Drawing.Point(6, 19);
            this.checkBoxEnableMe.Name = "checkBoxEnableMe";
            this.checkBoxEnableMe.Size = new System.Drawing.Size(81, 17);
            this.checkBoxEnableMe.TabIndex = 11;
            this.checkBoxEnableMe.Text = "Enable /me";
            this.checkBoxEnableMe.UseVisualStyleBackColor = true;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.textBoxSpinImpulseMultiplier);
            this.groupBox2.Controls.Add(this.S);
            this.groupBox2.Controls.Add(this.textBoxSpinProtectMass);
            this.groupBox2.Controls.Add(this.label41);
            this.groupBox2.Location = new System.Drawing.Point(7, 287);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(340, 75);
            this.groupBox2.TabIndex = 19;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Spin control";
            // 
            // textBoxSpinImpulseMultiplier
            // 
            this.textBoxSpinImpulseMultiplier.Location = new System.Drawing.Point(163, 45);
            this.textBoxSpinImpulseMultiplier.Name = "textBoxSpinImpulseMultiplier";
            this.textBoxSpinImpulseMultiplier.Size = new System.Drawing.Size(166, 20);
            this.textBoxSpinImpulseMultiplier.TabIndex = 22;
            // 
            // S
            // 
            this.S.AutoSize = true;
            this.S.Location = new System.Drawing.Point(5, 48);
            this.S.Name = "S";
            this.S.Size = new System.Drawing.Size(109, 13);
            this.S.TabIndex = 21;
            this.S.Text = "Spin impulse multiplier";
            // 
            // textBoxSpinProtectMass
            // 
            this.textBoxSpinProtectMass.Location = new System.Drawing.Point(163, 19);
            this.textBoxSpinProtectMass.Name = "textBoxSpinProtectMass";
            this.textBoxSpinProtectMass.Size = new System.Drawing.Size(166, 20);
            this.textBoxSpinProtectMass.TabIndex = 20;
            // 
            // label41
            // 
            this.label41.AutoSize = true;
            this.label41.Location = new System.Drawing.Point(5, 22);
            this.label41.Name = "label41";
            this.label41.Size = new System.Drawing.Size(151, 13);
            this.label41.TabIndex = 19;
            this.label41.Text = "Spin protect mass (-1 disabled)";
            // 
            // groupBox6
            // 
            this.groupBox6.Controls.Add(this.textBoxHullDropFactor);
            this.groupBox6.Controls.Add(this.label21);
            this.groupBox6.Controls.Add(this.textBoxHullDrop2NickName);
            this.groupBox6.Controls.Add(this.label18);
            this.groupBox6.Controls.Add(this.textBoxHullDrop1NickName);
            this.groupBox6.Controls.Add(this.label16);
            this.groupBox6.Controls.Add(this.label17);
            this.groupBox6.Controls.Add(this.textBoxCargoDropContainer);
            this.groupBox6.Location = new System.Drawing.Point(7, 152);
            this.groupBox6.Name = "groupBox6";
            this.groupBox6.Size = new System.Drawing.Size(340, 128);
            this.groupBox6.TabIndex = 18;
            this.groupBox6.TabStop = false;
            this.groupBox6.Text = "Items to drop when ship is destroyed";
            // 
            // textBoxHullDropFactor
            // 
            this.textBoxHullDropFactor.Location = new System.Drawing.Point(163, 19);
            this.textBoxHullDropFactor.Name = "textBoxHullDropFactor";
            this.textBoxHullDropFactor.Size = new System.Drawing.Size(166, 20);
            this.textBoxHullDropFactor.TabIndex = 20;
            // 
            // label21
            // 
            this.label21.AutoSize = true;
            this.label21.Location = new System.Drawing.Point(3, 22);
            this.label21.Name = "label21";
            this.label21.Size = new System.Drawing.Size(121, 13);
            this.label21.TabIndex = 19;
            this.label21.Text = "Cargo drop hull multiplier";
            // 
            // textBoxHullDrop2NickName
            // 
            this.textBoxHullDrop2NickName.Location = new System.Drawing.Point(163, 99);
            this.textBoxHullDrop2NickName.Name = "textBoxHullDrop2NickName";
            this.textBoxHullDrop2NickName.Size = new System.Drawing.Size(166, 20);
            this.textBoxHullDrop2NickName.TabIndex = 18;
            // 
            // label18
            // 
            this.label18.AutoSize = true;
            this.label18.Location = new System.Drawing.Point(5, 102);
            this.label18.Name = "label18";
            this.label18.Size = new System.Drawing.Size(80, 13);
            this.label18.TabIndex = 17;
            this.label18.Text = "2st Item to drop";
            // 
            // textBoxHullDrop1NickName
            // 
            this.textBoxHullDrop1NickName.Location = new System.Drawing.Point(163, 72);
            this.textBoxHullDrop1NickName.Name = "textBoxHullDrop1NickName";
            this.textBoxHullDrop1NickName.Size = new System.Drawing.Size(166, 20);
            this.textBoxHullDrop1NickName.TabIndex = 16;
            // 
            // label16
            // 
            this.label16.AutoSize = true;
            this.label16.Location = new System.Drawing.Point(5, 49);
            this.label16.Name = "label16";
            this.label16.Size = new System.Drawing.Size(129, 13);
            this.label16.TabIndex = 8;
            this.label16.Text = "Container to drop cargo in";
            // 
            // label17
            // 
            this.label17.AutoSize = true;
            this.label17.Location = new System.Drawing.Point(5, 76);
            this.label17.Name = "label17";
            this.label17.Size = new System.Drawing.Size(80, 13);
            this.label17.TabIndex = 16;
            this.label17.Text = "1st Item to drop";
            // 
            // textBoxCargoDropContainer
            // 
            this.textBoxCargoDropContainer.Location = new System.Drawing.Point(163, 46);
            this.textBoxCargoDropContainer.Name = "textBoxCargoDropContainer";
            this.textBoxCargoDropContainer.Size = new System.Drawing.Size(166, 20);
            this.textBoxCargoDropContainer.TabIndex = 9;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.textBoxDisconnectingPlayersRange);
            this.groupBox3.Controls.Add(this.label34);
            this.groupBox3.Controls.Add(this.label19);
            this.groupBox3.Controls.Add(this.textBoxDisconnectMsg);
            this.groupBox3.Controls.Add(this.checkBoxReportDisconnectingPlayers);
            this.groupBox3.Controls.Add(this.checkBoxKillDisconnectingPlayers);
            this.groupBox3.Controls.Add(this.checkBoxLootDisconnectingPlayers);
            this.groupBox3.Location = new System.Drawing.Point(7, 7);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(340, 139);
            this.groupBox3.TabIndex = 17;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Action when players disconnect in range of another player";
            // 
            // textBoxDisconnectingPlayersRange
            // 
            this.textBoxDisconnectingPlayersRange.Location = new System.Drawing.Point(229, 19);
            this.textBoxDisconnectingPlayersRange.Name = "textBoxDisconnectingPlayersRange";
            this.textBoxDisconnectingPlayersRange.Size = new System.Drawing.Size(100, 20);
            this.textBoxDisconnectingPlayersRange.TabIndex = 11;
            // 
            // label34
            // 
            this.label34.AutoSize = true;
            this.label34.Location = new System.Drawing.Point(184, 21);
            this.label34.Name = "label34";
            this.label34.Size = new System.Drawing.Size(39, 13);
            this.label34.TabIndex = 10;
            this.label34.Text = "Range";
            // 
            // label19
            // 
            this.label19.AutoSize = true;
            this.label19.Location = new System.Drawing.Point(6, 90);
            this.label19.Name = "label19";
            this.label19.Size = new System.Drawing.Size(210, 13);
            this.label19.TabIndex = 9;
            this.label19.Text = "Disconnect message (tags: %time, %player)";
            // 
            // textBoxDisconnectMsg
            // 
            this.textBoxDisconnectMsg.Location = new System.Drawing.Point(6, 106);
            this.textBoxDisconnectMsg.Name = "textBoxDisconnectMsg";
            this.textBoxDisconnectMsg.Size = new System.Drawing.Size(323, 20);
            this.textBoxDisconnectMsg.TabIndex = 8;
            // 
            // checkBoxReportDisconnectingPlayers
            // 
            this.checkBoxReportDisconnectingPlayers.AutoSize = true;
            this.checkBoxReportDisconnectingPlayers.Location = new System.Drawing.Point(6, 19);
            this.checkBoxReportDisconnectingPlayers.Name = "checkBoxReportDisconnectingPlayers";
            this.checkBoxReportDisconnectingPlayers.Size = new System.Drawing.Size(58, 17);
            this.checkBoxReportDisconnectingPlayers.TabIndex = 3;
            this.checkBoxReportDisconnectingPlayers.Text = "Report";
            this.checkBoxReportDisconnectingPlayers.UseVisualStyleBackColor = true;
            // 
            // checkBoxKillDisconnectingPlayers
            // 
            this.checkBoxKillDisconnectingPlayers.AutoSize = true;
            this.checkBoxKillDisconnectingPlayers.Location = new System.Drawing.Point(6, 42);
            this.checkBoxKillDisconnectingPlayers.Name = "checkBoxKillDisconnectingPlayers";
            this.checkBoxKillDisconnectingPlayers.Size = new System.Drawing.Size(61, 17);
            this.checkBoxKillDisconnectingPlayers.TabIndex = 4;
            this.checkBoxKillDisconnectingPlayers.Text = "Kill ship";
            this.checkBoxKillDisconnectingPlayers.UseVisualStyleBackColor = true;
            // 
            // checkBoxLootDisconnectingPlayers
            // 
            this.checkBoxLootDisconnectingPlayers.AutoSize = true;
            this.checkBoxLootDisconnectingPlayers.Location = new System.Drawing.Point(6, 65);
            this.checkBoxLootDisconnectingPlayers.Name = "checkBoxLootDisconnectingPlayers";
            this.checkBoxLootDisconnectingPlayers.Size = new System.Drawing.Size(69, 17);
            this.checkBoxLootDisconnectingPlayers.TabIndex = 5;
            this.checkBoxLootDisconnectingPlayers.Text = "Loot ship";
            this.checkBoxLootDisconnectingPlayers.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.textBoxEscortedShipDamageFactor);
            this.groupBox1.Controls.Add(this.label43);
            this.groupBox1.Controls.Add(this.label42);
            this.groupBox1.Controls.Add(this.textBoxEscortedShipMinimumMass);
            this.groupBox1.Controls.Add(this.checkBoxEnableEscort);
            this.groupBox1.Controls.Add(this.label25);
            this.groupBox1.Controls.Add(this.textBoxStuckMsg);
            this.groupBox1.Controls.Add(this.label24);
            this.groupBox1.Controls.Add(this.label23);
            this.groupBox1.Controls.Add(this.textBoxSmiteMusic);
            this.groupBox1.Controls.Add(this.textBoxCoinMsg);
            this.groupBox1.Controls.Add(this.textBoxDiceMsg);
            this.groupBox1.Controls.Add(this.label22);
            this.groupBox1.Controls.Add(this.textBoxPimpShipCost);
            this.groupBox1.Controls.Add(this.label20);
            this.groupBox1.Controls.Add(this.textBoxDropRepCost);
            this.groupBox1.Controls.Add(this.checkBoxEnableDropRepAndMisc);
            this.groupBox1.Controls.Add(this.label15);
            this.groupBox1.Controls.Add(this.checkBoxEnablePimpShip);
            this.groupBox1.Location = new System.Drawing.Point(353, 7);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(341, 403);
            this.groupBox1.TabIndex = 10;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "User commands";
            // 
            // textBoxEscortedShipDamageFactor
            // 
            this.textBoxEscortedShipDamageFactor.Location = new System.Drawing.Point(169, 297);
            this.textBoxEscortedShipDamageFactor.Name = "textBoxEscortedShipDamageFactor";
            this.textBoxEscortedShipDamageFactor.Size = new System.Drawing.Size(166, 20);
            this.textBoxEscortedShipDamageFactor.TabIndex = 32;
            // 
            // label43
            // 
            this.label43.AutoSize = true;
            this.label43.Location = new System.Drawing.Point(3, 300);
            this.label43.Name = "label43";
            this.label43.Size = new System.Drawing.Size(142, 13);
            this.label43.TabIndex = 31;
            this.label43.Text = "Escorted ship damage factor";
            // 
            // label42
            // 
            this.label42.AutoSize = true;
            this.label42.Location = new System.Drawing.Point(3, 274);
            this.label42.Name = "label42";
            this.label42.Size = new System.Drawing.Size(141, 13);
            this.label42.TabIndex = 30;
            this.label42.Text = "Escorted ship minimum mass";
            // 
            // textBoxEscortedShipMinimumMass
            // 
            this.textBoxEscortedShipMinimumMass.Location = new System.Drawing.Point(169, 271);
            this.textBoxEscortedShipMinimumMass.Name = "textBoxEscortedShipMinimumMass";
            this.textBoxEscortedShipMinimumMass.Size = new System.Drawing.Size(166, 20);
            this.textBoxEscortedShipMinimumMass.TabIndex = 29;
            // 
            // checkBoxEnableEscort
            // 
            this.checkBoxEnableEscort.AutoSize = true;
            this.checkBoxEnableEscort.Location = new System.Drawing.Point(6, 248);
            this.checkBoxEnableEscort.Name = "checkBoxEnableEscort";
            this.checkBoxEnableEscort.Size = new System.Drawing.Size(96, 17);
            this.checkBoxEnableEscort.TabIndex = 28;
            this.checkBoxEnableEscort.Text = "Enable /escort";
            this.checkBoxEnableEscort.UseVisualStyleBackColor = true;
            // 
            // label25
            // 
            this.label25.AutoSize = true;
            this.label25.Location = new System.Drawing.Point(8, 99);
            this.label25.Name = "label25";
            this.label25.Size = new System.Drawing.Size(76, 13);
            this.label25.TabIndex = 22;
            this.label25.Text = "/pimpship cost";
            // 
            // textBoxStuckMsg
            // 
            this.textBoxStuckMsg.Location = new System.Drawing.Point(6, 222);
            this.textBoxStuckMsg.Name = "textBoxStuckMsg";
            this.textBoxStuckMsg.Size = new System.Drawing.Size(329, 20);
            this.textBoxStuckMsg.TabIndex = 21;
            // 
            // label24
            // 
            this.label24.AutoSize = true;
            this.label24.Location = new System.Drawing.Point(7, 72);
            this.label24.Name = "label24";
            this.label24.Size = new System.Drawing.Size(74, 13);
            this.label24.TabIndex = 20;
            this.label24.Text = ".smiteall music";
            // 
            // label23
            // 
            this.label23.AutoSize = true;
            this.label23.Location = new System.Drawing.Point(7, 165);
            this.label23.Name = "label23";
            this.label23.Size = new System.Drawing.Size(184, 13);
            this.label23.TabIndex = 19;
            this.label23.Text = "/coin message (tags: %player %result)";
            // 
            // textBoxSmiteMusic
            // 
            this.textBoxSmiteMusic.Location = new System.Drawing.Point(89, 69);
            this.textBoxSmiteMusic.Name = "textBoxSmiteMusic";
            this.textBoxSmiteMusic.Size = new System.Drawing.Size(141, 20);
            this.textBoxSmiteMusic.TabIndex = 18;
            // 
            // textBoxCoinMsg
            // 
            this.textBoxCoinMsg.Location = new System.Drawing.Point(6, 181);
            this.textBoxCoinMsg.Name = "textBoxCoinMsg";
            this.textBoxCoinMsg.Size = new System.Drawing.Size(329, 20);
            this.textBoxCoinMsg.TabIndex = 17;
            // 
            // textBoxDiceMsg
            // 
            this.textBoxDiceMsg.Location = new System.Drawing.Point(6, 142);
            this.textBoxDiceMsg.Name = "textBoxDiceMsg";
            this.textBoxDiceMsg.Size = new System.Drawing.Size(329, 20);
            this.textBoxDiceMsg.TabIndex = 16;
            // 
            // label22
            // 
            this.label22.AutoSize = true;
            this.label22.Location = new System.Drawing.Point(7, 126);
            this.label22.Name = "label22";
            this.label22.Size = new System.Drawing.Size(194, 13);
            this.label22.TabIndex = 15;
            this.label22.Text = "/dice message (tags: %player %number)";
            // 
            // textBoxPimpShipCost
            // 
            this.textBoxPimpShipCost.Location = new System.Drawing.Point(89, 96);
            this.textBoxPimpShipCost.Name = "textBoxPimpShipCost";
            this.textBoxPimpShipCost.Size = new System.Drawing.Size(78, 20);
            this.textBoxPimpShipCost.TabIndex = 14;
            // 
            // label20
            // 
            this.label20.AutoSize = true;
            this.label20.Location = new System.Drawing.Point(7, 206);
            this.label20.Name = "label20";
            this.label20.Size = new System.Drawing.Size(83, 13);
            this.label20.TabIndex = 13;
            this.label20.Text = "/stuck message";
            // 
            // textBoxDropRepCost
            // 
            this.textBoxDropRepCost.Location = new System.Drawing.Point(250, 96);
            this.textBoxDropRepCost.Name = "textBoxDropRepCost";
            this.textBoxDropRepCost.Size = new System.Drawing.Size(72, 20);
            this.textBoxDropRepCost.TabIndex = 7;
            // 
            // checkBoxEnableDropRepAndMisc
            // 
            this.checkBoxEnableDropRepAndMisc.AutoSize = true;
            this.checkBoxEnableDropRepAndMisc.Location = new System.Drawing.Point(6, 22);
            this.checkBoxEnableDropRepAndMisc.Name = "checkBoxEnableDropRepAndMisc";
            this.checkBoxEnableDropRepAndMisc.Size = new System.Drawing.Size(230, 17);
            this.checkBoxEnableDropRepAndMisc.TabIndex = 12;
            this.checkBoxEnableDropRepAndMisc.Text = "Enable /pos, /dice, /coin, /stuck, /droprep";
            this.checkBoxEnableDropRepAndMisc.UseVisualStyleBackColor = true;
            // 
            // label15
            // 
            this.label15.AutoSize = true;
            this.label15.Location = new System.Drawing.Point(173, 99);
            this.label15.Name = "label15";
            this.label15.Size = new System.Drawing.Size(71, 13);
            this.label15.TabIndex = 6;
            this.label15.Text = "/droprep cost";
            // 
            // checkBoxEnablePimpShip
            // 
            this.checkBoxEnablePimpShip.AutoSize = true;
            this.checkBoxEnablePimpShip.Location = new System.Drawing.Point(6, 47);
            this.checkBoxEnablePimpShip.Name = "checkBoxEnablePimpShip";
            this.checkBoxEnablePimpShip.Size = new System.Drawing.Size(260, 17);
            this.checkBoxEnablePimpShip.TabIndex = 9;
            this.checkBoxEnablePimpShip.Text = "Enable /pimpship, /showsetup, /showitems, etc...";
            this.checkBoxEnablePimpShip.UseVisualStyleBackColor = true;
            // 
            // tabPage5
            // 
            this.tabPage5.Controls.Add(this.groupBox4);
            this.tabPage5.Controls.Add(this.groupBox5);
            this.tabPage5.Location = new System.Drawing.Point(4, 22);
            this.tabPage5.Name = "tabPage5";
            this.tabPage5.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage5.Size = new System.Drawing.Size(700, 416);
            this.tabPage5.TabIndex = 5;
            this.tabPage5.Text = "Rename/Givecash";
            this.tabPage5.UseVisualStyleBackColor = true;
            // 
            // groupBox4
            // 
            this.groupBox4.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox4.Controls.Add(this.checkBoxEnableGiveCash);
            this.groupBox4.Controls.Add(this.button3);
            this.groupBox4.Controls.Add(this.textBoxBlockedSystem);
            this.groupBox4.Controls.Add(this.label8);
            this.groupBox4.Controls.Add(this.label7);
            this.groupBox4.Controls.Add(this.label6);
            this.groupBox4.Controls.Add(this.textBoxMinTime);
            this.groupBox4.Controls.Add(this.textBoxMinTransfer);
            this.groupBox4.Controls.Add(this.label5);
            this.groupBox4.Controls.Add(this.label4);
            this.groupBox4.Location = new System.Drawing.Point(6, 159);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(688, 120);
            this.groupBox4.TabIndex = 19;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Cash commands";
            // 
            // checkBoxEnableGiveCash
            // 
            this.checkBoxEnableGiveCash.AutoSize = true;
            this.checkBoxEnableGiveCash.Location = new System.Drawing.Point(6, 19);
            this.checkBoxEnableGiveCash.Name = "checkBoxEnableGiveCash";
            this.checkBoxEnableGiveCash.Size = new System.Drawing.Size(301, 17);
            this.checkBoxEnableGiveCash.TabIndex = 25;
            this.checkBoxEnableGiveCash.Text = "Enable /givecash, /drawcash, /showcash, /set cashcode";
            this.checkBoxEnableGiveCash.UseVisualStyleBackColor = true;
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(318, 88);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(55, 23);
            this.button3.TabIndex = 23;
            this.button3.Text = "Change";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.Button3_Click);
            // 
            // textBoxBlockedSystem
            // 
            this.textBoxBlockedSystem.Location = new System.Drawing.Point(185, 90);
            this.textBoxBlockedSystem.Name = "textBoxBlockedSystem";
            this.textBoxBlockedSystem.Size = new System.Drawing.Size(126, 20);
            this.textBoxBlockedSystem.TabIndex = 22;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(3, 93);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(176, 13);
            this.label8.TabIndex = 21;
            this.label8.Text = "Prohibit transfers to/from this system";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(217, 43);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(38, 13);
            this.label7.TabIndex = 20;
            this.label7.Text = "credits";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(332, 67);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(47, 13);
            this.label6.TabIndex = 19;
            this.label6.Text = "seconds";
            // 
            // textBoxMinTime
            // 
            this.textBoxMinTime.Location = new System.Drawing.Point(254, 64);
            this.textBoxMinTime.Name = "textBoxMinTime";
            this.textBoxMinTime.Size = new System.Drawing.Size(72, 20);
            this.textBoxMinTime.TabIndex = 18;
            // 
            // textBoxMinTransfer
            // 
            this.textBoxMinTransfer.Location = new System.Drawing.Point(139, 40);
            this.textBoxMinTransfer.Name = "textBoxMinTransfer";
            this.textBoxMinTransfer.Size = new System.Drawing.Size(72, 20);
            this.textBoxMinTransfer.TabIndex = 17;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(3, 67);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(245, 13);
            this.label5.TabIndex = 16;
            this.label5.Text = "Prohibit transfers on characters active for less than";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(3, 43);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(130, 13);
            this.label4.TabIndex = 15;
            this.label4.Text = "Prohibit transfers less than";
            // 
            // groupBox5
            // 
            this.groupBox5.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox5.Controls.Add(this.checkBoxAsciiCharnameOnly);
            this.groupBox5.Controls.Add(this.label46);
            this.groupBox5.Controls.Add(this.textBoxCharnameTagCost);
            this.groupBox5.Controls.Add(this.label45);
            this.groupBox5.Controls.Add(this.checkBoxCharnameTag);
            this.groupBox5.Controls.Add(this.checkBoxEnableMoveChar);
            this.groupBox5.Controls.Add(this.label12);
            this.groupBox5.Controls.Add(this.textBoxRenameTimeLimit);
            this.groupBox5.Controls.Add(this.label14);
            this.groupBox5.Controls.Add(this.label9);
            this.groupBox5.Controls.Add(this.textBoxMoveCost);
            this.groupBox5.Controls.Add(this.label11);
            this.groupBox5.Controls.Add(this.checkBoxEnableRenameMe);
            this.groupBox5.Controls.Add(this.label10);
            this.groupBox5.Controls.Add(this.textBoxRenameCost);
            this.groupBox5.Controls.Add(this.label13);
            this.groupBox5.Location = new System.Drawing.Point(8, 6);
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.Size = new System.Drawing.Size(686, 147);
            this.groupBox5.TabIndex = 18;
            this.groupBox5.TabStop = false;
            this.groupBox5.Text = "Rename commands";
            // 
            // checkBoxAsciiCharnameOnly
            // 
            this.checkBoxAsciiCharnameOnly.AutoSize = true;
            this.checkBoxAsciiCharnameOnly.Location = new System.Drawing.Point(330, 68);
            this.checkBoxAsciiCharnameOnly.Name = "checkBoxAsciiCharnameOnly";
            this.checkBoxAsciiCharnameOnly.Size = new System.Drawing.Size(157, 17);
            this.checkBoxAsciiCharnameOnly.TabIndex = 37;
            this.checkBoxAsciiCharnameOnly.Text = "Prohibit unicode charnames";
            this.checkBoxAsciiCharnameOnly.UseVisualStyleBackColor = true;
            // 
            // label46
            // 
            this.label46.AutoSize = true;
            this.label46.Location = new System.Drawing.Point(573, 43);
            this.label46.Name = "label46";
            this.label46.Size = new System.Drawing.Size(38, 13);
            this.label46.TabIndex = 36;
            this.label46.Text = "credits";
            // 
            // textBoxCharnameTagCost
            // 
            this.textBoxCharnameTagCost.Location = new System.Drawing.Point(482, 40);
            this.textBoxCharnameTagCost.Name = "textBoxCharnameTagCost";
            this.textBoxCharnameTagCost.Size = new System.Drawing.Size(72, 20);
            this.textBoxCharnameTagCost.TabIndex = 35;
            // 
            // label45
            // 
            this.label45.AutoSize = true;
            this.label45.Location = new System.Drawing.Point(330, 43);
            this.label45.Name = "label45";
            this.label45.Size = new System.Drawing.Size(138, 13);
            this.label45.TabIndex = 34;
            this.label45.Text = "Cost of /maketag command";
            // 
            // checkBoxCharnameTag
            // 
            this.checkBoxCharnameTag.AutoSize = true;
            this.checkBoxCharnameTag.Location = new System.Drawing.Point(333, 19);
            this.checkBoxCharnameTag.Name = "checkBoxCharnameTag";
            this.checkBoxCharnameTag.Size = new System.Drawing.Size(281, 17);
            this.checkBoxCharnameTag.TabIndex = 33;
            this.checkBoxCharnameTag.Text = "Enable owned tags (/maketag, /settagpass, /droptag)";
            this.checkBoxCharnameTag.UseVisualStyleBackColor = true;
            // 
            // checkBoxEnableMoveChar
            // 
            this.checkBoxEnableMoveChar.AutoSize = true;
            this.checkBoxEnableMoveChar.Location = new System.Drawing.Point(4, 42);
            this.checkBoxEnableMoveChar.Name = "checkBoxEnableMoveChar";
            this.checkBoxEnableMoveChar.Size = new System.Drawing.Size(213, 17);
            this.checkBoxEnableMoveChar.TabIndex = 32;
            this.checkBoxEnableMoveChar.Text = "Enable /movechar, /set movecharcode";
            this.checkBoxEnableMoveChar.UseVisualStyleBackColor = true;
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(178, 121);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(47, 13);
            this.label12.TabIndex = 31;
            this.label12.Text = "seconds";
            // 
            // textBoxRenameTimeLimit
            // 
            this.textBoxRenameTimeLimit.Location = new System.Drawing.Point(100, 118);
            this.textBoxRenameTimeLimit.Name = "textBoxRenameTimeLimit";
            this.textBoxRenameTimeLimit.Size = new System.Drawing.Size(72, 20);
            this.textBoxRenameTimeLimit.TabIndex = 30;
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(3, 121);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(89, 13);
            this.label14.TabIndex = 29;
            this.label14.Text = "Rename time limit";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(231, 94);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(38, 13);
            this.label9.TabIndex = 28;
            this.label9.Text = "credits";
            // 
            // textBoxMoveCost
            // 
            this.textBoxMoveCost.Location = new System.Drawing.Point(153, 91);
            this.textBoxMoveCost.Name = "textBoxMoveCost";
            this.textBoxMoveCost.Size = new System.Drawing.Size(72, 20);
            this.textBoxMoveCost.TabIndex = 27;
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(5, 68);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(146, 13);
            this.label11.TabIndex = 26;
            this.label11.Text = "Cost of /renameme command";
            // 
            // checkBoxEnableRenameMe
            // 
            this.checkBoxEnableRenameMe.AutoSize = true;
            this.checkBoxEnableRenameMe.Location = new System.Drawing.Point(6, 19);
            this.checkBoxEnableRenameMe.Name = "checkBoxEnableRenameMe";
            this.checkBoxEnableRenameMe.Size = new System.Drawing.Size(116, 17);
            this.checkBoxEnableRenameMe.TabIndex = 24;
            this.checkBoxEnableRenameMe.Text = "Enable /renameme";
            this.checkBoxEnableRenameMe.UseVisualStyleBackColor = true;
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(231, 68);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(38, 13);
            this.label10.TabIndex = 20;
            this.label10.Text = "credits";
            // 
            // textBoxRenameCost
            // 
            this.textBoxRenameCost.Location = new System.Drawing.Point(153, 65);
            this.textBoxRenameCost.Name = "textBoxRenameCost";
            this.textBoxRenameCost.Size = new System.Drawing.Size(72, 20);
            this.textBoxRenameCost.TabIndex = 17;
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(3, 94);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(144, 13);
            this.label13.TabIndex = 15;
            this.label13.Text = "Cost of /movechar command";
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.buttonDelItem);
            this.tabPage2.Controls.Add(this.buttonAddItem);
            this.tabPage2.Controls.Add(this.label2);
            this.tabPage2.Controls.Add(this.dataGridViewShipPimperEquipList);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(700, 416);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Ship Pimper - Items";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // buttonDelItem
            // 
            this.buttonDelItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonDelItem.Location = new System.Drawing.Point(87, 387);
            this.buttonDelItem.Name = "buttonDelItem";
            this.buttonDelItem.Size = new System.Drawing.Size(75, 23);
            this.buttonDelItem.TabIndex = 22;
            this.buttonDelItem.Text = "Delete Item";
            this.buttonDelItem.UseVisualStyleBackColor = true;
            this.buttonDelItem.Click += new System.EventHandler(this.ButtonDelItem_Click);
            // 
            // buttonAddItem
            // 
            this.buttonAddItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonAddItem.Location = new System.Drawing.Point(6, 387);
            this.buttonAddItem.Name = "buttonAddItem";
            this.buttonAddItem.Size = new System.Drawing.Size(75, 23);
            this.buttonAddItem.TabIndex = 3;
            this.buttonAddItem.Text = "Add Item";
            this.buttonAddItem.UseVisualStyleBackColor = true;
            this.buttonAddItem.Click += new System.EventHandler(this.ButtonAddItem_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 3);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(229, 13);
            this.label2.TabIndex = 19;
            this.label2.Text = "The following items may be changed by players";
            // 
            // dataGridViewShipPimperEquipList
            // 
            this.dataGridViewShipPimperEquipList.AllowUserToAddRows = false;
            this.dataGridViewShipPimperEquipList.AllowUserToDeleteRows = false;
            this.dataGridViewShipPimperEquipList.AllowUserToResizeRows = false;
            this.dataGridViewShipPimperEquipList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewShipPimperEquipList.AutoGenerateColumns = false;
            this.dataGridViewShipPimperEquipList.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewShipPimperEquipList.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.iDSNameDataGridViewTextBoxColumn1,
            this.itemNickNameDataGridViewTextBoxColumn1,
            this.itemHashDataGridViewTextBoxColumn1,
            this.iDSInfoDataGridViewTextBoxColumn1,
            this.iDSInfo1DataGridViewTextBoxColumn1});
            this.dataGridViewShipPimperEquipList.DataSource = this.shipPimperItemListBindingSource;
            this.dataGridViewShipPimperEquipList.Location = new System.Drawing.Point(6, 19);
            this.dataGridViewShipPimperEquipList.Name = "dataGridViewShipPimperEquipList";
            this.dataGridViewShipPimperEquipList.ReadOnly = true;
            this.dataGridViewShipPimperEquipList.RowHeadersVisible = false;
            this.dataGridViewShipPimperEquipList.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridViewShipPimperEquipList.Size = new System.Drawing.Size(688, 362);
            this.dataGridViewShipPimperEquipList.TabIndex = 18;
            // 
            // iDSNameDataGridViewTextBoxColumn1
            // 
            this.iDSNameDataGridViewTextBoxColumn1.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.iDSNameDataGridViewTextBoxColumn1.DataPropertyName = "IDSName";
            this.iDSNameDataGridViewTextBoxColumn1.HeaderText = "Description";
            this.iDSNameDataGridViewTextBoxColumn1.Name = "iDSNameDataGridViewTextBoxColumn1";
            this.iDSNameDataGridViewTextBoxColumn1.ReadOnly = true;
            // 
            // itemNickNameDataGridViewTextBoxColumn1
            // 
            this.itemNickNameDataGridViewTextBoxColumn1.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.itemNickNameDataGridViewTextBoxColumn1.DataPropertyName = "ItemNickName";
            this.itemNickNameDataGridViewTextBoxColumn1.HeaderText = "NickName";
            this.itemNickNameDataGridViewTextBoxColumn1.Name = "itemNickNameDataGridViewTextBoxColumn1";
            this.itemNickNameDataGridViewTextBoxColumn1.ReadOnly = true;
            // 
            // itemHashDataGridViewTextBoxColumn1
            // 
            this.itemHashDataGridViewTextBoxColumn1.DataPropertyName = "ItemHash";
            this.itemHashDataGridViewTextBoxColumn1.HeaderText = "ItemHash";
            this.itemHashDataGridViewTextBoxColumn1.Name = "itemHashDataGridViewTextBoxColumn1";
            this.itemHashDataGridViewTextBoxColumn1.ReadOnly = true;
            this.itemHashDataGridViewTextBoxColumn1.Visible = false;
            // 
            // iDSInfoDataGridViewTextBoxColumn1
            // 
            this.iDSInfoDataGridViewTextBoxColumn1.DataPropertyName = "IDSInfo";
            this.iDSInfoDataGridViewTextBoxColumn1.HeaderText = "IDSInfo";
            this.iDSInfoDataGridViewTextBoxColumn1.Name = "iDSInfoDataGridViewTextBoxColumn1";
            this.iDSInfoDataGridViewTextBoxColumn1.ReadOnly = true;
            this.iDSInfoDataGridViewTextBoxColumn1.Visible = false;
            // 
            // iDSInfo1DataGridViewTextBoxColumn1
            // 
            this.iDSInfo1DataGridViewTextBoxColumn1.DataPropertyName = "IDSInfo1";
            this.iDSInfo1DataGridViewTextBoxColumn1.HeaderText = "IDSInfo1";
            this.iDSInfo1DataGridViewTextBoxColumn1.Name = "iDSInfo1DataGridViewTextBoxColumn1";
            this.iDSInfo1DataGridViewTextBoxColumn1.ReadOnly = true;
            this.iDSInfo1DataGridViewTextBoxColumn1.Visible = false;
            // 
            // shipPimperItemListBindingSource
            // 
            this.shipPimperItemListBindingSource.DataMember = "ShipPimperItemList";
            this.shipPimperItemListBindingSource.DataSource = this.uIDataSet;
            // 
            // uIDataSet
            // 
            this.uIDataSet.DataSetName = "UIDataSet";
            this.uIDataSet.SchemaSerializationMode = System.Data.SchemaSerializationMode.IncludeSchema;
            // 
            // tabPage4
            // 
            this.tabPage4.Controls.Add(this.buttonDelBase);
            this.tabPage4.Controls.Add(this.buttonAddBase);
            this.tabPage4.Controls.Add(this.label3);
            this.tabPage4.Controls.Add(this.dataGridViewShipPimperBases);
            this.tabPage4.Location = new System.Drawing.Point(4, 22);
            this.tabPage4.Name = "tabPage4";
            this.tabPage4.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage4.Size = new System.Drawing.Size(700, 416);
            this.tabPage4.TabIndex = 3;
            this.tabPage4.Text = "Ship Pimper - Rooms";
            this.tabPage4.UseVisualStyleBackColor = true;
            // 
            // buttonDelBase
            // 
            this.buttonDelBase.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonDelBase.Location = new System.Drawing.Point(90, 387);
            this.buttonDelBase.Name = "buttonDelBase";
            this.buttonDelBase.Size = new System.Drawing.Size(75, 23);
            this.buttonDelBase.TabIndex = 28;
            this.buttonDelBase.Text = "Delete Base";
            this.buttonDelBase.UseVisualStyleBackColor = true;
            this.buttonDelBase.Click += new System.EventHandler(this.ButtonDelBase_Click);
            // 
            // buttonAddBase
            // 
            this.buttonAddBase.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonAddBase.Location = new System.Drawing.Point(9, 387);
            this.buttonAddBase.Name = "buttonAddBase";
            this.buttonAddBase.Size = new System.Drawing.Size(75, 23);
            this.buttonAddBase.TabIndex = 27;
            this.buttonAddBase.Text = "Add Base";
            this.buttonAddBase.UseVisualStyleBackColor = true;
            this.buttonAddBase.Click += new System.EventHandler(this.ButtonAddBase_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(6, 3);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(195, 13);
            this.label3.TabIndex = 26;
            this.label3.Text = "Ship pimping is permitted at these rooms";
            // 
            // dataGridViewShipPimperBases
            // 
            this.dataGridViewShipPimperBases.AllowUserToAddRows = false;
            this.dataGridViewShipPimperBases.AllowUserToDeleteRows = false;
            this.dataGridViewShipPimperBases.AllowUserToResizeRows = false;
            this.dataGridViewShipPimperBases.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewShipPimperBases.AutoGenerateColumns = false;
            this.dataGridViewShipPimperBases.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewShipPimperBases.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.iDSNameDataGridViewTextBoxColumn,
            this.itemNickNameDataGridViewTextBoxColumn,
            this.itemHashDataGridViewTextBoxColumn,
            this.iDSInfoDataGridViewTextBoxColumn,
            this.iDSInfo1DataGridViewTextBoxColumn});
            this.dataGridViewShipPimperBases.DataSource = this.shipPimperBaseListBindingSource;
            this.dataGridViewShipPimperBases.Location = new System.Drawing.Point(6, 20);
            this.dataGridViewShipPimperBases.Name = "dataGridViewShipPimperBases";
            this.dataGridViewShipPimperBases.ReadOnly = true;
            this.dataGridViewShipPimperBases.RowHeadersVisible = false;
            this.dataGridViewShipPimperBases.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridViewShipPimperBases.Size = new System.Drawing.Size(688, 361);
            this.dataGridViewShipPimperBases.TabIndex = 25;
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
            // itemHashDataGridViewTextBoxColumn
            // 
            this.itemHashDataGridViewTextBoxColumn.DataPropertyName = "ItemHash";
            this.itemHashDataGridViewTextBoxColumn.HeaderText = "ItemHash";
            this.itemHashDataGridViewTextBoxColumn.Name = "itemHashDataGridViewTextBoxColumn";
            this.itemHashDataGridViewTextBoxColumn.ReadOnly = true;
            this.itemHashDataGridViewTextBoxColumn.Visible = false;
            // 
            // iDSInfoDataGridViewTextBoxColumn
            // 
            this.iDSInfoDataGridViewTextBoxColumn.DataPropertyName = "IDSInfo";
            this.iDSInfoDataGridViewTextBoxColumn.HeaderText = "IDSInfo";
            this.iDSInfoDataGridViewTextBoxColumn.Name = "iDSInfoDataGridViewTextBoxColumn";
            this.iDSInfoDataGridViewTextBoxColumn.ReadOnly = true;
            this.iDSInfoDataGridViewTextBoxColumn.Visible = false;
            // 
            // iDSInfo1DataGridViewTextBoxColumn
            // 
            this.iDSInfo1DataGridViewTextBoxColumn.DataPropertyName = "IDSInfo1";
            this.iDSInfo1DataGridViewTextBoxColumn.HeaderText = "IDSInfo1";
            this.iDSInfo1DataGridViewTextBoxColumn.Name = "iDSInfo1DataGridViewTextBoxColumn";
            this.iDSInfo1DataGridViewTextBoxColumn.ReadOnly = true;
            this.iDSInfo1DataGridViewTextBoxColumn.Visible = false;
            // 
            // shipPimperBaseListBindingSource
            // 
            this.shipPimperBaseListBindingSource.DataMember = "ShipPimperBaseList";
            this.shipPimperBaseListBindingSource.DataSource = this.uIDataSet;
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.buttonRepDelItem);
            this.tabPage3.Controls.Add(this.buttonRepEditItem);
            this.tabPage3.Controls.Add(this.buttonRepAddItem);
            this.tabPage3.Controls.Add(this.label1);
            this.tabPage3.Controls.Add(this.dataGridViewRepFixerItems);
            this.tabPage3.Controls.Add(this.checkBoxEnableRepFixUpdates);
            this.tabPage3.Controls.Add(this.checkBoxLogRepFixUpdates);
            this.tabPage3.Controls.Add(this.checkBoxItemMustBeMounted);
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage3.Size = new System.Drawing.Size(700, 416);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Reputation Fixer";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // buttonRepDelItem
            // 
            this.buttonRepDelItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonRepDelItem.Location = new System.Drawing.Point(168, 387);
            this.buttonRepDelItem.Name = "buttonRepDelItem";
            this.buttonRepDelItem.Size = new System.Drawing.Size(75, 23);
            this.buttonRepDelItem.TabIndex = 20;
            this.buttonRepDelItem.Text = "Del Item";
            this.buttonRepDelItem.UseVisualStyleBackColor = true;
            this.buttonRepDelItem.Click += new System.EventHandler(this.ButtonRepDelItem_Click);
            // 
            // buttonRepEditItem
            // 
            this.buttonRepEditItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonRepEditItem.Location = new System.Drawing.Point(87, 387);
            this.buttonRepEditItem.Name = "buttonRepEditItem";
            this.buttonRepEditItem.Size = new System.Drawing.Size(75, 23);
            this.buttonRepEditItem.TabIndex = 19;
            this.buttonRepEditItem.Text = "Edit Item";
            this.buttonRepEditItem.UseVisualStyleBackColor = true;
            this.buttonRepEditItem.Click += new System.EventHandler(this.ButtonEditRep_Click);
            // 
            // buttonRepAddItem
            // 
            this.buttonRepAddItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonRepAddItem.Location = new System.Drawing.Point(9, 387);
            this.buttonRepAddItem.Name = "buttonRepAddItem";
            this.buttonRepAddItem.Size = new System.Drawing.Size(75, 23);
            this.buttonRepAddItem.TabIndex = 18;
            this.buttonRepAddItem.Text = "Add Item";
            this.buttonRepAddItem.UseVisualStyleBackColor = true;
            this.buttonRepAddItem.Click += new System.EventHandler(this.ButtonAddRepItem_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 26);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(272, 13);
            this.label1.TabIndex = 17;
            this.label1.Text = "Adjust reputations on the presence of the following items";
            // 
            // dataGridViewRepFixerItems
            // 
            this.dataGridViewRepFixerItems.AllowUserToAddRows = false;
            this.dataGridViewRepFixerItems.AllowUserToDeleteRows = false;
            this.dataGridViewRepFixerItems.AllowUserToResizeRows = false;
            this.dataGridViewRepFixerItems.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewRepFixerItems.AutoGenerateColumns = false;
            this.dataGridViewRepFixerItems.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewRepFixerItems.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.iDSNameDataGridViewTextBoxColumn2,
            this.itemNickNameDataGridViewTextBoxColumn2,
            this.itemHashDataGridViewTextBoxColumn2,
            this.iDSInfoDataGridViewTextBoxColumn2,
            this.iDSInfo1DataGridViewTextBoxColumn2});
            this.dataGridViewRepFixerItems.DataSource = this.repFixerItemListBindingSource;
            this.dataGridViewRepFixerItems.Location = new System.Drawing.Point(6, 42);
            this.dataGridViewRepFixerItems.Name = "dataGridViewRepFixerItems";
            this.dataGridViewRepFixerItems.ReadOnly = true;
            this.dataGridViewRepFixerItems.RowHeadersVisible = false;
            this.dataGridViewRepFixerItems.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridViewRepFixerItems.Size = new System.Drawing.Size(688, 339);
            this.dataGridViewRepFixerItems.TabIndex = 16;
            this.dataGridViewRepFixerItems.CellDoubleClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.DataGridViewRepFixerItems_CellDoubleClick);
            // 
            // iDSNameDataGridViewTextBoxColumn2
            // 
            this.iDSNameDataGridViewTextBoxColumn2.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.iDSNameDataGridViewTextBoxColumn2.DataPropertyName = "IDSName";
            this.iDSNameDataGridViewTextBoxColumn2.HeaderText = "Description";
            this.iDSNameDataGridViewTextBoxColumn2.Name = "iDSNameDataGridViewTextBoxColumn2";
            this.iDSNameDataGridViewTextBoxColumn2.ReadOnly = true;
            // 
            // itemNickNameDataGridViewTextBoxColumn2
            // 
            this.itemNickNameDataGridViewTextBoxColumn2.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.itemNickNameDataGridViewTextBoxColumn2.DataPropertyName = "ItemNickName";
            this.itemNickNameDataGridViewTextBoxColumn2.HeaderText = "NickName";
            this.itemNickNameDataGridViewTextBoxColumn2.Name = "itemNickNameDataGridViewTextBoxColumn2";
            this.itemNickNameDataGridViewTextBoxColumn2.ReadOnly = true;
            // 
            // itemHashDataGridViewTextBoxColumn2
            // 
            this.itemHashDataGridViewTextBoxColumn2.DataPropertyName = "ItemHash";
            this.itemHashDataGridViewTextBoxColumn2.HeaderText = "ItemHash";
            this.itemHashDataGridViewTextBoxColumn2.Name = "itemHashDataGridViewTextBoxColumn2";
            this.itemHashDataGridViewTextBoxColumn2.ReadOnly = true;
            this.itemHashDataGridViewTextBoxColumn2.Visible = false;
            // 
            // iDSInfoDataGridViewTextBoxColumn2
            // 
            this.iDSInfoDataGridViewTextBoxColumn2.DataPropertyName = "IDSInfo";
            this.iDSInfoDataGridViewTextBoxColumn2.HeaderText = "IDSInfo";
            this.iDSInfoDataGridViewTextBoxColumn2.Name = "iDSInfoDataGridViewTextBoxColumn2";
            this.iDSInfoDataGridViewTextBoxColumn2.ReadOnly = true;
            this.iDSInfoDataGridViewTextBoxColumn2.Visible = false;
            // 
            // iDSInfo1DataGridViewTextBoxColumn2
            // 
            this.iDSInfo1DataGridViewTextBoxColumn2.DataPropertyName = "IDSInfo1";
            this.iDSInfo1DataGridViewTextBoxColumn2.HeaderText = "IDSInfo1";
            this.iDSInfo1DataGridViewTextBoxColumn2.Name = "iDSInfo1DataGridViewTextBoxColumn2";
            this.iDSInfo1DataGridViewTextBoxColumn2.ReadOnly = true;
            this.iDSInfo1DataGridViewTextBoxColumn2.Visible = false;
            // 
            // repFixerItemListBindingSource
            // 
            this.repFixerItemListBindingSource.DataMember = "RepFixerItemList";
            this.repFixerItemListBindingSource.DataSource = this.uIDataSet;
            // 
            // checkBoxEnableRepFixUpdates
            // 
            this.checkBoxEnableRepFixUpdates.AutoSize = true;
            this.checkBoxEnableRepFixUpdates.Location = new System.Drawing.Point(6, 6);
            this.checkBoxEnableRepFixUpdates.Name = "checkBoxEnableRepFixUpdates";
            this.checkBoxEnableRepFixUpdates.Size = new System.Drawing.Size(204, 17);
            this.checkBoxEnableRepFixUpdates.TabIndex = 15;
            this.checkBoxEnableRepFixUpdates.Text = "Enable reputation check and updates";
            this.checkBoxEnableRepFixUpdates.UseVisualStyleBackColor = true;
            // 
            // checkBoxLogRepFixUpdates
            // 
            this.checkBoxLogRepFixUpdates.AutoSize = true;
            this.checkBoxLogRepFixUpdates.Location = new System.Drawing.Point(216, 6);
            this.checkBoxLogRepFixUpdates.Name = "checkBoxLogRepFixUpdates";
            this.checkBoxLogRepFixUpdates.Size = new System.Drawing.Size(135, 17);
            this.checkBoxLogRepFixUpdates.TabIndex = 14;
            this.checkBoxLogRepFixUpdates.Text = "Log reputation updates";
            this.checkBoxLogRepFixUpdates.UseVisualStyleBackColor = true;
            // 
            // checkBoxItemMustBeMounted
            // 
            this.checkBoxItemMustBeMounted.AutoSize = true;
            this.checkBoxItemMustBeMounted.Location = new System.Drawing.Point(357, 6);
            this.checkBoxItemMustBeMounted.Name = "checkBoxItemMustBeMounted";
            this.checkBoxItemMustBeMounted.Size = new System.Drawing.Size(220, 17);
            this.checkBoxItemMustBeMounted.TabIndex = 0;
            this.checkBoxItemMustBeMounted.Text = "Reputation update only if item is mounted";
            this.checkBoxItemMustBeMounted.UseVisualStyleBackColor = true;
            // 
            // tabPage6
            // 
            this.tabPage6.Controls.Add(this.checkBoxCheckIDRestrictions);
            this.tabPage6.Controls.Add(this.buttonImportShipPurchaseRestrictions);
            this.tabPage6.Controls.Add(this.buttonPurchaseRestrictionDelItem);
            this.tabPage6.Controls.Add(this.buttonPurchaseRestrictionEdittem);
            this.tabPage6.Controls.Add(this.buttonPurchaseRestrictionAddItem);
            this.tabPage6.Controls.Add(this.label26);
            this.tabPage6.Controls.Add(this.dataGridViewPurchaseRestrictions);
            this.tabPage6.Controls.Add(this.checkBoxEnforceIDRestrictions);
            this.tabPage6.Location = new System.Drawing.Point(4, 22);
            this.tabPage6.Name = "tabPage6";
            this.tabPage6.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage6.Size = new System.Drawing.Size(700, 416);
            this.tabPage6.TabIndex = 6;
            this.tabPage6.Text = "Ship Purchase Restriction";
            this.tabPage6.UseVisualStyleBackColor = true;
            // 
            // checkBoxCheckIDRestrictions
            // 
            this.checkBoxCheckIDRestrictions.AutoSize = true;
            this.checkBoxCheckIDRestrictions.Location = new System.Drawing.Point(9, 6);
            this.checkBoxCheckIDRestrictions.Name = "checkBoxCheckIDRestrictions";
            this.checkBoxCheckIDRestrictions.Size = new System.Drawing.Size(195, 17);
            this.checkBoxCheckIDRestrictions.TabIndex = 25;
            this.checkBoxCheckIDRestrictions.Text = "Check and log purchase restrictions";
            this.checkBoxCheckIDRestrictions.UseVisualStyleBackColor = true;
            // 
            // buttonImportShipPurchaseRestrictions
            // 
            this.buttonImportShipPurchaseRestrictions.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonImportShipPurchaseRestrictions.Location = new System.Drawing.Point(619, 387);
            this.buttonImportShipPurchaseRestrictions.Name = "buttonImportShipPurchaseRestrictions";
            this.buttonImportShipPurchaseRestrictions.Size = new System.Drawing.Size(75, 23);
            this.buttonImportShipPurchaseRestrictions.TabIndex = 23;
            this.buttonImportShipPurchaseRestrictions.Text = "Import";
            this.buttonImportShipPurchaseRestrictions.UseVisualStyleBackColor = true;
            this.buttonImportShipPurchaseRestrictions.Click += new System.EventHandler(this.ButtonImportShipPurchaseRestrictions_Click);
            // 
            // buttonPurchaseRestrictionDelItem
            // 
            this.buttonPurchaseRestrictionDelItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonPurchaseRestrictionDelItem.Location = new System.Drawing.Point(171, 387);
            this.buttonPurchaseRestrictionDelItem.Name = "buttonPurchaseRestrictionDelItem";
            this.buttonPurchaseRestrictionDelItem.Size = new System.Drawing.Size(75, 23);
            this.buttonPurchaseRestrictionDelItem.TabIndex = 24;
            this.buttonPurchaseRestrictionDelItem.Text = "Del Item";
            this.buttonPurchaseRestrictionDelItem.UseVisualStyleBackColor = true;
            this.buttonPurchaseRestrictionDelItem.Click += new System.EventHandler(this.ButtonPurchaseRestrictionDelItem_Click);
            // 
            // buttonPurchaseRestrictionEdittem
            // 
            this.buttonPurchaseRestrictionEdittem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonPurchaseRestrictionEdittem.Location = new System.Drawing.Point(90, 387);
            this.buttonPurchaseRestrictionEdittem.Name = "buttonPurchaseRestrictionEdittem";
            this.buttonPurchaseRestrictionEdittem.Size = new System.Drawing.Size(75, 23);
            this.buttonPurchaseRestrictionEdittem.TabIndex = 23;
            this.buttonPurchaseRestrictionEdittem.Text = "Edit Item";
            this.buttonPurchaseRestrictionEdittem.UseVisualStyleBackColor = true;
            this.buttonPurchaseRestrictionEdittem.Click += new System.EventHandler(this.ButtonPurchaseRestrictionEditItem_Click);
            // 
            // buttonPurchaseRestrictionAddItem
            // 
            this.buttonPurchaseRestrictionAddItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonPurchaseRestrictionAddItem.Location = new System.Drawing.Point(9, 387);
            this.buttonPurchaseRestrictionAddItem.Name = "buttonPurchaseRestrictionAddItem";
            this.buttonPurchaseRestrictionAddItem.Size = new System.Drawing.Size(75, 23);
            this.buttonPurchaseRestrictionAddItem.TabIndex = 22;
            this.buttonPurchaseRestrictionAddItem.Text = "Add Item";
            this.buttonPurchaseRestrictionAddItem.UseVisualStyleBackColor = true;
            this.buttonPurchaseRestrictionAddItem.Click += new System.EventHandler(this.ButtonPurchaseRestrictionAddItem_Click);
            // 
            // label26
            // 
            this.label26.AutoSize = true;
            this.label26.Location = new System.Drawing.Point(6, 26);
            this.label26.Name = "label26";
            this.label26.Size = new System.Drawing.Size(311, 13);
            this.label26.TabIndex = 21;
            this.label26.Text = "Prohibit purchase of item unless ship has one of the control items";
            // 
            // dataGridViewPurchaseRestrictions
            // 
            this.dataGridViewPurchaseRestrictions.AllowUserToAddRows = false;
            this.dataGridViewPurchaseRestrictions.AllowUserToDeleteRows = false;
            this.dataGridViewPurchaseRestrictions.AllowUserToResizeRows = false;
            this.dataGridViewPurchaseRestrictions.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewPurchaseRestrictions.AutoGenerateColumns = false;
            this.dataGridViewPurchaseRestrictions.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewPurchaseRestrictions.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.descriptionDataGridViewTextBoxColumn,
            this.itemNickNameDataGridViewTextBoxColumn3,
            this.controlDescriptionDataGridViewTextBoxColumn,
            this.controlItemNickNameDataGridViewTextBoxColumn});
            this.dataGridViewPurchaseRestrictions.DataSource = this.shipPurchaseRestrictionItemsBindingSource;
            this.dataGridViewPurchaseRestrictions.Location = new System.Drawing.Point(6, 42);
            this.dataGridViewPurchaseRestrictions.Name = "dataGridViewPurchaseRestrictions";
            this.dataGridViewPurchaseRestrictions.ReadOnly = true;
            this.dataGridViewPurchaseRestrictions.RowHeadersVisible = false;
            this.dataGridViewPurchaseRestrictions.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridViewPurchaseRestrictions.Size = new System.Drawing.Size(688, 339);
            this.dataGridViewPurchaseRestrictions.TabIndex = 20;
            this.dataGridViewPurchaseRestrictions.CellDoubleClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.DataGridViewPurchaseRestrictions_CellDoubleClick);
            // 
            // descriptionDataGridViewTextBoxColumn
            // 
            this.descriptionDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.descriptionDataGridViewTextBoxColumn.DataPropertyName = "Description";
            this.descriptionDataGridViewTextBoxColumn.HeaderText = "Description";
            this.descriptionDataGridViewTextBoxColumn.Name = "descriptionDataGridViewTextBoxColumn";
            this.descriptionDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // itemNickNameDataGridViewTextBoxColumn3
            // 
            this.itemNickNameDataGridViewTextBoxColumn3.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.itemNickNameDataGridViewTextBoxColumn3.DataPropertyName = "ItemNickName";
            this.itemNickNameDataGridViewTextBoxColumn3.HeaderText = "Item NickName";
            this.itemNickNameDataGridViewTextBoxColumn3.Name = "itemNickNameDataGridViewTextBoxColumn3";
            this.itemNickNameDataGridViewTextBoxColumn3.ReadOnly = true;
            // 
            // controlDescriptionDataGridViewTextBoxColumn
            // 
            this.controlDescriptionDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.controlDescriptionDataGridViewTextBoxColumn.DataPropertyName = "ControlDescription";
            this.controlDescriptionDataGridViewTextBoxColumn.HeaderText = "Control Description";
            this.controlDescriptionDataGridViewTextBoxColumn.Name = "controlDescriptionDataGridViewTextBoxColumn";
            this.controlDescriptionDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // controlItemNickNameDataGridViewTextBoxColumn
            // 
            this.controlItemNickNameDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.controlItemNickNameDataGridViewTextBoxColumn.DataPropertyName = "ControlItemNickName";
            this.controlItemNickNameDataGridViewTextBoxColumn.HeaderText = "Control Item NickName";
            this.controlItemNickNameDataGridViewTextBoxColumn.Name = "controlItemNickNameDataGridViewTextBoxColumn";
            this.controlItemNickNameDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // shipPurchaseRestrictionItemsBindingSource
            // 
            this.shipPurchaseRestrictionItemsBindingSource.DataMember = "ShipPurchaseRestrictionItems";
            this.shipPurchaseRestrictionItemsBindingSource.DataSource = this.uIDataSet;
            // 
            // checkBoxEnforceIDRestrictions
            // 
            this.checkBoxEnforceIDRestrictions.AutoSize = true;
            this.checkBoxEnforceIDRestrictions.Location = new System.Drawing.Point(210, 6);
            this.checkBoxEnforceIDRestrictions.Name = "checkBoxEnforceIDRestrictions";
            this.checkBoxEnforceIDRestrictions.Size = new System.Drawing.Size(387, 17);
            this.checkBoxEnforceIDRestrictions.TabIndex = 9;
            this.checkBoxEnforceIDRestrictions.Text = "Enforce purchase restrictions (applies to equipment purchase restrictions too)";
            this.checkBoxEnforceIDRestrictions.UseVisualStyleBackColor = true;
            // 
            // tabPage11
            // 
            this.tabPage11.Controls.Add(this.buttonEquipPurchaseRestrictionDel);
            this.tabPage11.Controls.Add(this.buttonEquipPurchaseRestrictionEdit);
            this.tabPage11.Controls.Add(this.buttonEquipPurchaseRestrictionAdd);
            this.tabPage11.Controls.Add(this.label39);
            this.tabPage11.Controls.Add(this.dataGridViewEquipPurchaseRestriction);
            this.tabPage11.Location = new System.Drawing.Point(4, 22);
            this.tabPage11.Name = "tabPage11";
            this.tabPage11.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage11.Size = new System.Drawing.Size(700, 416);
            this.tabPage11.TabIndex = 11;
            this.tabPage11.Text = "Equipment Purchase Restriction";
            this.tabPage11.UseVisualStyleBackColor = true;
            // 
            // buttonEquipPurchaseRestrictionDel
            // 
            this.buttonEquipPurchaseRestrictionDel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonEquipPurchaseRestrictionDel.Location = new System.Drawing.Point(168, 390);
            this.buttonEquipPurchaseRestrictionDel.Name = "buttonEquipPurchaseRestrictionDel";
            this.buttonEquipPurchaseRestrictionDel.Size = new System.Drawing.Size(75, 23);
            this.buttonEquipPurchaseRestrictionDel.TabIndex = 27;
            this.buttonEquipPurchaseRestrictionDel.Text = "Del Item";
            this.buttonEquipPurchaseRestrictionDel.UseVisualStyleBackColor = true;
            this.buttonEquipPurchaseRestrictionDel.Click += new System.EventHandler(this.ButtonEquipPurchaseRestrictionDel_Click);
            // 
            // buttonEquipPurchaseRestrictionEdit
            // 
            this.buttonEquipPurchaseRestrictionEdit.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonEquipPurchaseRestrictionEdit.Location = new System.Drawing.Point(87, 390);
            this.buttonEquipPurchaseRestrictionEdit.Name = "buttonEquipPurchaseRestrictionEdit";
            this.buttonEquipPurchaseRestrictionEdit.Size = new System.Drawing.Size(75, 23);
            this.buttonEquipPurchaseRestrictionEdit.TabIndex = 26;
            this.buttonEquipPurchaseRestrictionEdit.Text = "Edit Item";
            this.buttonEquipPurchaseRestrictionEdit.UseVisualStyleBackColor = true;
            this.buttonEquipPurchaseRestrictionEdit.Click += new System.EventHandler(this.ButtonEquipPurchaseRestrictionEdit_Click);
            // 
            // buttonEquipPurchaseRestrictionAdd
            // 
            this.buttonEquipPurchaseRestrictionAdd.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonEquipPurchaseRestrictionAdd.Location = new System.Drawing.Point(6, 390);
            this.buttonEquipPurchaseRestrictionAdd.Name = "buttonEquipPurchaseRestrictionAdd";
            this.buttonEquipPurchaseRestrictionAdd.Size = new System.Drawing.Size(75, 23);
            this.buttonEquipPurchaseRestrictionAdd.TabIndex = 25;
            this.buttonEquipPurchaseRestrictionAdd.Text = "Add Item";
            this.buttonEquipPurchaseRestrictionAdd.UseVisualStyleBackColor = true;
            this.buttonEquipPurchaseRestrictionAdd.Click += new System.EventHandler(this.ButtonEquipPurchaseRestrictionAdd_Click);
            // 
            // label39
            // 
            this.label39.AutoSize = true;
            this.label39.Location = new System.Drawing.Point(6, 3);
            this.label39.Name = "label39";
            this.label39.Size = new System.Drawing.Size(311, 13);
            this.label39.TabIndex = 23;
            this.label39.Text = "Prohibit purchase of item unless ship has one of the control items";
            // 
            // dataGridViewEquipPurchaseRestriction
            // 
            this.dataGridViewEquipPurchaseRestriction.AllowUserToAddRows = false;
            this.dataGridViewEquipPurchaseRestriction.AllowUserToDeleteRows = false;
            this.dataGridViewEquipPurchaseRestriction.AllowUserToResizeRows = false;
            this.dataGridViewEquipPurchaseRestriction.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewEquipPurchaseRestriction.AutoGenerateColumns = false;
            this.dataGridViewEquipPurchaseRestriction.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewEquipPurchaseRestriction.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.descriptionDataGridViewTextBoxColumn1,
            this.itemNickNameDataGridViewTextBoxColumn4,
            this.controlDescriptionDataGridViewTextBoxColumn1,
            this.controlItemNickNameDataGridViewTextBoxColumn1});
            this.dataGridViewEquipPurchaseRestriction.DataSource = this.equipPurchaseRestrictionItemsBindingSource;
            this.dataGridViewEquipPurchaseRestriction.Location = new System.Drawing.Point(6, 19);
            this.dataGridViewEquipPurchaseRestriction.Name = "dataGridViewEquipPurchaseRestriction";
            this.dataGridViewEquipPurchaseRestriction.ReadOnly = true;
            this.dataGridViewEquipPurchaseRestriction.RowHeadersVisible = false;
            this.dataGridViewEquipPurchaseRestriction.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridViewEquipPurchaseRestriction.Size = new System.Drawing.Size(688, 367);
            this.dataGridViewEquipPurchaseRestriction.TabIndex = 22;
            this.dataGridViewEquipPurchaseRestriction.CellDoubleClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.DataGridViewEquipPurchaseRestriction_CellDoubleClick);
            // 
            // descriptionDataGridViewTextBoxColumn1
            // 
            this.descriptionDataGridViewTextBoxColumn1.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.descriptionDataGridViewTextBoxColumn1.DataPropertyName = "Description";
            this.descriptionDataGridViewTextBoxColumn1.HeaderText = "Description";
            this.descriptionDataGridViewTextBoxColumn1.Name = "descriptionDataGridViewTextBoxColumn1";
            this.descriptionDataGridViewTextBoxColumn1.ReadOnly = true;
            // 
            // itemNickNameDataGridViewTextBoxColumn4
            // 
            this.itemNickNameDataGridViewTextBoxColumn4.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.itemNickNameDataGridViewTextBoxColumn4.DataPropertyName = "ItemNickName";
            this.itemNickNameDataGridViewTextBoxColumn4.HeaderText = "Item NickName";
            this.itemNickNameDataGridViewTextBoxColumn4.Name = "itemNickNameDataGridViewTextBoxColumn4";
            this.itemNickNameDataGridViewTextBoxColumn4.ReadOnly = true;
            // 
            // controlDescriptionDataGridViewTextBoxColumn1
            // 
            this.controlDescriptionDataGridViewTextBoxColumn1.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.controlDescriptionDataGridViewTextBoxColumn1.DataPropertyName = "ControlDescription";
            this.controlDescriptionDataGridViewTextBoxColumn1.HeaderText = "Control Description";
            this.controlDescriptionDataGridViewTextBoxColumn1.Name = "controlDescriptionDataGridViewTextBoxColumn1";
            this.controlDescriptionDataGridViewTextBoxColumn1.ReadOnly = true;
            // 
            // controlItemNickNameDataGridViewTextBoxColumn1
            // 
            this.controlItemNickNameDataGridViewTextBoxColumn1.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.controlItemNickNameDataGridViewTextBoxColumn1.DataPropertyName = "ControlItemNickName";
            this.controlItemNickNameDataGridViewTextBoxColumn1.HeaderText = "Control Item NickName";
            this.controlItemNickNameDataGridViewTextBoxColumn1.Name = "controlItemNickNameDataGridViewTextBoxColumn1";
            this.controlItemNickNameDataGridViewTextBoxColumn1.ReadOnly = true;
            // 
            // equipPurchaseRestrictionItemsBindingSource
            // 
            this.equipPurchaseRestrictionItemsBindingSource.DataMember = "EquipPurchaseRestrictionItems";
            this.equipPurchaseRestrictionItemsBindingSource.DataSource = this.uIDataSet;
            // 
            // tabPage10
            // 
            this.tabPage10.Controls.Add(this.buttonDelNoBuyItem);
            this.tabPage10.Controls.Add(this.buttonEditNoBuyItem);
            this.tabPage10.Controls.Add(this.buttonAddNoBuyItem);
            this.tabPage10.Controls.Add(this.label37);
            this.tabPage10.Controls.Add(this.dataGridViewNoBuyList);
            this.tabPage10.Location = new System.Drawing.Point(4, 22);
            this.tabPage10.Name = "tabPage10";
            this.tabPage10.Size = new System.Drawing.Size(700, 416);
            this.tabPage10.TabIndex = 10;
            this.tabPage10.Text = "Base Good Purchase Restriction";
            this.tabPage10.UseVisualStyleBackColor = true;
            // 
            // buttonDelNoBuyItem
            // 
            this.buttonDelNoBuyItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonDelNoBuyItem.Location = new System.Drawing.Point(168, 390);
            this.buttonDelNoBuyItem.Name = "buttonDelNoBuyItem";
            this.buttonDelNoBuyItem.Size = new System.Drawing.Size(75, 23);
            this.buttonDelNoBuyItem.TabIndex = 26;
            this.buttonDelNoBuyItem.Text = "Del Item";
            this.buttonDelNoBuyItem.UseVisualStyleBackColor = true;
            this.buttonDelNoBuyItem.Click += new System.EventHandler(this.ButtonDelNoBuyItem_Click);
            // 
            // buttonEditNoBuyItem
            // 
            this.buttonEditNoBuyItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonEditNoBuyItem.Location = new System.Drawing.Point(87, 390);
            this.buttonEditNoBuyItem.Name = "buttonEditNoBuyItem";
            this.buttonEditNoBuyItem.Size = new System.Drawing.Size(75, 23);
            this.buttonEditNoBuyItem.TabIndex = 25;
            this.buttonEditNoBuyItem.Text = "Edit Item";
            this.buttonEditNoBuyItem.UseVisualStyleBackColor = true;
            this.buttonEditNoBuyItem.Click += new System.EventHandler(this.ButtonEditNoBuyItem_Click);
            // 
            // buttonAddNoBuyItem
            // 
            this.buttonAddNoBuyItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonAddNoBuyItem.Location = new System.Drawing.Point(6, 390);
            this.buttonAddNoBuyItem.Name = "buttonAddNoBuyItem";
            this.buttonAddNoBuyItem.Size = new System.Drawing.Size(75, 23);
            this.buttonAddNoBuyItem.TabIndex = 24;
            this.buttonAddNoBuyItem.Text = "Add Item";
            this.buttonAddNoBuyItem.UseVisualStyleBackColor = true;
            this.buttonAddNoBuyItem.Click += new System.EventHandler(this.ButtonAddNoBuyItem_Click);
            // 
            // label37
            // 
            this.label37.AutoSize = true;
            this.label37.Location = new System.Drawing.Point(3, 0);
            this.label37.Name = "label37";
            this.label37.Size = new System.Drawing.Size(190, 13);
            this.label37.TabIndex = 23;
            this.label37.Text = "Prohibit purchase of the following items";
            // 
            // dataGridViewNoBuyList
            // 
            this.dataGridViewNoBuyList.AllowUserToAddRows = false;
            this.dataGridViewNoBuyList.AllowUserToDeleteRows = false;
            this.dataGridViewNoBuyList.AllowUserToResizeRows = false;
            this.dataGridViewNoBuyList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewNoBuyList.AutoGenerateColumns = false;
            this.dataGridViewNoBuyList.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewNoBuyList.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.baseDescriptionDataGridViewTextBoxColumn,
            this.baseNickNameDataGridViewTextBoxColumn,
            this.goodDescriptionDataGridViewTextBoxColumn,
            this.goodNickNameDataGridViewTextBoxColumn});
            this.dataGridViewNoBuyList.DataSource = this.baseGoodNoBuyListBindingSource;
            this.dataGridViewNoBuyList.Location = new System.Drawing.Point(3, 16);
            this.dataGridViewNoBuyList.Name = "dataGridViewNoBuyList";
            this.dataGridViewNoBuyList.ReadOnly = true;
            this.dataGridViewNoBuyList.RowHeadersVisible = false;
            this.dataGridViewNoBuyList.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridViewNoBuyList.Size = new System.Drawing.Size(688, 368);
            this.dataGridViewNoBuyList.TabIndex = 22;
            this.dataGridViewNoBuyList.CellDoubleClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.DataGridView1_CellDoubleClick);
            // 
            // baseDescriptionDataGridViewTextBoxColumn
            // 
            this.baseDescriptionDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.baseDescriptionDataGridViewTextBoxColumn.DataPropertyName = "BaseDescription";
            this.baseDescriptionDataGridViewTextBoxColumn.HeaderText = "Base Description";
            this.baseDescriptionDataGridViewTextBoxColumn.Name = "baseDescriptionDataGridViewTextBoxColumn";
            this.baseDescriptionDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // baseNickNameDataGridViewTextBoxColumn
            // 
            this.baseNickNameDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.baseNickNameDataGridViewTextBoxColumn.DataPropertyName = "BaseNickName";
            this.baseNickNameDataGridViewTextBoxColumn.HeaderText = "Base NickName";
            this.baseNickNameDataGridViewTextBoxColumn.Name = "baseNickNameDataGridViewTextBoxColumn";
            this.baseNickNameDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // goodDescriptionDataGridViewTextBoxColumn
            // 
            this.goodDescriptionDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.goodDescriptionDataGridViewTextBoxColumn.DataPropertyName = "GoodDescription";
            this.goodDescriptionDataGridViewTextBoxColumn.HeaderText = "Good Description";
            this.goodDescriptionDataGridViewTextBoxColumn.Name = "goodDescriptionDataGridViewTextBoxColumn";
            this.goodDescriptionDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // goodNickNameDataGridViewTextBoxColumn
            // 
            this.goodNickNameDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.goodNickNameDataGridViewTextBoxColumn.DataPropertyName = "GoodNickName";
            this.goodNickNameDataGridViewTextBoxColumn.HeaderText = "Good NickName";
            this.goodNickNameDataGridViewTextBoxColumn.Name = "goodNickNameDataGridViewTextBoxColumn";
            this.goodNickNameDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // baseGoodNoBuyListBindingSource
            // 
            this.baseGoodNoBuyListBindingSource.DataMember = "BaseGoodNoBuyList";
            this.baseGoodNoBuyListBindingSource.DataSource = this.uIDataSet;
            // 
            // tabPage7
            // 
            this.tabPage7.Controls.Add(this.richTextBoxSwearWords);
            this.tabPage7.Controls.Add(this.label44);
            this.tabPage7.Controls.Add(this.richTextBoxHelpLines);
            this.tabPage7.Controls.Add(this.textBoxCmdEchoStyle);
            this.tabPage7.Controls.Add(this.label28);
            this.tabPage7.Controls.Add(this.checkBoxCmdHide);
            this.tabPage7.Controls.Add(this.checkBoxCmdEcho);
            this.tabPage7.Controls.Add(this.label27);
            this.tabPage7.Controls.Add(this.checkBoxCustomHelp);
            this.tabPage7.Location = new System.Drawing.Point(4, 22);
            this.tabPage7.Name = "tabPage7";
            this.tabPage7.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage7.Size = new System.Drawing.Size(700, 416);
            this.tabPage7.TabIndex = 7;
            this.tabPage7.Text = "Messages";
            this.tabPage7.UseVisualStyleBackColor = true;
            // 
            // richTextBoxSwearWords
            // 
            this.richTextBoxSwearWords.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxSwearWords.DetectUrls = false;
            this.richTextBoxSwearWords.Location = new System.Drawing.Point(10, 248);
            this.richTextBoxSwearWords.Name = "richTextBoxSwearWords";
            this.richTextBoxSwearWords.Size = new System.Drawing.Size(684, 162);
            this.richTextBoxSwearWords.TabIndex = 11;
            this.richTextBoxSwearWords.Text = "";
            // 
            // label44
            // 
            this.label44.AutoSize = true;
            this.label44.Location = new System.Drawing.Point(7, 231);
            this.label44.Name = "label44";
            this.label44.Size = new System.Drawing.Size(314, 13);
            this.label44.TabIndex = 10;
            this.label44.Text = "Suppress chat containing the following (1 line per word or phrase)";
            // 
            // richTextBoxHelpLines
            // 
            this.richTextBoxHelpLines.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxHelpLines.DetectUrls = false;
            this.richTextBoxHelpLines.Location = new System.Drawing.Point(9, 100);
            this.richTextBoxHelpLines.Name = "richTextBoxHelpLines";
            this.richTextBoxHelpLines.Size = new System.Drawing.Size(684, 128);
            this.richTextBoxHelpLines.TabIndex = 9;
            this.richTextBoxHelpLines.Text = "";
            // 
            // textBoxCmdEchoStyle
            // 
            this.textBoxCmdEchoStyle.Location = new System.Drawing.Point(458, 5);
            this.textBoxCmdEchoStyle.Name = "textBoxCmdEchoStyle";
            this.textBoxCmdEchoStyle.Size = new System.Drawing.Size(108, 20);
            this.textBoxCmdEchoStyle.TabIndex = 6;
            // 
            // label28
            // 
            this.label28.AutoSize = true;
            this.label28.Location = new System.Drawing.Point(326, 8);
            this.label28.Name = "label28";
            this.label28.Size = new System.Drawing.Size(126, 13);
            this.label28.TabIndex = 5;
            this.label28.Text = "Echo command font style";
            // 
            // checkBoxCmdHide
            // 
            this.checkBoxCmdHide.AutoSize = true;
            this.checkBoxCmdHide.Location = new System.Drawing.Point(9, 31);
            this.checkBoxCmdHide.Name = "checkBoxCmdHide";
            this.checkBoxCmdHide.Size = new System.Drawing.Size(264, 17);
            this.checkBoxCmdHide.TabIndex = 4;
            this.checkBoxCmdHide.Text = "Hide FLHook / and . commands from other players";
            this.checkBoxCmdHide.UseVisualStyleBackColor = true;
            // 
            // checkBoxCmdEcho
            // 
            this.checkBoxCmdEcho.AutoSize = true;
            this.checkBoxCmdEcho.Location = new System.Drawing.Point(9, 8);
            this.checkBoxCmdEcho.Name = "checkBoxCmdEcho";
            this.checkBoxCmdEcho.Size = new System.Drawing.Size(181, 17);
            this.checkBoxCmdEcho.TabIndex = 3;
            this.checkBoxCmdEcho.Text = "Echo FLHook / and . commands";
            this.checkBoxCmdEcho.UseVisualStyleBackColor = true;
            // 
            // label27
            // 
            this.label27.AutoSize = true;
            this.label27.Location = new System.Drawing.Point(6, 83);
            this.label27.Name = "label27";
            this.label27.Size = new System.Drawing.Size(110, 13);
            this.label27.TabIndex = 2;
            this.label27.Text = "Custom help message";
            // 
            // checkBoxCustomHelp
            // 
            this.checkBoxCustomHelp.AutoSize = true;
            this.checkBoxCustomHelp.Location = new System.Drawing.Point(9, 54);
            this.checkBoxCustomHelp.Name = "checkBoxCustomHelp";
            this.checkBoxCustomHelp.Size = new System.Drawing.Size(290, 17);
            this.checkBoxCustomHelp.TabIndex = 0;
            this.checkBoxCustomHelp.Text = "Enable custom /help and override default FLHook /help";
            this.checkBoxCustomHelp.UseVisualStyleBackColor = true;
            // 
            // tabPage8
            // 
            this.tabPage8.Controls.Add(this.label32);
            this.tabPage8.Controls.Add(this.textBoxStandardBannerDelay);
            this.tabPage8.Controls.Add(this.buttonBannerDelItem);
            this.tabPage8.Controls.Add(this.buttonBannerEditItem);
            this.tabPage8.Controls.Add(this.buttonBannerAddItem);
            this.tabPage8.Controls.Add(this.dataGridViewStandardBanners);
            this.tabPage8.Controls.Add(this.label31);
            this.tabPage8.Location = new System.Drawing.Point(4, 22);
            this.tabPage8.Name = "tabPage8";
            this.tabPage8.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage8.Size = new System.Drawing.Size(700, 416);
            this.tabPage8.TabIndex = 8;
            this.tabPage8.Text = "Banners";
            this.tabPage8.UseVisualStyleBackColor = true;
            // 
            // label32
            // 
            this.label32.AutoSize = true;
            this.label32.Location = new System.Drawing.Point(5, 9);
            this.label32.Name = "label32";
            this.label32.Size = new System.Drawing.Size(178, 13);
            this.label32.TabIndex = 30;
            this.label32.Text = "Seconds between standard banners";
            // 
            // textBoxStandardBannerDelay
            // 
            this.textBoxStandardBannerDelay.Location = new System.Drawing.Point(189, 6);
            this.textBoxStandardBannerDelay.Name = "textBoxStandardBannerDelay";
            this.textBoxStandardBannerDelay.Size = new System.Drawing.Size(108, 20);
            this.textBoxStandardBannerDelay.TabIndex = 28;
            // 
            // buttonBannerDelItem
            // 
            this.buttonBannerDelItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonBannerDelItem.Location = new System.Drawing.Point(167, 390);
            this.buttonBannerDelItem.Name = "buttonBannerDelItem";
            this.buttonBannerDelItem.Size = new System.Drawing.Size(75, 23);
            this.buttonBannerDelItem.TabIndex = 27;
            this.buttonBannerDelItem.Text = "Del Item";
            this.buttonBannerDelItem.UseVisualStyleBackColor = true;
            this.buttonBannerDelItem.Click += new System.EventHandler(this.ButtonBannerDelItem_Click);
            // 
            // buttonBannerEditItem
            // 
            this.buttonBannerEditItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonBannerEditItem.Location = new System.Drawing.Point(86, 390);
            this.buttonBannerEditItem.Name = "buttonBannerEditItem";
            this.buttonBannerEditItem.Size = new System.Drawing.Size(75, 23);
            this.buttonBannerEditItem.TabIndex = 26;
            this.buttonBannerEditItem.Text = "Edit Item";
            this.buttonBannerEditItem.UseVisualStyleBackColor = true;
            this.buttonBannerEditItem.Click += new System.EventHandler(this.ButtonBannerEditItem_Click);
            // 
            // buttonBannerAddItem
            // 
            this.buttonBannerAddItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonBannerAddItem.Location = new System.Drawing.Point(5, 390);
            this.buttonBannerAddItem.Name = "buttonBannerAddItem";
            this.buttonBannerAddItem.Size = new System.Drawing.Size(75, 23);
            this.buttonBannerAddItem.TabIndex = 25;
            this.buttonBannerAddItem.Text = "Add Item";
            this.buttonBannerAddItem.UseVisualStyleBackColor = true;
            this.buttonBannerAddItem.Click += new System.EventHandler(this.ButtonBannerAddItem_Click);
            // 
            // dataGridViewStandardBanners
            // 
            this.dataGridViewStandardBanners.AllowUserToAddRows = false;
            this.dataGridViewStandardBanners.AllowUserToDeleteRows = false;
            this.dataGridViewStandardBanners.AllowUserToResizeRows = false;
            this.dataGridViewStandardBanners.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewStandardBanners.AutoGenerateColumns = false;
            this.dataGridViewStandardBanners.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewStandardBanners.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.bannerTextDataGridViewTextBoxColumn});
            this.dataGridViewStandardBanners.DataSource = this.standardBannerListBindingSource;
            this.dataGridViewStandardBanners.Location = new System.Drawing.Point(7, 48);
            this.dataGridViewStandardBanners.Name = "dataGridViewStandardBanners";
            this.dataGridViewStandardBanners.ReadOnly = true;
            this.dataGridViewStandardBanners.RowHeadersVisible = false;
            this.dataGridViewStandardBanners.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridViewStandardBanners.Size = new System.Drawing.Size(686, 336);
            this.dataGridViewStandardBanners.TabIndex = 12;
            this.dataGridViewStandardBanners.CellDoubleClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.DataGridViewStandardBanners_CellDoubleClick);
            // 
            // bannerTextDataGridViewTextBoxColumn
            // 
            this.bannerTextDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.bannerTextDataGridViewTextBoxColumn.DataPropertyName = "BannerText";
            this.bannerTextDataGridViewTextBoxColumn.HeaderText = "BannerText";
            this.bannerTextDataGridViewTextBoxColumn.Name = "bannerTextDataGridViewTextBoxColumn";
            this.bannerTextDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // standardBannerListBindingSource
            // 
            this.standardBannerListBindingSource.DataMember = "StandardBannerList";
            this.standardBannerListBindingSource.DataSource = this.uIDataSet;
            // 
            // label31
            // 
            this.label31.AutoSize = true;
            this.label31.Location = new System.Drawing.Point(8, 32);
            this.label31.Name = "label31";
            this.label31.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.label31.Size = new System.Drawing.Size(91, 13);
            this.label31.TabIndex = 11;
            this.label31.Text = "Standard banners";
            // 
            // tabPage9
            // 
            this.tabPage9.Controls.Add(this.richTextBox1);
            this.tabPage9.Controls.Add(this.richTextBoxGreetingBannerLines);
            this.tabPage9.Controls.Add(this.label29);
            this.tabPage9.Controls.Add(this.richTextBoxSpecialBannerLines);
            this.tabPage9.Controls.Add(this.label33);
            this.tabPage9.Controls.Add(this.textBoxSpecialBannerDelay);
            this.tabPage9.Controls.Add(this.label30);
            this.tabPage9.Location = new System.Drawing.Point(4, 22);
            this.tabPage9.Name = "tabPage9";
            this.tabPage9.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage9.Size = new System.Drawing.Size(700, 416);
            this.tabPage9.TabIndex = 9;
            this.tabPage9.Text = "Special Banners";
            this.tabPage9.UseVisualStyleBackColor = true;
            // 
            // richTextBox1
            // 
            this.richTextBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBox1.Location = new System.Drawing.Point(9, 246);
            this.richTextBox1.Name = "richTextBox1";
            this.richTextBox1.ReadOnly = true;
            this.richTextBox1.Size = new System.Drawing.Size(684, 164);
            this.richTextBox1.TabIndex = 39;
            this.richTextBox1.Text = resources.GetString("richTextBox1.Text");
            // 
            // richTextBoxGreetingBannerLines
            // 
            this.richTextBoxGreetingBannerLines.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxGreetingBannerLines.DetectUrls = false;
            this.richTextBoxGreetingBannerLines.Location = new System.Drawing.Point(9, 143);
            this.richTextBoxGreetingBannerLines.Name = "richTextBoxGreetingBannerLines";
            this.richTextBoxGreetingBannerLines.Size = new System.Drawing.Size(684, 97);
            this.richTextBoxGreetingBannerLines.TabIndex = 38;
            this.richTextBoxGreetingBannerLines.Text = "";
            // 
            // label29
            // 
            this.label29.AutoSize = true;
            this.label29.Location = new System.Drawing.Point(6, 127);
            this.label29.Name = "label29";
            this.label29.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.label29.Size = new System.Drawing.Size(83, 13);
            this.label29.TabIndex = 37;
            this.label29.Text = "Greeting banner";
            // 
            // richTextBoxSpecialBannerLines
            // 
            this.richTextBoxSpecialBannerLines.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxSpecialBannerLines.DetectUrls = false;
            this.richTextBoxSpecialBannerLines.Location = new System.Drawing.Point(8, 54);
            this.richTextBoxSpecialBannerLines.Name = "richTextBoxSpecialBannerLines";
            this.richTextBoxSpecialBannerLines.Size = new System.Drawing.Size(685, 71);
            this.richTextBoxSpecialBannerLines.TabIndex = 36;
            this.richTextBoxSpecialBannerLines.Text = "";
            // 
            // label33
            // 
            this.label33.AutoSize = true;
            this.label33.Location = new System.Drawing.Point(6, 9);
            this.label33.Name = "label33";
            this.label33.Size = new System.Drawing.Size(170, 13);
            this.label33.TabIndex = 35;
            this.label33.Text = "Seconds between special banners";
            // 
            // textBoxSpecialBannerDelay
            // 
            this.textBoxSpecialBannerDelay.Location = new System.Drawing.Point(182, 6);
            this.textBoxSpecialBannerDelay.Name = "textBoxSpecialBannerDelay";
            this.textBoxSpecialBannerDelay.Size = new System.Drawing.Size(108, 20);
            this.textBoxSpecialBannerDelay.TabIndex = 34;
            // 
            // label30
            // 
            this.label30.AutoSize = true;
            this.label30.Location = new System.Drawing.Point(6, 38);
            this.label30.Name = "label30";
            this.label30.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.label30.Size = new System.Drawing.Size(78, 13);
            this.label30.TabIndex = 33;
            this.label30.Text = "Special banner";
            // 
            // tabPage12
            // 
            this.tabPage12.Controls.Add(this.buttonSysSensorDelItem);
            this.tabPage12.Controls.Add(this.buttonSysSensorEditItem);
            this.tabPage12.Controls.Add(this.buttonSysSensorAddItem);
            this.tabPage12.Controls.Add(this.label40);
            this.tabPage12.Controls.Add(this.dataGridViewSysSensor);
            this.tabPage12.Location = new System.Drawing.Point(4, 22);
            this.tabPage12.Name = "tabPage12";
            this.tabPage12.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage12.Size = new System.Drawing.Size(700, 416);
            this.tabPage12.TabIndex = 12;
            this.tabPage12.Text = "System Sensors";
            this.tabPage12.UseVisualStyleBackColor = true;
            // 
            // buttonSysSensorDelItem
            // 
            this.buttonSysSensorDelItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonSysSensorDelItem.Location = new System.Drawing.Point(171, 387);
            this.buttonSysSensorDelItem.Name = "buttonSysSensorDelItem";
            this.buttonSysSensorDelItem.Size = new System.Drawing.Size(75, 23);
            this.buttonSysSensorDelItem.TabIndex = 31;
            this.buttonSysSensorDelItem.Text = "Del Item";
            this.buttonSysSensorDelItem.UseVisualStyleBackColor = true;
            this.buttonSysSensorDelItem.Click += new System.EventHandler(this.ButtonSysSensorDelItem_Click);
            // 
            // buttonSysSensorEditItem
            // 
            this.buttonSysSensorEditItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonSysSensorEditItem.Location = new System.Drawing.Point(90, 387);
            this.buttonSysSensorEditItem.Name = "buttonSysSensorEditItem";
            this.buttonSysSensorEditItem.Size = new System.Drawing.Size(75, 23);
            this.buttonSysSensorEditItem.TabIndex = 30;
            this.buttonSysSensorEditItem.Text = "Edit Item";
            this.buttonSysSensorEditItem.UseVisualStyleBackColor = true;
            this.buttonSysSensorEditItem.Click += new System.EventHandler(this.ButtonSysSensorEditItem_Click);
            // 
            // buttonSysSensorAddItem
            // 
            this.buttonSysSensorAddItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonSysSensorAddItem.Location = new System.Drawing.Point(9, 387);
            this.buttonSysSensorAddItem.Name = "buttonSysSensorAddItem";
            this.buttonSysSensorAddItem.Size = new System.Drawing.Size(75, 23);
            this.buttonSysSensorAddItem.TabIndex = 29;
            this.buttonSysSensorAddItem.Text = "Add Item";
            this.buttonSysSensorAddItem.UseVisualStyleBackColor = true;
            this.buttonSysSensorAddItem.Click += new System.EventHandler(this.ButtonSysSensorAddItem_Click);
            // 
            // label40
            // 
            this.label40.AutoSize = true;
            this.label40.Location = new System.Drawing.Point(6, 2);
            this.label40.Name = "label40";
            this.label40.Size = new System.Drawing.Size(532, 13);
            this.label40.TabIndex = 28;
            this.label40.Text = "Ships in the specified systems that have the listed equipment receive notificatio" +
    "ns when people use a tradelane.";
            // 
            // dataGridViewSysSensor
            // 
            this.dataGridViewSysSensor.AllowUserToAddRows = false;
            this.dataGridViewSysSensor.AllowUserToDeleteRows = false;
            this.dataGridViewSysSensor.AllowUserToResizeRows = false;
            this.dataGridViewSysSensor.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridViewSysSensor.AutoGenerateColumns = false;
            this.dataGridViewSysSensor.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridViewSysSensor.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.systemDescriptionDataGridViewTextBoxColumn,
            this.systemNickNameDataGridViewTextBoxColumn,
            this.equipDescriptionDataGridViewTextBoxColumn,
            this.equipNickNameDataGridViewTextBoxColumn,
            this.NetworkID});
            this.dataGridViewSysSensor.DataSource = this.sysSensorListBindingSource;
            this.dataGridViewSysSensor.Location = new System.Drawing.Point(6, 18);
            this.dataGridViewSysSensor.Name = "dataGridViewSysSensor";
            this.dataGridViewSysSensor.ReadOnly = true;
            this.dataGridViewSysSensor.RowHeadersVisible = false;
            this.dataGridViewSysSensor.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridViewSysSensor.Size = new System.Drawing.Size(688, 363);
            this.dataGridViewSysSensor.TabIndex = 27;
            this.dataGridViewSysSensor.CellDoubleClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.DataGridViewSysSensor_CellDoubleClick);
            // 
            // systemDescriptionDataGridViewTextBoxColumn
            // 
            this.systemDescriptionDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.systemDescriptionDataGridViewTextBoxColumn.DataPropertyName = "SystemDescription";
            this.systemDescriptionDataGridViewTextBoxColumn.HeaderText = "System Description";
            this.systemDescriptionDataGridViewTextBoxColumn.Name = "systemDescriptionDataGridViewTextBoxColumn";
            this.systemDescriptionDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // systemNickNameDataGridViewTextBoxColumn
            // 
            this.systemNickNameDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.systemNickNameDataGridViewTextBoxColumn.DataPropertyName = "SystemNickName";
            this.systemNickNameDataGridViewTextBoxColumn.HeaderText = "System NickName";
            this.systemNickNameDataGridViewTextBoxColumn.Name = "systemNickNameDataGridViewTextBoxColumn";
            this.systemNickNameDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // equipDescriptionDataGridViewTextBoxColumn
            // 
            this.equipDescriptionDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.equipDescriptionDataGridViewTextBoxColumn.DataPropertyName = "EquipDescription";
            this.equipDescriptionDataGridViewTextBoxColumn.HeaderText = "Equip Description";
            this.equipDescriptionDataGridViewTextBoxColumn.Name = "equipDescriptionDataGridViewTextBoxColumn";
            this.equipDescriptionDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // equipNickNameDataGridViewTextBoxColumn
            // 
            this.equipNickNameDataGridViewTextBoxColumn.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.equipNickNameDataGridViewTextBoxColumn.DataPropertyName = "EquipNickName";
            this.equipNickNameDataGridViewTextBoxColumn.HeaderText = "Equip NickName";
            this.equipNickNameDataGridViewTextBoxColumn.Name = "equipNickNameDataGridViewTextBoxColumn";
            this.equipNickNameDataGridViewTextBoxColumn.ReadOnly = true;
            // 
            // NetworkID
            // 
            this.NetworkID.DataPropertyName = "NetworkID";
            this.NetworkID.HeaderText = "NetworkID";
            this.NetworkID.Name = "NetworkID";
            this.NetworkID.ReadOnly = true;
            // 
            // sysSensorListBindingSource
            // 
            this.sysSensorListBindingSource.DataMember = "SysSensorList";
            this.sysSensorListBindingSource.DataSource = this.uIDataSet;
            // 
            // tabPage13
            // 
            this.tabPage13.Controls.Add(this.richTextBoxSounds);
            this.tabPage13.Controls.Add(this.label47);
            this.tabPage13.Controls.Add(this.checkBoxEnableLoginSound);
            this.tabPage13.Location = new System.Drawing.Point(4, 22);
            this.tabPage13.Name = "tabPage13";
            this.tabPage13.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage13.Size = new System.Drawing.Size(700, 416);
            this.tabPage13.TabIndex = 13;
            this.tabPage13.Text = "Login Sounds";
            this.tabPage13.UseVisualStyleBackColor = true;
            // 
            // richTextBoxSounds
            // 
            this.richTextBoxSounds.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxSounds.DetectUrls = false;
            this.richTextBoxSounds.Location = new System.Drawing.Point(6, 43);
            this.richTextBoxSounds.Name = "richTextBoxSounds";
            this.richTextBoxSounds.Size = new System.Drawing.Size(684, 367);
            this.richTextBoxSounds.TabIndex = 11;
            this.richTextBoxSounds.Text = resources.GetString("richTextBoxSounds.Text");
            this.richTextBoxSounds.TextChanged += new System.EventHandler(this.RichTextBoxSounds_TextChanged);
            // 
            // label47
            // 
            this.label47.AutoSize = true;
            this.label47.Location = new System.Drawing.Point(3, 26);
            this.label47.Name = "label47";
            this.label47.Size = new System.Drawing.Size(97, 13);
            this.label47.TabIndex = 10;
            this.label47.Text = "List of login sounds";
            // 
            // checkBoxEnableLoginSound
            // 
            this.checkBoxEnableLoginSound.AutoSize = true;
            this.checkBoxEnableLoginSound.Location = new System.Drawing.Point(6, 6);
            this.checkBoxEnableLoginSound.Name = "checkBoxEnableLoginSound";
            this.checkBoxEnableLoginSound.Size = new System.Drawing.Size(248, 17);
            this.checkBoxEnableLoginSound.TabIndex = 4;
            this.checkBoxEnableLoginSound.Text = "Enable Login Sounds (Play on Server Connect)";
            this.checkBoxEnableLoginSound.UseVisualStyleBackColor = true;
            // 
            // tabPage14
            // 
            this.tabPage14.Controls.Add(this.richTextBoxBodies);
            this.tabPage14.Controls.Add(this.label49);
            this.tabPage14.Controls.Add(this.richTextBoxHeads);
            this.tabPage14.Controls.Add(this.label48);
            this.tabPage14.Controls.Add(this.checkBoxEnableWardrobe);
            this.tabPage14.Location = new System.Drawing.Point(4, 22);
            this.tabPage14.Name = "tabPage14";
            this.tabPage14.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage14.Size = new System.Drawing.Size(700, 416);
            this.tabPage14.TabIndex = 14;
            this.tabPage14.Text = "Wardrobe";
            this.tabPage14.UseVisualStyleBackColor = true;
            // 
            // richTextBoxBodies
            // 
            this.richTextBoxBodies.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxBodies.DetectUrls = false;
            this.richTextBoxBodies.Location = new System.Drawing.Point(6, 236);
            this.richTextBoxBodies.Name = "richTextBoxBodies";
            this.richTextBoxBodies.Size = new System.Drawing.Size(684, 174);
            this.richTextBoxBodies.TabIndex = 15;
            this.richTextBoxBodies.Text = "orillion, pi_orillion_body";
            // 
            // label49
            // 
            this.label49.AutoSize = true;
            this.label49.Location = new System.Drawing.Point(3, 220);
            this.label49.Name = "label49";
            this.label49.Size = new System.Drawing.Size(39, 13);
            this.label49.TabIndex = 14;
            this.label49.Text = "Bodies";
            // 
            // richTextBoxHeads
            // 
            this.richTextBoxHeads.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxHeads.DetectUrls = false;
            this.richTextBoxHeads.Location = new System.Drawing.Point(6, 43);
            this.richTextBoxHeads.Name = "richTextBoxHeads";
            this.richTextBoxHeads.Size = new System.Drawing.Size(684, 174);
            this.richTextBoxHeads.TabIndex = 13;
            this.richTextBoxHeads.Text = "monkey, sh_male5_head";
            this.richTextBoxHeads.TextChanged += new System.EventHandler(this.RichTextBoxHeads_TextChanged);
            // 
            // label48
            // 
            this.label48.AutoSize = true;
            this.label48.Location = new System.Drawing.Point(3, 26);
            this.label48.Name = "label48";
            this.label48.Size = new System.Drawing.Size(38, 13);
            this.label48.TabIndex = 12;
            this.label48.Text = "Heads";
            // 
            // checkBoxEnableWardrobe
            // 
            this.checkBoxEnableWardrobe.AutoSize = true;
            this.checkBoxEnableWardrobe.Location = new System.Drawing.Point(6, 6);
            this.checkBoxEnableWardrobe.Name = "checkBoxEnableWardrobe";
            this.checkBoxEnableWardrobe.Size = new System.Drawing.Size(109, 17);
            this.checkBoxEnableWardrobe.TabIndex = 5;
            this.checkBoxEnableWardrobe.Text = "Enable Wardrobe";
            this.checkBoxEnableWardrobe.UseVisualStyleBackColor = true;
            // 
            // tabPage15
            // 
            this.tabPage15.Controls.Add(this.textBoxRestartMaxRank);
            this.tabPage15.Controls.Add(this.checkBoxEnableRestart);
            this.tabPage15.Controls.Add(this.textBoxRestartMaxCash);
            this.tabPage15.Controls.Add(this.label36);
            this.tabPage15.Controls.Add(this.label35);
            this.tabPage15.Controls.Add(this.richTextBoxRestarts);
            this.tabPage15.Controls.Add(this.label50);
            this.tabPage15.Controls.Add(this.checkBoxEnableRestartCost);
            this.tabPage15.Location = new System.Drawing.Point(4, 22);
            this.tabPage15.Name = "tabPage15";
            this.tabPage15.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage15.Size = new System.Drawing.Size(700, 416);
            this.tabPage15.TabIndex = 15;
            this.tabPage15.Text = "Restarts";
            this.tabPage15.UseVisualStyleBackColor = true;
            // 
            // textBoxRestartMaxRank
            // 
            this.textBoxRestartMaxRank.Location = new System.Drawing.Point(583, 7);
            this.textBoxRestartMaxRank.Name = "textBoxRestartMaxRank";
            this.textBoxRestartMaxRank.Size = new System.Drawing.Size(100, 20);
            this.textBoxRestartMaxRank.TabIndex = 24;
            // 
            // checkBoxEnableRestart
            // 
            this.checkBoxEnableRestart.AutoSize = true;
            this.checkBoxEnableRestart.Location = new System.Drawing.Point(9, 6);
            this.checkBoxEnableRestart.Name = "checkBoxEnableRestart";
            this.checkBoxEnableRestart.Size = new System.Drawing.Size(166, 17);
            this.checkBoxEnableRestart.TabIndex = 10;
            this.checkBoxEnableRestart.Text = "Enable /restart, /showrestarts";
            this.checkBoxEnableRestart.UseVisualStyleBackColor = true;
            // 
            // textBoxRestartMaxCash
            // 
            this.textBoxRestartMaxCash.Location = new System.Drawing.Point(583, 33);
            this.textBoxRestartMaxCash.Name = "textBoxRestartMaxCash";
            this.textBoxRestartMaxCash.Size = new System.Drawing.Size(100, 20);
            this.textBoxRestartMaxCash.TabIndex = 26;
            // 
            // label36
            // 
            this.label36.AutoSize = true;
            this.label36.Location = new System.Drawing.Point(368, 33);
            this.label36.Name = "label36";
            this.label36.Size = new System.Drawing.Size(209, 13);
            this.label36.TabIndex = 25;
            this.label36.Text = "Prohibit restart if player has cash more than";
            // 
            // label35
            // 
            this.label35.AutoSize = true;
            this.label35.Location = new System.Drawing.Point(370, 10);
            this.label35.Name = "label35";
            this.label35.Size = new System.Drawing.Size(207, 13);
            this.label35.TabIndex = 23;
            this.label35.Text = "Prohibit restart if player has rank more than";
            // 
            // richTextBoxRestarts
            // 
            this.richTextBoxRestarts.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxRestarts.DetectUrls = false;
            this.richTextBoxRestarts.Location = new System.Drawing.Point(6, 65);
            this.richTextBoxRestarts.Name = "richTextBoxRestarts";
            this.richTextBoxRestarts.Size = new System.Drawing.Size(684, 345);
            this.richTextBoxRestarts.TabIndex = 16;
            this.richTextBoxRestarts.Text = "zoner, 10000";
            // 
            // label50
            // 
            this.label50.AutoSize = true;
            this.label50.Location = new System.Drawing.Point(6, 49);
            this.label50.Name = "label50";
            this.label50.Size = new System.Drawing.Size(85, 13);
            this.label50.TabIndex = 15;
            this.label50.Text = "Names and Cost";
            // 
            // checkBoxEnableRestartCost
            // 
            this.checkBoxEnableRestartCost.AutoSize = true;
            this.checkBoxEnableRestartCost.Location = new System.Drawing.Point(9, 29);
            this.checkBoxEnableRestartCost.Name = "checkBoxEnableRestartCost";
            this.checkBoxEnableRestartCost.Size = new System.Drawing.Size(120, 17);
            this.checkBoxEnableRestartCost.TabIndex = 14;
            this.checkBoxEnableRestartCost.Text = "Enable Restart Cost";
            this.checkBoxEnableRestartCost.UseVisualStyleBackColor = true;
            // 
            // buttonClose
            // 
            this.buttonClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonClose.Location = new System.Drawing.Point(641, 460);
            this.buttonClose.Name = "buttonClose";
            this.buttonClose.Size = new System.Drawing.Size(75, 23);
            this.buttonClose.TabIndex = 1;
            this.buttonClose.Text = "Close";
            this.buttonClose.UseVisualStyleBackColor = true;
            this.buttonClose.Click += new System.EventHandler(this.ButtonClose_Click);
            // 
            // buttonSave
            // 
            this.buttonSave.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonSave.Location = new System.Drawing.Point(560, 460);
            this.buttonSave.Name = "buttonSave";
            this.buttonSave.Size = new System.Drawing.Size(75, 23);
            this.buttonSave.TabIndex = 2;
            this.buttonSave.Text = "Save";
            this.buttonSave.UseVisualStyleBackColor = true;
            this.buttonSave.Click += new System.EventHandler(this.ButtonSave_Click);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1,
            this.toolStripStatusLabel});
            this.statusStrip1.Location = new System.Drawing.Point(0, 486);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(724, 22);
            this.statusStrip1.TabIndex = 3;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(42, 17);
            this.toolStripStatusLabel1.Text = "Status:";
            // 
            // toolStripStatusLabel
            // 
            this.toolStripStatusLabel.Name = "toolStripStatusLabel";
            this.toolStripStatusLabel.Size = new System.Drawing.Size(0, 17);
            // 
            // button1
            // 
            this.button1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.button1.Location = new System.Drawing.Point(171, 387);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 24;
            this.button1.Text = "Del Item";
            this.button1.UseVisualStyleBackColor = true;
            // 
            // button2
            // 
            this.button2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.button2.Location = new System.Drawing.Point(90, 387);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 23;
            this.button2.Text = "Edit Item";
            this.button2.UseVisualStyleBackColor = true;
            // 
            // button4
            // 
            this.button4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.button4.Location = new System.Drawing.Point(9, 387);
            this.button4.Name = "button4";
            this.button4.Size = new System.Drawing.Size(75, 23);
            this.button4.TabIndex = 22;
            this.button4.Text = "Add Item";
            this.button4.UseVisualStyleBackColor = true;
            // 
            // label38
            // 
            this.label38.AutoSize = true;
            this.label38.Location = new System.Drawing.Point(6, 26);
            this.label38.Name = "label38";
            this.label38.Size = new System.Drawing.Size(311, 13);
            this.label38.TabIndex = 21;
            this.label38.Text = "Prohibit purchase of item unless ship has one of the control items";
            // 
            // dataGridView2
            // 
            this.dataGridView2.AllowUserToAddRows = false;
            this.dataGridView2.AllowUserToDeleteRows = false;
            this.dataGridView2.AllowUserToResizeRows = false;
            this.dataGridView2.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridView2.AutoGenerateColumns = false;
            this.dataGridView2.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridView2.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.dataGridViewTextBoxColumn1,
            this.dataGridViewTextBoxColumn2,
            this.dataGridViewTextBoxColumn3,
            this.dataGridViewTextBoxColumn4});
            this.dataGridView2.DataSource = this.shipPurchaseRestrictionItemsBindingSource;
            this.dataGridView2.Location = new System.Drawing.Point(6, 42);
            this.dataGridView2.Name = "dataGridView2";
            this.dataGridView2.ReadOnly = true;
            this.dataGridView2.RowHeadersVisible = false;
            this.dataGridView2.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
            this.dataGridView2.Size = new System.Drawing.Size(688, 335);
            this.dataGridView2.TabIndex = 20;
            // 
            // dataGridViewTextBoxColumn1
            // 
            this.dataGridViewTextBoxColumn1.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.dataGridViewTextBoxColumn1.DataPropertyName = "Description";
            this.dataGridViewTextBoxColumn1.HeaderText = "Description";
            this.dataGridViewTextBoxColumn1.Name = "dataGridViewTextBoxColumn1";
            this.dataGridViewTextBoxColumn1.ReadOnly = true;
            // 
            // dataGridViewTextBoxColumn2
            // 
            this.dataGridViewTextBoxColumn2.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.dataGridViewTextBoxColumn2.DataPropertyName = "ItemNickName";
            this.dataGridViewTextBoxColumn2.HeaderText = "Item NickName";
            this.dataGridViewTextBoxColumn2.Name = "dataGridViewTextBoxColumn2";
            this.dataGridViewTextBoxColumn2.ReadOnly = true;
            // 
            // dataGridViewTextBoxColumn3
            // 
            this.dataGridViewTextBoxColumn3.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.dataGridViewTextBoxColumn3.DataPropertyName = "ControlDescription";
            this.dataGridViewTextBoxColumn3.HeaderText = "Control Description";
            this.dataGridViewTextBoxColumn3.Name = "dataGridViewTextBoxColumn3";
            this.dataGridViewTextBoxColumn3.ReadOnly = true;
            // 
            // dataGridViewTextBoxColumn4
            // 
            this.dataGridViewTextBoxColumn4.AutoSizeMode = System.Windows.Forms.DataGridViewAutoSizeColumnMode.Fill;
            this.dataGridViewTextBoxColumn4.DataPropertyName = "ControlItemNickName";
            this.dataGridViewTextBoxColumn4.HeaderText = "Control Item NickName";
            this.dataGridViewTextBoxColumn4.Name = "dataGridViewTextBoxColumn4";
            this.dataGridViewTextBoxColumn4.ReadOnly = true;
            // 
            // checkBox1
            // 
            this.checkBox1.AutoSize = true;
            this.checkBox1.Location = new System.Drawing.Point(9, 6);
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.Size = new System.Drawing.Size(163, 17);
            this.checkBox1.TabIndex = 9;
            this.checkBox1.Text = "Enforce purchase restrictions";
            this.checkBox1.UseVisualStyleBackColor = true;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(724, 508);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.buttonSave);
            this.Controls.Add(this.buttonClose);
            this.Controls.Add(this.tabControl1);
            this.MaximumSize = new System.Drawing.Size(740, 1542);
            this.MinimumSize = new System.Drawing.Size(740, 542);
            this.Name = "Form1";
            this.ShowIcon = false;
            this.Text = "Player Control Setup";
            this.Load += new System.EventHandler(this.Form1_Load);
            ((System.ComponentModel.ISupportInitialize)(gameDataSet)).EndInit();
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.groupBox7.ResumeLayout(false);
            this.groupBox7.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox6.ResumeLayout(false);
            this.groupBox6.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.tabPage5.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewShipPimperEquipList)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.shipPimperItemListBindingSource)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.uIDataSet)).EndInit();
            this.tabPage4.ResumeLayout(false);
            this.tabPage4.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewShipPimperBases)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.shipPimperBaseListBindingSource)).EndInit();
            this.tabPage3.ResumeLayout(false);
            this.tabPage3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewRepFixerItems)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.repFixerItemListBindingSource)).EndInit();
            this.tabPage6.ResumeLayout(false);
            this.tabPage6.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewPurchaseRestrictions)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.shipPurchaseRestrictionItemsBindingSource)).EndInit();
            this.tabPage11.ResumeLayout(false);
            this.tabPage11.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewEquipPurchaseRestriction)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.equipPurchaseRestrictionItemsBindingSource)).EndInit();
            this.tabPage10.ResumeLayout(false);
            this.tabPage10.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewNoBuyList)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.baseGoodNoBuyListBindingSource)).EndInit();
            this.tabPage7.ResumeLayout(false);
            this.tabPage7.PerformLayout();
            this.tabPage8.ResumeLayout(false);
            this.tabPage8.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewStandardBanners)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.standardBannerListBindingSource)).EndInit();
            this.tabPage9.ResumeLayout(false);
            this.tabPage9.PerformLayout();
            this.tabPage12.ResumeLayout(false);
            this.tabPage12.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridViewSysSensor)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.sysSensorListBindingSource)).EndInit();
            this.tabPage13.ResumeLayout(false);
            this.tabPage13.PerformLayout();
            this.tabPage14.ResumeLayout(false);
            this.tabPage14.PerformLayout();
            this.tabPage15.ResumeLayout(false);
            this.tabPage15.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView2)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.Button buttonClose;
        private System.Windows.Forms.Button buttonSave;
        private System.Windows.Forms.CheckBox checkBoxEnablePimpShip;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.CheckBox checkBoxEnableDropRepAndMisc;
        private System.Windows.Forms.CheckBox checkBoxEnableRestart;
        private System.Windows.Forms.TabPage tabPage4;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.DataGridView dataGridViewRepFixerItems;
        private System.Windows.Forms.CheckBox checkBoxEnableRepFixUpdates;
        private System.Windows.Forms.CheckBox checkBoxLogRepFixUpdates;
        private System.Windows.Forms.CheckBox checkBoxItemMustBeMounted;
        private System.Windows.Forms.Button buttonRepEditItem;
        private System.Windows.Forms.Button buttonRepAddItem;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.DataGridView dataGridViewShipPimperEquipList;
        private UIDataSet uIDataSet;
        private System.Windows.Forms.Button buttonAddItem;
        private System.Windows.Forms.Button buttonDelItem;
        private System.Windows.Forms.BindingSource shipPimperItemListBindingSource;
        private System.Windows.Forms.BindingSource shipPimperBaseListBindingSource;
        private System.Windows.Forms.DataGridViewTextBoxColumn iDSNameDataGridViewTextBoxColumn1;
        private System.Windows.Forms.DataGridViewTextBoxColumn itemNickNameDataGridViewTextBoxColumn1;
        private System.Windows.Forms.DataGridViewTextBoxColumn itemHashDataGridViewTextBoxColumn1;
        private System.Windows.Forms.DataGridViewTextBoxColumn iDSInfoDataGridViewTextBoxColumn1;
        private System.Windows.Forms.DataGridViewTextBoxColumn iDSInfo1DataGridViewTextBoxColumn1;
        private System.Windows.Forms.Button buttonDelBase;
        private System.Windows.Forms.Button buttonAddBase;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.DataGridView dataGridViewShipPimperBases;
        private System.Windows.Forms.DataGridViewTextBoxColumn iDSNameDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn itemNickNameDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn itemHashDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn iDSInfoDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn iDSInfo1DataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn iDSNameDataGridViewTextBoxColumn2;
        private System.Windows.Forms.DataGridViewTextBoxColumn itemNickNameDataGridViewTextBoxColumn2;
        private System.Windows.Forms.DataGridViewTextBoxColumn itemHashDataGridViewTextBoxColumn2;
        private System.Windows.Forms.DataGridViewTextBoxColumn iDSInfoDataGridViewTextBoxColumn2;
        private System.Windows.Forms.DataGridViewTextBoxColumn iDSInfo1DataGridViewTextBoxColumn2;
        private System.Windows.Forms.BindingSource repFixerItemListBindingSource;
        private System.Windows.Forms.TabPage tabPage5;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.CheckBox checkBoxEnableRenameMe;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.TextBox textBoxRenameCost;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.TextBox textBoxMoveCost;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.TextBox textBoxRenameTimeLimit;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.CheckBox checkBoxEnableGiveCash;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.TextBox textBoxBlockedSystem;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox textBoxMinTime;
        private System.Windows.Forms.TextBox textBoxMinTransfer;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.CheckBox checkBoxEnableMoveChar;
        private System.Windows.Forms.TabPage tabPage6;
        private System.Windows.Forms.CheckBox checkBoxEnforceIDRestrictions;
        private System.Windows.Forms.GroupBox groupBox6;
        private System.Windows.Forms.TextBox textBoxHullDrop2NickName;
        private System.Windows.Forms.TextBox textBoxDropRepCost;
        private System.Windows.Forms.Label label18;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.TextBox textBoxHullDrop1NickName;
        private System.Windows.Forms.Label label16;
        private System.Windows.Forms.Label label17;
        private System.Windows.Forms.TextBox textBoxCargoDropContainer;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Label label19;
        private System.Windows.Forms.TextBox textBoxDisconnectMsg;
        private System.Windows.Forms.CheckBox checkBoxReportDisconnectingPlayers;
        private System.Windows.Forms.CheckBox checkBoxKillDisconnectingPlayers;
        private System.Windows.Forms.CheckBox checkBoxLootDisconnectingPlayers;
        private System.Windows.Forms.TextBox textBoxPimpShipCost;
        private System.Windows.Forms.Label label20;
        private System.Windows.Forms.TextBox textBoxHullDropFactor;
        private System.Windows.Forms.Label label21;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel;
        private System.Windows.Forms.Label label24;
        private System.Windows.Forms.Label label23;
        private System.Windows.Forms.TextBox textBoxSmiteMusic;
        private System.Windows.Forms.TextBox textBoxCoinMsg;
        private System.Windows.Forms.TextBox textBoxDiceMsg;
        private System.Windows.Forms.Label label22;
        private System.Windows.Forms.Label label25;
        private System.Windows.Forms.TextBox textBoxStuckMsg;
        private System.Windows.Forms.Button buttonPurchaseRestrictionAddItem;
        private System.Windows.Forms.Label label26;
        private System.Windows.Forms.DataGridView dataGridViewPurchaseRestrictions;
        private System.Windows.Forms.Button buttonRepDelItem;
        private System.Windows.Forms.Button buttonPurchaseRestrictionDelItem;
        private System.Windows.Forms.TabPage tabPage7;
        private System.Windows.Forms.CheckBox checkBoxCustomHelp;
        private System.Windows.Forms.Label label27;
        private System.Windows.Forms.Label label28;
        private System.Windows.Forms.CheckBox checkBoxCmdHide;
        private System.Windows.Forms.CheckBox checkBoxCmdEcho;
        private System.Windows.Forms.TextBox textBoxCmdEchoStyle;
        private System.Windows.Forms.TabPage tabPage8;
        private System.Windows.Forms.Label label31;
        private System.Windows.Forms.DataGridView dataGridViewStandardBanners;
        private System.Windows.Forms.Button buttonBannerDelItem;
        private System.Windows.Forms.Button buttonBannerEditItem;
        private System.Windows.Forms.Button buttonBannerAddItem;
        private System.Windows.Forms.BindingSource standardBannerListBindingSource;
        private System.Windows.Forms.DataGridViewTextBoxColumn bannerTextDataGridViewTextBoxColumn;
        private System.Windows.Forms.Button buttonPurchaseRestrictionEdittem;
        private System.Windows.Forms.DataGridViewTextBoxColumn descriptionDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn itemNickNameDataGridViewTextBoxColumn3;
        private System.Windows.Forms.DataGridViewTextBoxColumn controlDescriptionDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn controlItemNickNameDataGridViewTextBoxColumn;
        private System.Windows.Forms.BindingSource shipPurchaseRestrictionItemsBindingSource;
        private System.Windows.Forms.Label label32;
        private System.Windows.Forms.TextBox textBoxStandardBannerDelay;
        private System.Windows.Forms.RichTextBox richTextBoxHelpLines;
        private System.Windows.Forms.TabPage tabPage9;
        private System.Windows.Forms.RichTextBox richTextBoxSpecialBannerLines;
        private System.Windows.Forms.Label label33;
        private System.Windows.Forms.TextBox textBoxSpecialBannerDelay;
        private System.Windows.Forms.Label label30;
        private System.Windows.Forms.RichTextBox richTextBoxGreetingBannerLines;
        private System.Windows.Forms.Label label29;
        private System.Windows.Forms.RichTextBox richTextBox1;
        private System.Windows.Forms.TextBox textBoxDisconnectingPlayersRange;
        private System.Windows.Forms.Label label34;
        private System.Windows.Forms.TextBox textBoxRestartMaxCash;
        private System.Windows.Forms.Label label36;
        private System.Windows.Forms.TextBox textBoxRestartMaxRank;
        private System.Windows.Forms.Label label35;
        private System.Windows.Forms.TabPage tabPage10;
        private System.Windows.Forms.Label label37;
        private System.Windows.Forms.DataGridView dataGridViewNoBuyList;
        private System.Windows.Forms.BindingSource baseGoodNoBuyListBindingSource;
        private System.Windows.Forms.DataGridViewTextBoxColumn baseDescriptionDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn baseNickNameDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn goodDescriptionDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn goodNickNameDataGridViewTextBoxColumn;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button button4;
        private System.Windows.Forms.Label label38;
        private System.Windows.Forms.DataGridView dataGridView2;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn1;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn2;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn3;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn4;
        private System.Windows.Forms.CheckBox checkBox1;
        private System.Windows.Forms.Button buttonAddNoBuyItem;
        private System.Windows.Forms.Button buttonEditNoBuyItem;
        private System.Windows.Forms.Button buttonDelNoBuyItem;
        private System.Windows.Forms.Button buttonImportShipPurchaseRestrictions;
        private System.Windows.Forms.CheckBox checkBoxCheckIDRestrictions;
        private System.Windows.Forms.TabPage tabPage11;
        private System.Windows.Forms.Label label39;
        private System.Windows.Forms.DataGridView dataGridViewEquipPurchaseRestriction;
        private System.Windows.Forms.BindingSource equipPurchaseRestrictionItemsBindingSource;
        private System.Windows.Forms.Button buttonEquipPurchaseRestrictionDel;
        private System.Windows.Forms.Button buttonEquipPurchaseRestrictionEdit;
        private System.Windows.Forms.Button buttonEquipPurchaseRestrictionAdd;
        private System.Windows.Forms.DataGridViewTextBoxColumn descriptionDataGridViewTextBoxColumn1;
        private System.Windows.Forms.DataGridViewTextBoxColumn itemNickNameDataGridViewTextBoxColumn4;
        private System.Windows.Forms.DataGridViewTextBoxColumn controlDescriptionDataGridViewTextBoxColumn1;
        private System.Windows.Forms.DataGridViewTextBoxColumn controlItemNickNameDataGridViewTextBoxColumn1;
        private System.Windows.Forms.TabPage tabPage12;
        private System.Windows.Forms.Button buttonSysSensorDelItem;
        private System.Windows.Forms.Button buttonSysSensorEditItem;
        private System.Windows.Forms.Button buttonSysSensorAddItem;
        private System.Windows.Forms.Label label40;
        private System.Windows.Forms.DataGridView dataGridViewSysSensor;
        private System.Windows.Forms.BindingSource sysSensorListBindingSource;
        private System.Windows.Forms.DataGridViewTextBoxColumn systemDescriptionDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn systemNickNameDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn equipDescriptionDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn equipNickNameDataGridViewTextBoxColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn NetworkID;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TextBox textBoxSpinProtectMass;
        private System.Windows.Forms.Label label41;
        private System.Windows.Forms.TextBox textBoxSpinImpulseMultiplier;
        private System.Windows.Forms.Label S;
        private System.Windows.Forms.TextBox textBoxEscortedShipDamageFactor;
        private System.Windows.Forms.Label label43;
        private System.Windows.Forms.Label label42;
        private System.Windows.Forms.TextBox textBoxEscortedShipMinimumMass;
        private System.Windows.Forms.CheckBox checkBoxEnableEscort;
        private System.Windows.Forms.RichTextBox richTextBoxSwearWords;
        private System.Windows.Forms.Label label44;
        private System.Windows.Forms.CheckBox checkBoxCharnameTag;
        private System.Windows.Forms.Label label46;
        private System.Windows.Forms.TextBox textBoxCharnameTagCost;
        private System.Windows.Forms.Label label45;
        private System.Windows.Forms.CheckBox checkBoxAsciiCharnameOnly;
        private System.Windows.Forms.TabPage tabPage13;
        private System.Windows.Forms.RichTextBox richTextBoxSounds;
        private System.Windows.Forms.Label label47;
        private System.Windows.Forms.CheckBox checkBoxEnableLoginSound;
        private System.Windows.Forms.GroupBox groupBox7;
        private System.Windows.Forms.CheckBox checkBoxEnableDo;
        private System.Windows.Forms.CheckBox checkBoxEnableMe;
        private System.Windows.Forms.TabPage tabPage14;
        private System.Windows.Forms.RichTextBox richTextBoxBodies;
        private System.Windows.Forms.Label label49;
        private System.Windows.Forms.RichTextBox richTextBoxHeads;
        private System.Windows.Forms.Label label48;
        private System.Windows.Forms.CheckBox checkBoxEnableWardrobe;
        private System.Windows.Forms.TabPage tabPage15;
        private System.Windows.Forms.RichTextBox richTextBoxRestarts;
        private System.Windows.Forms.Label label50;
        private System.Windows.Forms.CheckBox checkBoxEnableRestartCost;
    }
}

