using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace PlayerCntlSetup
{
    public partial class Form1 : Form, LogRecorderInterface
    {
        FLGameData gameData = new FLGameData();

        string flExePath = "C:\\Program Files\\Microsoft Games\\Freelancer\\EXE";
        string cfgFilePath = "playercntl.cfg";

        FLDataFile cfgFile = new FLDataFile(false);
        BackgroundWorker bgWkr = null;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            // This application wants to be started from the flhook_plugins directory. Check to
            // see if it is in the correct location and it can find the Freelancer.ini 
            // and playercntl.ini
            if (File.Exists("..\\freelancer.ini"))
            {
                flExePath = "..";
            }
            // This is really here for dev purposes.
            else if (File.Exists("C:\\Program Files\\Microsoft Games\\Freelancer\\EXE\\freelancer.ini"))
            {
                flExePath = "C:\\Program Files\\Microsoft Games\\Freelancer\\EXE";
            }
            else
            {
                MessageBox.Show("freelancer.ini not found.\rStart this program from the flhook_plugins directory.");           
            }

            buttonRepAddItem.Enabled = false;
            buttonRepEditItem.Enabled = false;
            buttonRepDelItem.Enabled = false;

            button3.Enabled = false;
            buttonAddBase.Enabled = false;
            buttonDelBase.Enabled = false;
            buttonAddItem.Enabled = false;
            buttonDelItem.Enabled = false;
            buttonSave.Enabled = false;

            buttonPurchaseRestrictionAddItem.Enabled = false;
            buttonPurchaseRestrictionEdittem.Enabled = false;
            buttonPurchaseRestrictionDelItem.Enabled = false;

            buttonEquipPurchaseRestrictionAdd.Enabled = false;
            buttonEquipPurchaseRestrictionEdit.Enabled = false;
            buttonEquipPurchaseRestrictionDel.Enabled = false;

            buttonSysSensorEditItem.Enabled = false;
            buttonSysSensorAddItem.Enabled = false;
            buttonSysSensorDelItem.Enabled = false;

            bgWkr = new BackgroundWorker();
            bgWkr.DoWork += new DoWorkEventHandler(Loader);
            bgWkr.WorkerReportsProgress = true;
            bgWkr.ProgressChanged += new ProgressChangedEventHandler(Bg_ProgressChanged);
            bgWkr.RunWorkerCompleted += new RunWorkerCompletedEventHandler(BgWkr_RunWorkerCompleted);
            bgWkr.RunWorkerAsync();
        }

        void LogRecorderInterface.AddLog(string entry)
        {
            if (bgWkr != null)
            {
                bgWkr.ReportProgress(0, entry);
            }
        }

        void Bg_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            toolStripStatusLabel.Text = (string)e.UserState;
        }

        // Load the game data
        void Loader(object sender, DoWorkEventArgs e)
        {
            bgWkr.ReportProgress(0, "Loading game data");
            gameData.LoadAll(flExePath, this);
            bgWkr.ReportProgress(0, "Game data loaded");
        }
        
        void  BgWkr_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            bgWkr = null;

            buttonRepAddItem.Enabled = true;
            buttonRepEditItem.Enabled = true;
            buttonRepDelItem.Enabled = true;
            button3.Enabled = true;
            buttonAddBase.Enabled = true;
            buttonDelBase.Enabled = true;
            buttonAddItem.Enabled = true;
            buttonDelItem.Enabled = true;
            buttonSave.Enabled = true;
            buttonPurchaseRestrictionAddItem.Enabled = true;
            buttonPurchaseRestrictionEdittem.Enabled = true;
            buttonPurchaseRestrictionDelItem.Enabled = true;
            buttonEquipPurchaseRestrictionAdd.Enabled = true;
            buttonEquipPurchaseRestrictionEdit.Enabled = true;
            buttonEquipPurchaseRestrictionDel.Enabled = true;
            buttonSysSensorEditItem.Enabled = true;
            buttonSysSensorAddItem.Enabled = true;
            buttonSysSensorDelItem.Enabled = true;

            // Load our ini file
            try
            {
                cfgFilePath = Path.GetFullPath(cfgFilePath);
                cfgFile = new FLDataFile(cfgFilePath, false);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error '" + ex.Message + "' occured when loading the configuration file. Using defaults.");
            }

            try
            {
                checkBoxEnableMoveChar.Checked = cfgFile.GetSettingBool("General", "EnableMoveChar", true);
                checkBoxEnableRenameMe.Checked = cfgFile.GetSettingBool("General", "EnableRenameMe", true);
                checkBoxEnablePimpShip.Checked = cfgFile.GetSettingBool("General", "EnablePimpShip", true);
                checkBoxEnableRestart.Checked = cfgFile.GetSettingBool("General", "EnableRestart", true);
                checkBoxEnableGiveCash.Checked = cfgFile.GetSettingBool("General", "EnableGiveCash", true);
                checkBoxEnableDropRepAndMisc.Checked = cfgFile.GetSettingBool("General", "EnableDropRep", true);

                textBoxDropRepCost.Text = cfgFile.GetSettingStr("General", "RepDropCost", "350000");
                textBoxStuckMsg.Text = cfgFile.GetSettingStr("General", "StuckMsg", "Attention! Stand clear. Towing %player");
                textBoxDiceMsg.Text = cfgFile.GetSettingStr("General", "DiceMsg", "%player rolled %number");
                textBoxCoinMsg.Text = cfgFile.GetSettingStr("General", "CoinMsg", "%player tossed %result");
                textBoxSmiteMusic.Text = cfgFile.GetSettingStr("General", "SmiteMusic", "music_danger");

                textBoxDisconnectingPlayersRange.Text = cfgFile.GetSettingStr("General", "DisconnectingPlayersRange", "5000");
                checkBoxReportDisconnectingPlayers.Checked = cfgFile.GetSettingBool("General", "ReportDisconnectingPlayers", true);
                checkBoxKillDisconnectingPlayers.Checked = cfgFile.GetSettingBool("General", "KillDisconnectingPlayers", true);
                checkBoxLootDisconnectingPlayers.Checked = cfgFile.GetSettingBool("General", "LootDisconnectingPlayers", true);

                textBoxHullDropFactor.Text = cfgFile.GetSettingStr("General", "HullDropFactor", "0.0");
                textBoxCargoDropContainer.Text = cfgFile.GetSettingStr("General", "CargoDropContainer", "lootcrate_ast_loot_metal");
                textBoxHullDrop1NickName.Text = cfgFile.GetSettingStr("General", "HullDrop1NickName", "commodity_super_alloys");
                textBoxHullDrop2NickName.Text = cfgFile.GetSettingStr("General", "HullDrop2NickName", "commodity_engine_components");
                textBoxDisconnectMsg.Text = cfgFile.GetSettingStr("General", "DisconnectMsg", "%player is attempting to engage cloaking device");

                textBoxSpinProtectMass.Text = cfgFile.GetSettingStr("General", "SpinProtectionMass", "-1.0");
                textBoxSpinImpulseMultiplier.Text = cfgFile.GetSettingStr("General", "SpinProtectionMultiplier", "-8.0");

                checkBoxEnableEscort.Checked = cfgFile.GetSettingBool("General", "EnableEscort", false);
                textBoxEscortedShipMinimumMass.Text = cfgFile.GetSettingStr("General", "EscortedShipMinimumMass", "100000");
                textBoxEscortedShipDamageFactor.Text = cfgFile.GetSettingStr("General", "EscortedShipDamageFactor", "0.3");

                textBoxRestartMaxRank.Text = cfgFile.GetSettingStr("Restart", "MaxRank", "5");
                textBoxRestartMaxCash.Text = cfgFile.GetSettingStr("Restart", "MaxCash", "1000000");

                textBoxRenameCost.Text = cfgFile.GetSettingStr("Rename", "RenameCost", "2000000");
                textBoxRenameTimeLimit.Text = cfgFile.GetSettingStr("Rename", "RenameTimeLimit", "86400");
                textBoxMoveCost.Text = cfgFile.GetSettingStr("Rename", "MoveCost", "2000000");
                checkBoxCharnameTag.Checked = cfgFile.GetSettingBool("Rename", "CharnameTag", false);
                textBoxCharnameTagCost.Text = cfgFile.GetSettingStr("Rename", "MakeTagCost", "50000000");
                checkBoxAsciiCharnameOnly.Checked = cfgFile.GetSettingBool("Rename", "AsciiCharnameOnly", false);
                

                textBoxMinTransfer.Text = cfgFile.GetSettingStr("GiveCash", "MinTransfer", "100000");
                textBoxMinTime.Text = cfgFile.GetSettingStr("GiveCash", "MinTime", "3600");
                textBoxBlockedSystem.Text = cfgFile.GetSettingStr("GiveCash", "BlockedSystem", "iw09");

                textBoxPimpShipCost.Text = cfgFile.GetSettingStr("ShipPimper", "cost", "2000000");
                foreach (FLDataFile.Setting set in cfgFile.GetSettings("ShipPimper", "equip"))
                {
                    string[] args = set.Str(0).Split(',');
                    if (args.Length > 0)
                    {
                        GameDataSet.HashListRow item = gameData.GetItemByNickName(args[0]);
                        if (item != null)
                        {
                            uIDataSet.ShipPimperItemList.AddShipPimperItemListRow(item.IDSName, item.ItemNickName, item.ItemHash, item.IDSInfo, item.IDSInfo1);
                        }
                    }
                }

                foreach (FLDataFile.Setting set in cfgFile.GetSettings("ShipPimper", "room"))
                {
                    GameDataSet.HashListRow item = gameData.GetItemByNickName(set.Str(0));
                    if (item != null)
                    {
                        uIDataSet.ShipPimperBaseList.AddShipPimperBaseListRow(item.IDSName, item.ItemNickName, item.ItemHash, item.IDSInfo, item.IDSInfo1);
                    }
                }

                checkBoxEnableRepFixUpdates.Checked = cfgFile.GetSettingBool("RepFixer", "EnableRepFixUpdates", true);
                checkBoxLogRepFixUpdates.Checked = cfgFile.GetSettingBool("RepFixer", "LogRepFixUpdates", true);
                checkBoxItemMustBeMounted.Checked = cfgFile.GetSettingBool("RepFixer", "ItemMustBeMounted", true);

                foreach (FLDataFile.Setting set in cfgFile.GetSettings("RepFixerItems"))
                {
                    GameDataSet.HashListRow item = gameData.GetItemByNickName(set.settingName);
                    if (item != null)
                    {
                        uIDataSet.RepFixerItemList.AddRepFixerItemListRow(item.IDSName, item.ItemNickName, item.ItemHash, item.IDSInfo, item.IDSInfo1);
                    }
                }

                checkBoxCheckIDRestrictions.Checked = cfgFile.GetSettingBool("PurchaseRestrictions", "CheckIDRestrictions", false);
                checkBoxEnforceIDRestrictions.Checked = cfgFile.GetSettingBool("PurchaseRestrictions", "EnforceIDRestrictions", false);
                foreach (FLDataFile.Setting set in cfgFile.GetSettings("ShipItemRestrictions"))
                {
                    GameDataSet.HashListRow shipItem = gameData.GetItemByNickName(set.settingName);
                    GameDataSet.HashListRow controlItem = gameData.GetItemByNickName(set.Str(0));
                    if (shipItem != null && controlItem != null)
                    {
                        uIDataSet.ShipPurchaseRestrictionItems.AddShipPurchaseRestrictionItemsRow(shipItem.IDSName, shipItem.ItemNickName, controlItem.IDSName, controlItem.ItemNickName);
                    }
                }

                foreach (FLDataFile.Setting set in cfgFile.GetSettings("GoodItemRestrictions"))
                {
                    GameDataSet.HashListRow equipItem = gameData.GetItemByNickName(set.settingName);
                    GameDataSet.HashListRow controlItem = gameData.GetItemByNickName(set.Str(0));
                    if (equipItem != null && controlItem != null)
                    {
                        uIDataSet.EquipPurchaseRestrictionItems.AddEquipPurchaseRestrictionItemsRow(equipItem.IDSName, equipItem.ItemNickName, controlItem.IDSName, controlItem.ItemNickName);
                    }
                }

                foreach (FLDataFile.Setting set in cfgFile.GetSettings("NoBuy"))
                {
                    GameDataSet.HashListRow baseItem = gameData.GetItemByNickName(set.settingName);
                    GameDataSet.HashListRow goodItem = gameData.GetItemByNickName(set.Str(0));
                    if (baseItem != null && goodItem != null)
                    {
                        uIDataSet.BaseGoodNoBuyList.AddBaseGoodNoBuyListRow(baseItem.IDSName, baseItem.ItemNickName, goodItem.IDSName, goodItem.ItemNickName);
                    }
                }

                foreach (FLDataFile.Setting set in cfgFile.GetSettings("SystemSensor"))
                {
                    GameDataSet.HashListRow sysItem = gameData.GetItemByNickName(set.settingName);
                    string[] args = set.Str(0).Split(',');
                    if (args.Length > 0)
                    {
                        GameDataSet.HashListRow equipItem = gameData.GetItemByNickName(args[0]);
                        uint networkID = 1;
                        if (args.Length > 1)
                            networkID = UInt32.Parse(args[1]);
                        if (sysItem != null && equipItem != null)
                        {
                            uIDataSet.SysSensorList.AddSysSensorListRow(sysItem.IDSName, sysItem.ItemNickName, equipItem.IDSName, equipItem.ItemNickName, networkID);
                        }
                    }
                }

                checkBoxCustomHelp.Checked = cfgFile.GetSettingBool("Message", "CustomHelp", true);
                checkBoxCmdEcho.Checked = cfgFile.GetSettingBool("Message", "CmdEcho", true);
                checkBoxCmdHide.Checked = cfgFile.GetSettingBool("Message", "CmdHide", true);
                textBoxCmdEchoStyle.Text = cfgFile.GetSettingStr("Message", "CmdEchoStyle", "0x00AA0090");

                richTextBoxHelpLines.Clear();
                foreach (FLDataFile.Setting set in cfgFile.GetSettings("Help"))
                {
                    richTextBoxHelpLines.AppendText(set.settingName);
                    richTextBoxHelpLines.AppendText("\n");
                }

                richTextBoxSwearWords.Clear();
                foreach (FLDataFile.Setting set in cfgFile.GetSettings("SwearWords"))
                {
                    richTextBoxSwearWords.AppendText(set.Str(0));
                    richTextBoxSwearWords.AppendText("\n");
                }

                richTextBoxGreetingBannerLines.Clear();
                foreach (FLDataFile.Setting set in cfgFile.GetSettings("GreetingBanner"))
                {
                    richTextBoxGreetingBannerLines.AppendText(set.Str(0));
                    richTextBoxGreetingBannerLines.AppendText("\n");
                }

                textBoxSpecialBannerDelay.Text = cfgFile.GetSettingStr("Message", "SpecialBannerDelay", "3000");
                textBoxStandardBannerDelay.Text = cfgFile.GetSettingStr("Message", "StandardBannerDelay", "1200");

                richTextBoxSpecialBannerLines.Clear();
                foreach (FLDataFile.Setting set in cfgFile.GetSettings("SpecialBanner"))
                {
                    richTextBoxSpecialBannerLines.AppendText(set.Str(0));
                    richTextBoxSpecialBannerLines.AppendText("\n");
                }

                uIDataSet.StandardBannerList.Clear();
                foreach (FLDataFile.Section sect in cfgFile.sections)
                {
                    if (sect.sectionName == "StandardBanner")
                    {
                        StringBuilder sb = new StringBuilder();
                        foreach (FLDataFile.Setting set in sect.settings)
                        {
                            sb.AppendLine(set.Str(0));
                        }
                        uIDataSet.StandardBannerList.AddStandardBannerListRow(sb.ToString());
                    }
                }

                checkBoxEnableLoginSound.Checked = cfgFile.GetSettingBool("General", "EnableLoginSound", false);
                richTextBoxSounds.Clear();
                foreach (FLDataFile.Setting set in cfgFile.GetSettings("Sounds"))
                {
                    richTextBoxSounds.AppendText(set.Str(0));
                    richTextBoxSounds.AppendText("\n");
                }
                if (richTextBoxSounds.TextLength == 0)
                {
                    richTextBoxSounds.AppendText(
                        "dx_s075x_03a03_or_pilot_03\n" +
                        "dx_s032a_01a01_hvis_xtr_1\n" +
                        "dx_s075x_03a01_or_pilot_01\n" +
                        "dx_s075x_03a02_or_pilot_02\n" +
                        "dx_s003x_0801_trent\n" +
                        "dx_s003x_0802_trent\n" +
                        "dx_s004x_1302_juni\n" +
                        "dx_s009d_0101_trent\n" +
                        "dx_s071c_0101_trent"
                       );
                }

                checkBoxEnableWardrobe.Checked = cfgFile.GetSettingBool("General", "EnableWardrobe", false);
                richTextBoxHeads.Clear();
                richTextBoxBodies.Clear();
                foreach (FLDataFile.Setting set in cfgFile.GetSettings("Wardrobe"))
                {
                    if (set.settingName == "head")
                    {
                        richTextBoxHeads.AppendText(set.Str(0));
                        richTextBoxHeads.AppendText("\n");
                    }
                    else if (set.settingName == "body")
                    {
                        richTextBoxBodies.AppendText(set.Str(0));
                        richTextBoxBodies.AppendText("\n");
                    }
                   
                }
                if (richTextBoxHeads.TextLength == 0)
                {
                    richTextBoxHeads.AppendText("monkey, sh_male5_head");
                }
                if (richTextBoxBodies.TextLength == 0)
                {
                    richTextBoxBodies.AppendText("orillion, pi_orillion_body");
                }

                checkBoxEnableMe.Checked = cfgFile.GetSettingBool("General", "EnableMe", false);
                checkBoxEnableDo.Checked = cfgFile.GetSettingBool("General", "EnableDo", false);

                checkBoxEnableRestartCost.Checked = cfgFile.GetSettingBool("General", "EnableRestartCost", false);
                richTextBoxRestarts.Clear();
                foreach (FLDataFile.Setting set in cfgFile.GetSettings("Restart"))
                {
                    if (set.settingName == "restart")
                    {
                        richTextBoxRestarts.AppendText(set.Str(0));
                        richTextBoxRestarts.AppendText("\n");
                    }
                }
                if (richTextBoxRestarts.TextLength == 0)
                {
                    richTextBoxRestarts.AppendText("zoner, 10000");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error '" + ex.Message + "' occured when parsing the configuration file.");
            }

            
        }

        private void ButtonClose_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void ButtonSave_Click(object sender, EventArgs e)
        {
            cfgFile.AddSetting("Settings", "dllname", new object[] { "playercntl.dll" });
            cfgFile.AddSetting("Settings", "name", new object[] { "Player Control v1.0 by Cannon" } );
            cfgFile.AddSetting("Settings", "shortname", new object[] { "playercntl" });
            cfgFile.AddSetting("Settings", "maypause", new object[] { "false" } );
            cfgFile.AddSetting("Settings", "mayunload", new object[] { "true" } );

            cfgFile.DeleteSection("Hooks");
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "LoadSettings, 3" } );
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "ClearClientInfo, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkTimerCheckKick, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "SendDeathMsg, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::PlayerLaunch, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::BaseEnter, 3" } );
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::LocationEnter, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::DisConnect, 3" } );
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::CharacterSelect, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::JumpInComplete, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::SystemSwitchOutComplete, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::SPObjCollision, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::GFGoodBuy, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::ReqAddItem, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::ReqChangeCash, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::ReqSetCash, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::ReqEquipment, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::ReqHullStatus, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::ReqShipArch, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::TractorObjects, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::SetTarget, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::CharacterInfoReq, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::SubmitChat, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::GoTradelane, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIServerImpl::StopTradelane, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkIEngine::Dock_Call, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkCb_AddDmgEntry, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "HkCb_SendChat, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "UserCmd_Process, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "UserCmd_Help, 3" });
            cfgFile.AddSettingNotUnique("Hooks", "hook", new object[] { "ExecuteCommandString_Callback, 3" });

            cfgFile.AddSetting("General", "EnableMoveChar", new object[] { checkBoxEnableMoveChar.Checked });
            cfgFile.AddSetting("General", "EnableRenameMe", new object[] { checkBoxEnableRenameMe.Checked });
            cfgFile.AddSetting("General", "EnablePimpShip", new object[] { checkBoxEnablePimpShip.Checked });
            cfgFile.AddSetting("General", "EnableRestart", new object[] { checkBoxEnableRestart.Checked });
            cfgFile.AddSetting("General", "EnableGiveCash", new object[] { checkBoxEnableGiveCash.Checked });
            cfgFile.AddSetting("General", "EnableDropRepAndMisc", new object[] { checkBoxEnableDropRepAndMisc.Checked });
            cfgFile.AddSetting("General", "EnableLoginSound", new object[] { checkBoxEnableLoginSound.Checked });
            cfgFile.AddSetting("General", "EnableWardrobe", new object[] { checkBoxEnableWardrobe.Checked });
            cfgFile.AddSetting("General", "EnableRestartCost", new object[] { checkBoxEnableRestartCost.Checked });
            cfgFile.AddSetting("General", "EnableMe", new object[] { checkBoxEnableMe.Checked });
            cfgFile.AddSetting("General", "EnableDo", new object[] { checkBoxEnableDo.Checked });

            cfgFile.AddSetting("General", "RepDropCost", new object[] { textBoxDropRepCost.Text });
            cfgFile.AddSetting("General", "StuckMsg", new object[] { textBoxStuckMsg.Text });
            cfgFile.AddSetting("General", "DiceMsg", new object[] { textBoxDiceMsg.Text });
            cfgFile.AddSetting("General", "CoinMsg", new object[] { textBoxCoinMsg.Text });
            cfgFile.AddSetting("General", "SmiteMusic", new object[] { textBoxSmiteMusic.Text });

            cfgFile.AddSetting("General", "DisconnectingPlayersRange", new object[] { textBoxDisconnectingPlayersRange.Text });
            cfgFile.AddSetting("General", "ReportDisconnectingPlayers", new object[] { checkBoxReportDisconnectingPlayers.Checked });
            cfgFile.AddSetting("General", "KillDisconnectingPlayers", new object[] { checkBoxKillDisconnectingPlayers.Checked });
            cfgFile.AddSetting("General", "LootDisconnectingPlayers", new object[] { checkBoxLootDisconnectingPlayers.Checked });
            cfgFile.AddSetting("General", "ReportDisconnectingPlayers", new object[] { checkBoxReportDisconnectingPlayers.Checked });
            cfgFile.AddSetting("General", "HullDropFactor", new object[] { textBoxHullDropFactor.Text });
            cfgFile.AddSetting("General", "CargoDropContainer", new object[] { textBoxCargoDropContainer.Text });
            cfgFile.AddSetting("General", "HullDrop1NickName", new object[] { textBoxHullDrop1NickName.Text});
            cfgFile.AddSetting("General", "HullDrop2NickName", new object[] { textBoxHullDrop2NickName.Text});
            cfgFile.AddSetting("General", "DisconnectMsg", new object[] { textBoxDisconnectMsg.Text});

            cfgFile.AddSetting("General", "SpinProtectionMass", new object[] { textBoxSpinProtectMass.Text });
            cfgFile.AddSetting("General", "SpinProtectionMultiplier", new object[] { textBoxSpinImpulseMultiplier.Text });

            cfgFile.AddSetting("General", "EnableEscort", new object[] { checkBoxEnableEscort.Checked });
            cfgFile.AddSetting("General", "EscortedShipMinimumMass", new object[] { textBoxEscortedShipMinimumMass.Text });
            cfgFile.AddSetting("General", "EscortedShipDamageFactor", new object[] { textBoxEscortedShipDamageFactor.Text });

            cfgFile.AddSetting("Rename", "RenameCost", new object[] { textBoxRenameCost.Text });
            cfgFile.AddSetting("Rename", "RenameTimeLimit", new object[] { textBoxRenameTimeLimit.Text });
            cfgFile.AddSetting("Rename", "MoveCost", new object[] { textBoxMoveCost.Text });
            cfgFile.AddSetting("Rename", "CharnameTag", new object[] { checkBoxCharnameTag.Checked });
            cfgFile.AddSetting("Rename", "MakeTagCost", new object[] { textBoxCharnameTagCost.Text });
            cfgFile.AddSetting("Rename", "AsciiCharnameOnly", new object[] { checkBoxAsciiCharnameOnly.Checked });
            
            cfgFile.AddSetting("GiveCash", "MinTransfer", new object[] { textBoxMinTransfer.Text });
            cfgFile.AddSetting("GiveCash", "MinTime", new object[] { textBoxMinTime.Text });
            cfgFile.AddSetting("GiveCash", "BlockedSystem", new object[] { textBoxBlockedSystem.Text });

            cfgFile.DeleteSection("ShipPimper");
            cfgFile.AddSettingNotUnique("ShipPimper", "cost", new object[] { textBoxPimpShipCost.Text });
            foreach (UIDataSet.ShipPimperItemListRow row in uIDataSet.ShipPimperItemList.Rows)
            {
                cfgFile.AddSettingNotUnique("ShipPimper", "equip", new object[] { row.ItemNickName } );
            }
            foreach (UIDataSet.ShipPimperBaseListRow row in uIDataSet.ShipPimperBaseList.Rows)
            {
                cfgFile.AddSettingNotUnique("ShipPimper", "room", new object[] { row.ItemNickName });
            }

            cfgFile.AddSetting("RepFixer", "EnableRepFixUpdates", new object[] { checkBoxEnableRepFixUpdates.Checked });
            cfgFile.AddSetting("RepFixer", "LogRepFixUpdates", new object[] { checkBoxLogRepFixUpdates.Checked });
            cfgFile.AddSetting("RepFixer", "ItemMustBeMounted", new object[] { checkBoxItemMustBeMounted.Checked });
            cfgFile.DeleteSection("RepFixerItems");
            foreach (UIDataSet.RepFixerItemListRow row in uIDataSet.RepFixerItemList.Rows)
            {
                cfgFile.AddSetting("RepFixerItems", row.ItemNickName, new object[] { });
            }

            
            cfgFile.AddSetting("PurchaseRestrictions", "CheckIDRestrictions", new object[] { checkBoxCheckIDRestrictions.Checked });
            cfgFile.AddSetting("PurchaseRestrictions", "EnforceIDRestrictions", new object[] { checkBoxEnforceIDRestrictions.Checked });
            cfgFile.DeleteSection("ShipItemRestrictions");
            foreach (UIDataSet.ShipPurchaseRestrictionItemsRow row in uIDataSet.ShipPurchaseRestrictionItems.Rows)
            {
                if (row.ControlItemNickName.Length>0)
                    cfgFile.AddSettingNotUnique("ShipItemRestrictions", row.ItemNickName, new object[] { row.ControlItemNickName });
            }

            cfgFile.DeleteSection("GoodItemRestrictions");
            foreach (UIDataSet.EquipPurchaseRestrictionItemsRow row in uIDataSet.EquipPurchaseRestrictionItems.Rows)
            {
                if (row.ControlItemNickName.Length > 0)
                    cfgFile.AddSettingNotUnique("GoodItemRestrictions", row.ItemNickName, new object[] { row.ControlItemNickName });
            }

            cfgFile.DeleteSection("NoBuy");
            foreach (UIDataSet.BaseGoodNoBuyListRow row in uIDataSet.BaseGoodNoBuyList.Rows)
            {
                if (row.GoodNickName.Length > 0)
                    cfgFile.AddSettingNotUnique("NoBuy", row.BaseNickName, new object[] { row.GoodNickName });
            }

            cfgFile.DeleteSection("SystemSensor");
            foreach (UIDataSet.SysSensorListRow row in uIDataSet.SysSensorList.Rows)
            {
                if (row.EquipNickName.Length > 0)
                    cfgFile.AddSettingNotUnique("SystemSensor", row.SystemNickName, new object[] { row.EquipNickName, row.NetworkID });
            }

            cfgFile.AddSetting("Message", "CustomHelp", new object[] { checkBoxCustomHelp.Checked });
            cfgFile.AddSetting("Message", "CmdEcho", new object[] { checkBoxCmdEcho.Checked });
            cfgFile.AddSetting("Message", "CmdHide", new object[] { checkBoxCmdHide.Checked });
            cfgFile.AddSetting("Message", "CmdEchoStyle", new object[] { textBoxCmdEchoStyle.Text });

            cfgFile.AddSetting("Message", "SpecialBannerDelay", new object[] { textBoxSpecialBannerDelay.Text });
            cfgFile.AddSetting("Message", "StandardBannerDelay", new object[] { textBoxStandardBannerDelay.Text });

            cfgFile.DeleteSection("Help");
            foreach (string line in richTextBoxHelpLines.Lines)
            {
                if (line.Length > 0)
                    cfgFile.AddSettingNotUnique("Help", line, new object[] { });
            }

            cfgFile.DeleteSection("SwearWords");
            foreach (string line in richTextBoxSwearWords.Lines)
            {
                if (line.Length > 0)
                    cfgFile.AddSettingNotUnique("SwearWords", "Text", new object[] { line });
            }

            cfgFile.DeleteSection("GreetingBanner");
            foreach (string line in richTextBoxGreetingBannerLines.Lines)
            {
                if (line.Length > 0)
                    cfgFile.AddSettingNotUnique("GreetingBanner", "Text", new object[] { line });
            }

            cfgFile.DeleteSection("SpecialBanner");
            foreach (string line in richTextBoxSpecialBannerLines.Lines)
            {
                if (line.Length>0)
                    cfgFile.AddSettingNotUnique("SpecialBanner", "Text", new object[] { line });
            }

            while (cfgFile.DeleteSection("StandardBanner")) ;
            foreach (UIDataSet.StandardBannerListRow row in uIDataSet.StandardBannerList.Rows)
            {
                var sect = new FLDataFile.Section()
                {
                    sectionName = "StandardBanner"
                };
                foreach (string line in row.BannerText.Split(new char[] { '\r', '\n' }))
                {
                    if (line.Length>0)
                        sect.settings.Add(new FLDataFile.Setting("StandardBanner", "Text", "", new object[] {line}));
                }
                cfgFile.sections.Add(sect);
            }

            cfgFile.DeleteSection("Sounds");
            foreach (string line in richTextBoxSounds.Lines)
            {
                if (line.Length > 0)
                    cfgFile.AddSettingNotUnique("Sounds", "sound", new object[] { line });
            }

            cfgFile.DeleteSection("Wardrobe");
            foreach (string line in richTextBoxHeads.Lines)
            {
                if (line.Length > 0)
                    cfgFile.AddSettingNotUnique("Wardrobe", "head", new object[] { line });
            }
            foreach (string line in richTextBoxBodies.Lines)
            {
                if (line.Length > 0)
                    cfgFile.AddSettingNotUnique("Wardrobe", "body", new object[] { line });
            }

            cfgFile.DeleteSection("Restart");
            cfgFile.AddSetting("Restart", "MaxRank", new object[] { textBoxRestartMaxRank.Text });
            cfgFile.AddSetting("Restart", "MaxCash", new object[] { textBoxRestartMaxCash.Text });
            foreach (string line in richTextBoxRestarts.Lines)
            {
                if (line.Length > 0)
                    cfgFile.AddSettingNotUnique("Restart", "restart", new object[] { line });
            }

            try
            {
                cfgFile.SaveSettings(cfgFilePath, false);
                MessageBox.Show("File saved to " + cfgFilePath, "File saved");
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error " + ex.Message, "Unable to save ini file");
            }
        }

        private void ButtonAddItem_Click(object sender, EventArgs e)
        {
            new AddShipPimperItemWindow(gameData, uIDataSet.ShipPimperItemList).ShowDialog();
        }

        private void ButtonDelItem_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridViewShipPimperEquipList.SelectedRows)
            {
                UIDataSet.ShipPimperItemListRow item = (UIDataSet.ShipPimperItemListRow)((DataRowView)row.DataBoundItem).Row;
                uIDataSet.ShipPimperItemList.Rows.Remove(item);
            }
        }

        private void ButtonAddBase_Click(object sender, EventArgs e)
        {
            new AddShipPimperBaseWindow(gameData, uIDataSet.ShipPimperBaseList).ShowDialog();
        }

        private void ButtonDelBase_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridViewShipPimperBases.SelectedRows)
            {
                UIDataSet.ShipPimperBaseListRow item = (UIDataSet.ShipPimperBaseListRow)((DataRowView)row.DataBoundItem).Row;
                uIDataSet.ShipPimperBaseList.Rows.Remove(item);
            }
        }

        private void ButtonAddRepItem_Click(object sender, EventArgs e)
        {
            new AddRepFixerItemWindow(gameData, uIDataSet.RepFixerItemList).ShowDialog();
        }

        private void ButtonEditRep_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridViewRepFixerItems.SelectedRows)
            {
                UIDataSet.RepFixerItemListRow item = (UIDataSet.RepFixerItemListRow)((DataRowView)row.DataBoundItem).Row;
                new EditReps(gameData, item.ItemNickName, cfgFile).ShowDialog();
            }
        }

        private void Button3_Click(object sender, EventArgs e)
        {
            new SetBlockedSystemWindow(gameData, textBoxBlockedSystem).ShowDialog();
        }

        private void ButtonRepDelItem_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridViewRepFixerItems.SelectedRows)
            {
                UIDataSet.RepFixerItemListRow item = (UIDataSet.RepFixerItemListRow)((DataRowView)row.DataBoundItem).Row;
                uIDataSet.RepFixerItemList.Rows.Remove(item);
            }
        }

        private void ButtonPurchaseRestrictionAddItem_Click(object sender, EventArgs e)
        {
            new AddPurchaseRestrictionItemWindow(gameData, uIDataSet.ShipPurchaseRestrictionItems).ShowDialog(); 
        }

        private void ButtonPurchaseRestrictionEditItem_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridViewPurchaseRestrictions.SelectedRows)
            {
                UIDataSet.ShipPurchaseRestrictionItemsRow item = (UIDataSet.ShipPurchaseRestrictionItemsRow)((DataRowView)row.DataBoundItem).Row;
                new EditPurchaseRestrictionControlItemWindow(gameData, item).ShowDialog();
            }
        }

        private void ButtonPurchaseRestrictionDelItem_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridViewPurchaseRestrictions.SelectedRows)
            {
                UIDataSet.ShipPurchaseRestrictionItemsRow item = (UIDataSet.ShipPurchaseRestrictionItemsRow)((DataRowView)row.DataBoundItem).Row;
                uIDataSet.ShipPurchaseRestrictionItems.Rows.Remove(item);
            }
        }

        private void ButtonBannerAddItem_Click(object sender, EventArgs e)
        {
            uIDataSet.StandardBannerList.Rows.Add(new object[] { "" });
        }

        private void ButtonBannerEditItem_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridViewStandardBanners.SelectedRows)
            {
                UIDataSet.StandardBannerListRow item = (UIDataSet.StandardBannerListRow)((DataRowView)row.DataBoundItem).Row;
                new EditBannerWindow(item).ShowDialog();
            }
        }

        private void ButtonBannerDelItem_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridViewStandardBanners.SelectedRows)
            {
                UIDataSet.StandardBannerListRow item = (UIDataSet.StandardBannerListRow)((DataRowView)row.DataBoundItem).Row;
                uIDataSet.StandardBannerList.Rows.Remove(item);
            }
        }

        private void DataGridViewStandardBanners_CellDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            UIDataSet.StandardBannerListRow item = (UIDataSet.StandardBannerListRow)(((DataRowView)(dataGridViewStandardBanners[e.ColumnIndex, e.RowIndex].OwningRow).DataBoundItem)).Row;
            new EditBannerWindow(item).ShowDialog();
        }

        private void DataGridViewPurchaseRestrictions_CellDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            UIDataSet.ShipPurchaseRestrictionItemsRow item = (UIDataSet.ShipPurchaseRestrictionItemsRow)(((DataRowView)(dataGridViewPurchaseRestrictions[e.ColumnIndex, e.RowIndex].OwningRow).DataBoundItem)).Row;
            new EditPurchaseRestrictionControlItemWindow(gameData, item).ShowDialog();
        }

        private void DataGridViewRepFixerItems_CellDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            UIDataSet.RepFixerItemListRow item = (UIDataSet.RepFixerItemListRow)(((DataRowView)(dataGridViewRepFixerItems[e.ColumnIndex, e.RowIndex].OwningRow).DataBoundItem)).Row;
            new EditReps(gameData, item.ItemNickName, cfgFile).ShowDialog();
        }

        private void ButtonAddNoBuyItem_Click(object sender, EventArgs e)
        {
            new AddNoBuyItemWindow(gameData, uIDataSet.BaseGoodNoBuyList).ShowDialog(); 
        }

        private void ButtonEditNoBuyItem_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridViewNoBuyList.SelectedRows)
            {
                UIDataSet.BaseGoodNoBuyListRow item = (UIDataSet.BaseGoodNoBuyListRow)((DataRowView)row.DataBoundItem).Row;
                new EditNoBuyItemWindow(gameData, item).ShowDialog();
            }
        }

        private void ButtonDelNoBuyItem_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridViewNoBuyList.SelectedRows)
            {
                UIDataSet.BaseGoodNoBuyListRow item = (UIDataSet.BaseGoodNoBuyListRow)((DataRowView)row.DataBoundItem).Row;
                uIDataSet.BaseGoodNoBuyList.Rows.Remove(item);
            }
        }

        private void DataGridView1_CellDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            UIDataSet.BaseGoodNoBuyListRow item = (UIDataSet.BaseGoodNoBuyListRow)(((DataRowView)(dataGridViewNoBuyList[e.ColumnIndex, e.RowIndex].OwningRow).DataBoundItem)).Row;
            new EditNoBuyItemWindow(gameData, item).ShowDialog();
        }

        /// <summary>
        /// Read a CSV file and import ship purchase lines
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ButtonImportShipPurchaseRestrictions_Click(object sender, EventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog()
            {
                Filter = "CSV (*.csv)|*.csv|All Files|*.*",
                Title = "Open CSV File"
            };
            
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    // We expect the first row to contain the ship nicknames and the first
                    // column to contain the id nicknames. The first field on the first row
                    // must be empty.
                    using (StreamReader sr = File.OpenText(dialog.FileName))
                    {
                        CsvStream cs = new CsvStream(sr);

                        Dictionary<int, List<GameDataSet.HashListRow>> shipItemsByCol = new Dictionary<int, List<GameDataSet.HashListRow>>();

                        List<string> shipTypeRow = cs.GetNextRow();
                        if (shipTypeRow == null)
                            throw new Exception("No ship type row found");

                        // Validate the nicknames in each field.
                        for (int col = 1; col < shipTypeRow.Count; col++)
                        {
                            List<GameDataSet.HashListRow> shipItems = new List<GameDataSet.HashListRow>();
                            foreach (string nickname in shipTypeRow[col].Split(','))
                            {
                                if (nickname.Length > 0)
                                {
                                    GameDataSet.HashListRow item = gameData.GetItemByNickName(nickname.Trim());
                                    if (item == null)
                                        throw new Exception(String.Format("Item '{0}' not found", nickname));
                                    if (item.ItemType != FLGameData.GAMEDATA_SHIPS)
                                        throw new Exception(String.Format("Item '{0}' is not ship", nickname));
                                    shipItems.Add(item);
                                }
                            }
                            shipItemsByCol.Add(col, shipItems);
                        }

                        List<string> row = cs.GetNextRow();
                        while (row != null)
                        {
                            // Extract and verify item nicknames for this row.
                            List<GameDataSet.HashListRow> controlItems = new List<GameDataSet.HashListRow>();
                            foreach (string nickname in row[0].Split(','))
                            {
                                if (nickname.Length > 0)
                                {
                                    GameDataSet.HashListRow item = gameData.GetItemByNickName(nickname.Trim());
                                    if (item == null)
                                        throw new Exception(String.Format("Item '{0}' not found", nickname));
                                    controlItems.Add(item);
                                }
                            }

                            // For all cell with a 1 in them add lines to data set.
                            for (int col = 1; col < row.Count; col++)
                            {
                                if (row[col] == "1")
                                {
                                    foreach (GameDataSet.HashListRow shipItem in shipItemsByCol[col])
                                    {
                                        foreach (GameDataSet.HashListRow controlItem in controlItems)
                                        {
                                            uIDataSet.ShipPurchaseRestrictionItems.AddShipPurchaseRestrictionItemsRow(shipItem.IDSName, shipItem.ItemNickName, controlItem.IDSName, controlItem.ItemNickName);
                                        }
                                    }
                                }
                            }

                            row = cs.GetNextRow();
                        }
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(this, "Error '" + ex.Message + "'", "Error");
                }
            }
        }

        private void ButtonEquipPurchaseRestrictionAdd_Click(object sender, EventArgs e)
        {
            new AddEquipPurchaseRestrictionItemWindow(gameData, uIDataSet.EquipPurchaseRestrictionItems).ShowDialog();
        }

        private void ButtonEquipPurchaseRestrictionEdit_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridViewEquipPurchaseRestriction.SelectedRows)
            {
                UIDataSet.EquipPurchaseRestrictionItemsRow item = (UIDataSet.EquipPurchaseRestrictionItemsRow)((DataRowView)row.DataBoundItem).Row;
                new EditEquipPurchaseRestrictionControlItemWindow(gameData, item).ShowDialog();
            }
        }

        private void ButtonEquipPurchaseRestrictionDel_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridViewEquipPurchaseRestriction.SelectedRows)
            {
                UIDataSet.EquipPurchaseRestrictionItemsRow item = (UIDataSet.EquipPurchaseRestrictionItemsRow)((DataRowView)row.DataBoundItem).Row;
                uIDataSet.EquipPurchaseRestrictionItems.Rows.Remove(item);
            }
        }

        private void DataGridViewEquipPurchaseRestriction_CellDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            UIDataSet.EquipPurchaseRestrictionItemsRow item = (UIDataSet.EquipPurchaseRestrictionItemsRow)(((DataRowView)(dataGridViewEquipPurchaseRestriction[e.ColumnIndex, e.RowIndex].OwningRow).DataBoundItem)).Row;
            new EditEquipPurchaseRestrictionControlItemWindow(gameData, item).ShowDialog();
        }

        private void ButtonSysSensorAddItem_Click(object sender, EventArgs e)
        {
             new AddSysSensorItemWindow(gameData, uIDataSet.SysSensorList).ShowDialog(); 
        }

        private void ButtonSysSensorEditItem_Click(object sender, EventArgs e)
        {
            foreach (DataGridViewRow row in dataGridViewSysSensor.SelectedRows)
            {
                UIDataSet.SysSensorListRow item = (UIDataSet.SysSensorListRow)((DataRowView)row.DataBoundItem).Row;
                new EditSysSensorItemWindow(gameData, item).ShowDialog();
            }
        }

        private void ButtonSysSensorDelItem_Click(object sender, EventArgs e)
        {
             foreach (DataGridViewRow row in dataGridViewSysSensor.SelectedRows)
            {
                UIDataSet.SysSensorListRow item = (UIDataSet.SysSensorListRow)((DataRowView)row.DataBoundItem).Row;
                uIDataSet.SysSensorList.Rows.Remove(item);
            }
        }

        private void DataGridViewSysSensor_CellDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            UIDataSet.SysSensorListRow item = (UIDataSet.SysSensorListRow)(((DataRowView)(dataGridViewSysSensor[e.ColumnIndex, e.RowIndex].OwningRow).DataBoundItem)).Row;
            new EditSysSensorItemWindow(gameData, item).ShowDialog();
        }

        private void RichTextBoxHeads_TextChanged(object sender, EventArgs e)
        {

        }

        private void RichTextBoxSounds_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
