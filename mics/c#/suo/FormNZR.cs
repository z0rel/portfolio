using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;


namespace SUOTrening
{
    public partial class FormNZR : Form
    {
        
        TBaseData RefBaseData;
        Correctures correctures;
        bool flag_first;

        ShootData.Nabludenie typeNabl_old;

        public void setShoot(DelUgl DovorotFromTargetKnp, ShootData.Nabludenie typeNabl, ShootData.Nabludenie typeNabl_old)
        {
            if (typeNabl != ShootData.Nabludenie.NotDetected)
                checkBoxNotVisible.Checked = false;
            if (typeNabl != ShootData.Nabludenie.KillTarget)
                checkBoxKillTarget.Checked = false;
            if (typeNabl == ShootData.Nabludenie.AllPerelet)
                radioButtonPerelet.Checked = true;
            else if (typeNabl == ShootData.Nabludenie.AllNedolet)
                radioButtonNedolet.Checked = true;
            textBox_alpha_view.Text = DovorotFromTargetKnp.ToString(true);
            this.typeNabl_old = typeNabl_old;
        }

        public void setNotDetected(DelUgl DovorotFromTargetKnp)
        {
            checkBoxKillTarget.Checked = false;
            textBox_alpha_view.Text = DovorotFromTargetKnp.ToString(true);
            checkBoxNotVisible.Checked = true;
        }

        public void setShootDefence(DelUgl front, DelUgl mistake_center, ShootData.Nabludenie typeNabl,
            int otkl_dist_expl_trgt)
        {
            checkBoxNotVisible.Checked = false;
            //if (typeNabl != ShootData.Nabludenie.KillTarget)
            //    checkBoxKillTarget.Checked = false;
            textBox_FrontExplosions.Text = front.ToString(false);
            textBox_α_explosions_center.Text = mistake_center.ToString();

            labelOtklDistVal.Text = otkl_dist_expl_trgt.ToString();

            switch (typeNabl)
            {
                case ShootData.Nabludenie.AllPerelet: radioButton_all_perelet.Checked           = true; break;
                case ShootData.Nabludenie.AllNedolet: radioButton_all_nedolet.Checked           = true; break;
                case ShootData.Nabludenie.EqualsFringeHigh: radioButton_equality_high.Checked   = true; break;
                case ShootData.Nabludenie.EqualsFringeLow: radioButton_equality_low.Checked     = true; break;
                case ShootData.Nabludenie.HighOlklCenterZalp: radioButton_otkl_napr.Checked     = true; break;
                case ShootData.Nabludenie.MorePerelet: radioButton_dominance_perelet.Checked    = true; break;
                case ShootData.Nabludenie.MoreNedolet: radioButton_dominance_nedolet.Checked    = true; break;
                case ShootData.Nabludenie.KillTarget: radioButton_targetDefence.Checked         = true; break;
            }
        }


        public void setShootKillInPristrelka(DelUgl dov_from_center_target, double delta_x)
        {
            checkBoxKillTarget.Checked = true;
            checkBoxNotVisible.Checked = false;
            maskedTextBoxKillTarget.Text = ((int)Math.Round(delta_x)).ToString();
            textBox_alpha_view.Text = dov_from_center_target.ToString(true);
        }

        public FormNZR(TBaseData BaseData)
        {
            InitializeComponent();
            HideNames();
            RefBaseData = BaseData;
            SolvedData solved_data = RefBaseData.solved_data;

            label_xc.Text = Convert.ToString(solved_data.Xc);
            label_yc.Text = Convert.ToString(solved_data.Yc);

            label_dk.Text = Convert.ToString(BaseData.DistCommander);
            //label_dovk.Text = solved_data.dov_k.ToString();

            label_dct.Text = solved_data.Dct.ToString();
            label_dov_ct.Text = solved_data.dov_ct.ToString();

            label_dci.Text = Convert.ToString(solved_data.Dci);
            label_dov_ci.Text = solved_data.dov_ci.ToString();

            label_meteo_ΔД.Text = Convert.ToString(solved_data.dist_correcture);
            label_meteo_Δdov.Text = solved_data.angle_correcture.ToString();

            label_h_c.Text = Convert.ToString(solved_data.Hc);
            label_eps_c_op.Text = solved_data.eps_c_op.ToString();

            label_ur.Text = solved_data.У.ToString(false);

            label_pricel_ts.Text = Convert.ToString(solved_data.pricel_ts);
            label_delta_x_mil.Text = Convert.ToString(solved_data.delta_x_mil);

            label_k_u.Text = Convert.ToString((double)((int)(solved_data.Ku * 10000)) / 10000); // избавление от доп. знаков после запятой
            label_ps.Text = solved_data.PS.ToString(false);
            label_Shu100.Text = solved_data.Shu100.ToString(false);

            label_front_m.Text = solved_data.FrontTarget_m.ToString();

            correctures = new Correctures(RefBaseData);

            label_ΔД.Text = "";

            flag_first = true;

            label_veer.Text = RefBaseData.solved_data.ShotVeer.ToString(false);
            labelSkPricelValue.Text = RefBaseData.solved_data.SkPricel.ToString();
            labelUstUglomerValue.Text = RefBaseData.solved_data.UstUgl.ToString();
            buttonCmd_Click(null, null);

            labelDu_op.Text = solved_data.TargetDirAngleFromOp.ToString(false);
            labelOtklDistVal.Text = "";
        }

        public void HideNames()
        {
            this.label_ΔД.Text = "первый выстрел";

            this.label_dci.Text = "";
            this.label_dov_ci.Text = "";
            this.label_k_u.Text = "";
            this.label_Shu100.Text = "";
            this.label_eps_c_op.Text = "";
            this.label_h_c.Text = "";
            this.label_ur.Text = "";
            this.label_delta_x_mil.Text = "";
            this.toolStripStatusLabel1.Text = "";
            this.label_П.Text = "";
        }

        private void HandleFireDefence()
        {
            DelUgl FrontExplosions = new DelUgl(textBox_FrontExplosions.Text);
            DelUgl dov_explosions_center = new DelUgl(textBox_α_explosions_center.Text);

            if (!(radioButton_all_nedolet.Checked       || radioButton_all_perelet.Checked       ||
                  radioButton_dominance_perelet.Checked || radioButton_dominance_nedolet.Checked ||
                  radioButton_equality_high.Checked     || radioButton_equality_low.Checked      ||
                  radioButton_otkl_napr.Checked         || radioButton_targetDefence.Checked))
                throw new Exception("Нужно выбрать характер разрывов");

            ShootData.Nabludenie typeNabl = ShootData.Nabludenie.NotDetected;

            if      (radioButton_all_perelet.Checked)       typeNabl = ShootData.Nabludenie.AllPerelet;
            else if (radioButton_all_nedolet.Checked)       typeNabl = ShootData.Nabludenie.AllNedolet;
            else if (radioButton_equality_high.Checked)     typeNabl = ShootData.Nabludenie.EqualsFringeHigh;
            else if (radioButton_equality_low.Checked)      typeNabl = ShootData.Nabludenie.EqualsFringeLow;
            else if (radioButton_otkl_napr.Checked)         typeNabl = ShootData.Nabludenie.HighOlklCenterZalp;
            else if (radioButton_dominance_perelet.Checked) typeNabl = ShootData.Nabludenie.MorePerelet;
            else if (radioButton_dominance_nedolet.Checked) typeNabl = ShootData.Nabludenie.MoreNedolet;
            else if (radioButton_targetDefence.Checked)     typeNabl = ShootData.Nabludenie.KillTarget;

            int ΔД = correctures.getDefenceDeltaD(typeNabl, Convert.ToInt32(labelOtklDistVal.Text));

            correctures.next_shotDefence(dov_explosions_center, ΔД, FrontExplosions, typeNabl);

            string outstr = "ΔП = " + ((correctures.ΔП > 0) ? "+" : "")
                + correctures.ΔП.ToString() + "\tβ = " + correctures.Δβ.ToString();
            if (RefBaseData.solved_data.PS.ToInt32() < 500)
                outstr += "\t ΔIвр= " + correctures.ΔI_veer_razr.ToString();

            FormTrening.thisFrmTreining.initialize_callback(0, 0, (int)correctures.ΔП, true,
                ((correctures.ΔI_veer_razr.ToInt32() != 0) ? correctures.ΔI_veer_razr.ToString() : "")
                , "", correctures.Δβ.ToString(true));

            richTextBoxCmd.Text += outstr + "\n";
        }


        private bool IsGotoDefence = false;
        private void buttonCmd_Click(object sender, EventArgs e)
        {
            try
            {
                if (flag_first)
                {
                    correctures.first_shot();

                    richTextBoxCmd.Text += "Цель " 
                        + RefBaseData.NumTarget.ToString() + " пехота укрытая, " +
                    "заряд " + RefBaseData.TypeZarToString() + "\n" +
                    "Прицел  " + correctures.П.ToString() + "; Уровень " + correctures.У.ToString(false) +
                    "; Доворот " + correctures.dov.ToString() + "\n\n";
                    flag_first = false;
                    label_vilka.Text = correctures.vilka.ToString();

                    FormTrening.thisFrmTreining.initialize_callback(RefBaseData.solved_data.SkPricel, RefBaseData.solved_data.UstUgl,
                        (int)correctures.П, false, RefBaseData.solved_data.ShotVeer.ToString(false), correctures.У.ToString(false),
                        correctures.dov.ToString(true));

                    groupBox_nabl.Enabled = true;

                }
                else if (!groupBoxFireDefence.Enabled)
                {
                    if (checkBoxNotVisible.Checked) // знак разрыва не удалось определить (неизвестно - перелет или недолет)
                    {
                        correctures.output_to_line_visible(
                            new DelUgl(textBox_alpha_view.Text));
                        richTextBoxCmd.Text += "Δβ = " + correctures.Δβ.ToString() + "\n";
                    }
                    else if (!checkBoxKillTarget.Checked)
                    {
                        correctures.next_shotNZR
                          (
                              new DelUgl(textBox_alpha_view.Text),
                              ((radioButtonNedolet.Checked) ? ShootData.Nabludenie.AllNedolet :
                                    ShootData.Nabludenie.AllPerelet),
                              typeNabl_old,
                              IsGotoDefence
                          );

                        label_ΔД.Text = correctures.ΔД.ToString();
                        label_П.Text = correctures.П.ToString();

                        label_vilka.Text = correctures.vilka.ToString();

                        richTextBoxCmd.Text += "ΔП = " + ((correctures.ΔП > 0) ? "+" : "")
                            + correctures.ΔП.ToString() + "\tβ = " + correctures.Δβ.ToString() + "\n";

                        FormTrening.thisFrmTreining.initialize_callback(0, 0, (int)correctures.ΔП, true,
                            "", "", correctures.Δβ.ToString(true));

                        if (
                            ((correctures.vilka == 50) && (RefBaseData.DepthTarget < 100))
                            || ((correctures.vilka == 100) && (RefBaseData.DepthTarget > 100))
                           )
                        {
                            IsGotoDefence = true;
                            richTextBoxCmd.Text += "Пероход к стрельбе на поражение\n";
                        }

                        if (IsGotoDefence)
                        {
                            groupBox_nabl.Enabled = false;
                            groupBoxFireDefence.Enabled = true;
                        }
                    }
                    else if (checkBoxKillTarget.Checked)
                    {
                        DelUgl dov_from_center_target = new DelUgl(textBox_alpha_view.Text);
              
                        int dist_first = Convert.ToInt32(maskedTextBoxKillTarget.Text) + (int)RefBaseData.DistCommander;
                        
                        correctures.next_shotDalnomer(dov_from_center_target, dist_first);

                        label_ΔД.Text = correctures.ΔД.ToString();
                        label_П.Text = correctures.П.ToString();

                        richTextBoxCmd.Text += "ΔП = " + ((correctures.ΔП > 0) ? "+" : "")
                            + correctures.ΔП.ToString() + "\tβ = " + correctures.Δβ.ToString() + "\n";

                        FormTrening.thisFrmTreining.initialize_callback(0, 0, (int)correctures.ΔП, true,
                            "", "", correctures.Δβ.ToString(true));

                        richTextBoxCmd.Text += "Пероход к стрельбе на поражение\n";

                        IsGotoDefence = true;
                        groupBox_nabl.Enabled = false;
                        groupBoxFireDefence.Enabled = true;
                    }
                }
                else if (groupBoxFireDefence.Enabled)
                    HandleFireDefence();
            }
            catch (Exception exc)
            {
                MessageBox.Show(exc.Message);
            }
        }

        private void checkBoxNotVisible_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBoxNotVisible.Checked)
            {
                radioButtonPerelet.Checked = false;
                radioButtonPerelet.Enabled = false;
                radioButtonNedolet.Checked = false;
                radioButtonNedolet.Enabled = false;
            }
            else
            {
                radioButtonPerelet.Enabled = true;
                radioButtonNedolet.Enabled = true;
            }
        }

        private void checkBoxKillTarget_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBoxKillTarget.Checked)
                 groupBoxKillTarget.Enabled = true;
            else groupBoxKillTarget.Enabled = false;
        }

        private void textBox_α_explosions_center_Validating(object sender, CancelEventArgs e)
        {
            DelUgl.ValidateDelUglControl(textBox_α_explosions_center, e, toolStripStatusLabel1,
                "^[+-]{0,1}[\\d]{1,2}-[\\d]{2}$");
        }

        private void textBox_FrontExplosions_Validating(object sender, CancelEventArgs e)
        {
            DelUgl.ValidateDelUglControl(textBox_FrontExplosions, e, toolStripStatusLabel1,
                "^[\\d]{1,2}-[\\d]{2}$");
        }

        private void textBox_alpha_view_Validating(object sender, CancelEventArgs e)
        {
            DelUgl.ValidateDelUglControl(textBox_alpha_view, e, toolStripStatusLabel1,
                "^[+-]{1}[\\d]{1,2}-[\\d]{2}$");
        }
    }
}
