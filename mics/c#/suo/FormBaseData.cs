using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Text.RegularExpressions;
using System.Data.OleDb;

namespace SUOTrening
{
    public enum TypesForm
    {
        eNzr  = 0,
        eDist = 1,
        eSN   = 2
    };
    
    public partial class FormBaseData : Form
    {
        public FormDalnomer frmDalnomer;
        public FormNZR frmNZR;
        public FormSN  frmSN;

        TypesForm type_next_form;
        public FormBaseData(TypesForm etype, TBaseData base_data)
        {
            InitializeComponent();
            // Attach DataGridView events to the corresponding event handlers.
            this.dataGridViewOrientir.CellValidating += new
                DataGridViewCellValidatingEventHandler(dataGridViewOrientir_CellValidating);
            this.dataGridViewOrientir.CellEndEdit += new
                DataGridViewCellEventHandler(dataGridViewOrientir_CellEndEdit);
            this.dataGridViewPopr.CellValidating += new
                DataGridViewCellValidatingEventHandler(dataGridViewOrientir_CellValidating);
            this.dataGridViewPopr.CellEndEdit += new
                DataGridViewCellEventHandler(dataGridViewOrientir_CellEndEdit);
            test_init(base_data);
            type_next_form = etype;
            toolStripStatusLabel1.Text = "";
        }


        private void test_init(TBaseData base_data)
        {
            OleDbConnection connection =
                           new OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=OrientirsDB.mdb;");
            
            try
            {
                connection.Open();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
            try
            {
                OleDbCommand command = connection.CreateCommand();
                command.CommandText = "select * from Orientirs;";
                OleDbDataReader reader = command.ExecuteReader();  
                while (reader.Read())
                    dataGridViewOrientir.Rows.Add(reader["id"].ToString(), reader["name"].ToString(), reader["angle"].ToString(), reader["distance"].ToString());
                connection.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
            
            maskedTextBox_NumOrient.Text = base_data.NumOrientir.ToString();

            dataGridViewPopr.Rows.Add(base_data.listNodeCorrectue[0].SupportDist.ToString(),  base_data.listNodeCorrectue[0].CorrDist.ToString(), base_data.listNodeCorrectue[0].CorrAngle.ToString());
            dataGridViewPopr.Rows.Add(base_data.listNodeCorrectue[1].SupportDist.ToString(), base_data.listNodeCorrectue[1].CorrDist.ToString(), base_data.listNodeCorrectue[1].CorrAngle.ToString());
            dataGridViewPopr.Rows.Add(base_data.listNodeCorrectue[2].SupportDist.ToString(), base_data.listNodeCorrectue[2].CorrDist.ToString(), base_data.listNodeCorrectue[2].CorrAngle.ToString());

            maskedTextBox_eps_targ.Text = base_data.EpsTarget.ToString();
            maskedTextBox_depth_target.Text = base_data.DepthTarget.ToString();
            maskedTextBox_NumTarget.Text = base_data.NumTarget.ToString();
            textBox_FrontTarget.Text = base_data.FrontTarget.ToString(false);

            radioButton_z_1.Checked = true;
            textBox_alpha_on.Text = base_data.AlphaOn.ToString(false);
            maskedTextBoxYknp.Text = base_data.Knp.y.ToString();
            maskedTextBoxXknp.Text = base_data.Knp.x.ToString();
            maskedTextBoxHknp.Text = base_data.Knp.h.ToString();
            textBox_NameKnp.Text = base_data.Knp.name;

            maskedTextBoxXnp.Text = base_data.Np.x.ToString();
            maskedTextBoxYnp.Text = base_data.Np.y.ToString();
            maskedTextBoxHnp.Text = base_data.Np.h.ToString();
            textBox_NameNp.Text = base_data.Np.name;

            maskedTextBoxYop.Text = base_data.Op.y.ToString();
            maskedTextBoxXop.Text = base_data.Op.x.ToString();

            maskedTextBoxHop.Text = base_data.Op.h.ToString();
            textBox_NameOp.Text = base_data.Op.name;

            if (base_data.NumOrientir > 0)
            {
                maskedTextBox_DistFromOrient.Text = base_data.listNodeOrientir[0].DeltaDist.ToString();
                textBox_DovFromOrient.Text = base_data.listNodeOrientir[0].DeltaAngle.ToString();
            }
            radioButton_fromKnp.Checked = true;
            maskedTextBox_dist_target_knp.Text = base_data.DistCommander.ToString();
            textBox_DirAngleCenter_trgt_knp.Text = base_data.DirectionAngleFromKnp.ToString(false) ;

            comboBoxShot.Text = "Р";
        }

        
        private void radioButton_fromOrient_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton_fromOrient.Checked)
            {
                panelAngleFromOrientir.Enabled = true;
                panelAngleFromKnp.Enabled = false;
            }
            else
            {
                panelAngleFromOrientir.Enabled = false;
                panelAngleFromKnp.Enabled = true;
            }
        }



        private void verifyOrientir(ref DataGridViewCellValidatingEventArgs e, string sregexp, string errtxt_void, string errtxt_not_pattern)
        {
            Regex pat = new Regex(sregexp);
            Match match = pat.Match(e.FormattedValue.ToString());

            if (string.IsNullOrEmpty(e.FormattedValue.ToString()))
            {
                dataGridViewOrientir.Rows[e.RowIndex].ErrorText = errtxt_void;
                e.Cancel = true;
            }
            else if (!match.Success)
            {
                dataGridViewOrientir.Rows[e.RowIndex].ErrorText = errtxt_not_pattern;
                e.Cancel = true;
            }
        }

        private void dataGridViewOrientir_CellValidating(object sender, DataGridViewCellValidatingEventArgs e)
        {
            string headerText =
                dataGridViewOrientir.Columns[e.ColumnIndex].HeaderText;

            if (headerText.Equals("№"))
                verifyOrientir(ref e, "^[\\d]+$", "Номер не может быть пустым", "Номер состоит только из цифр");
            else if (headerText.Equals("Дальность от КНП до ориентира"))
                verifyOrientir(ref e, "^[\\d]+$", "Дальность должна быть задана", "Дальность состоит только из цифр");
            else if (headerText.Equals("Дирекционный угол ориентира"))
                verifyOrientir(ref e, "^[\\d]{1,2}-[\\d]{2}$", "Дирекционный угол должен быть задан", "Дирекционный угол должен быть задан в формате DD-DD");
            else if (headerText.Equals("Опорная дальность"))
                verifyOrientir(ref e, "^[\\d]+$", "Дальность должна быть задана", "Дальность состоит только из цифр");
            else if (headerText.Equals("Поправка в дальность"))
                verifyOrientir(ref e, "^[\\d]+$", "Поправка в дальность должна быть задана", "Поправка в дальность состоит только из цифр");
            else if (headerText.Equals("Поправка в направлении"))
                verifyOrientir(ref e, "^[+-]{1}[\\d]{1,2}-[\\d]{2}$", "Поправка в направлении должна быть задана", "Поправка в направлении должна быть задана в формате +/-DD-DD");

        }

        void dataGridViewOrientir_CellEndEdit(object sender, DataGridViewCellEventArgs e)
        {
            dataGridViewOrientir.Rows[e.RowIndex].ErrorText = String.Empty;
        }



        private TBaseData.eZar getTypeZar()
        {
            if (radioButton_z_p.Checked) return TBaseData.eZar.eFull;
            else if (radioButton_z_u.Checked) return TBaseData.eZar.eSmall;
            else if (radioButton_z_1.Checked) return TBaseData.eZar.eOne;
            else if (radioButton_z_2.Checked) return TBaseData.eZar.eTwo;
            else if (radioButton_z_3.Checked) return TBaseData.eZar.eThree;
            else return TBaseData.eZar.eNot;
        }


        private void button_SetBaseData_Click(object sender, EventArgs e)
        {
            TBaseData fire_data = new TBaseData();
            //try
            {
                fire_data.listNodeOrientir.Clear();
                /*foreach (DataGridViewRow row in dataGridViewOrientir.Rows)
                {
                    if (row.Cells["ColumnNum"].Value == null) continue;
                    TBaseData.NodeOrientir node = new TBaseData.NodeOrientir();
                    node.num = Convert.ToInt32(row.Cells["ColumnNum"].Value);
                    node.name = row.Cells["ColumnName"].Value.ToString();
                    node.DeltaAngle = new DelUgl(row.Cells["ColumnDirAngle"].Value.ToString());
                    node.DeltaDist = Convert.ToInt64(row.Cells["ColumnDist"].Value);
                    fire_data.listNodeOrientir.Add(node);
                }    */
                                
                if (radioButton_fromKnp.Checked) //ДК и доворот заданы
                {
                    fire_data.NumOrientir = -1;
                    string str = maskedTextBox_dist_target_knp.Text.ToString();
                    
                    fire_data.DistCommander = Convert.ToInt32(str);
                    fire_data.DirectionAngleFromKnp.fromString(textBox_DirAngleCenter_trgt_knp.Text);
                }
                else // Заданы дальность и доворот от ориентира
                {
                    //1) отложть от ориентира данный доворот
                    //2) прибавить к дальности дальность от ориентира
                    fire_data.NumOrientir = Convert.ToInt32(maskedTextBox_NumOrient.Text);

                    foreach (TBaseData.NodeOrientir node in fire_data.listNodeOrientir)
                    {
                        if (node.num == fire_data.NumOrientir)
                        {
                            fire_data.DistCommander = node.DeltaDist + Convert.ToInt64(maskedTextBox_DistFromOrient.Text);
                            
                            DelUgl dgl = new DelUgl();
                            dgl.fromString(textBox_DovFromOrient.Text);
                            
                            fire_data.DirectionAngleFromKnp.fromInt32(node.DeltaAngle.ToInt32() + dgl.ToInt32());
                        }
                    }
                }
                
                fire_data.EpsTarget.fromString(maskedTextBox_eps_targ.Text);
                fire_data.DepthTarget = Convert.ToInt64(maskedTextBox_depth_target.Text);
                fire_data.NumTarget = Convert.ToInt64(maskedTextBox_NumTarget.Text);
                fire_data.FrontTarget.fromString(textBox_FrontTarget.Text);

                fire_data.listNodeCorrectue.Clear();
                foreach (DataGridViewRow row in dataGridViewPopr.Rows)
                {
                    if (row.Cells["ColumnDistPopr"].Value == null) continue;
                    TBaseData.NodeCorrectue node = new TBaseData.NodeCorrectue();
                    node.SupportDist = Convert.ToInt64(row.Cells["ColumnDistPopr"].Value.ToString());
                    node.CorrDist = Convert.ToInt64(row.Cells["ColumnValPopr"].Value.ToString());
                    node.CorrAngle.fromString(row.Cells["ColumnPoprNapr"].Value.ToString());
                    fire_data.listNodeCorrectue.Add(node);
                }
                
                fire_data.TypeZar = getTypeZar();
                
                fire_data.AlphaOn.fromString(textBox_alpha_on.Text);
                fire_data.Knp.x = Convert.ToInt64(maskedTextBoxXknp.Text);
                fire_data.Knp.y = Convert.ToInt64(maskedTextBoxYknp.Text);
                fire_data.Knp.h = Convert.ToInt64(maskedTextBoxHknp.Text.Substring(0,5));
                fire_data.Knp.name = textBox_NameKnp.Text;

                fire_data.Np.x = Convert.ToInt64(maskedTextBoxXnp.Text);
                fire_data.Np.y = Convert.ToInt64(maskedTextBoxYnp.Text);
                fire_data.Np.h = Convert.ToInt64(maskedTextBoxHnp.Text.Substring(0, 5));
                fire_data.Np.name = textBox_NameNp.Text;

                fire_data.Op.x = Convert.ToInt64(maskedTextBoxXop.Text);
                fire_data.Op.y = Convert.ToInt64(maskedTextBoxYop.Text);
                fire_data.Op.h = Convert.ToInt64(maskedTextBoxHop.Text.Substring(0, 5));
                fire_data.Op.name = textBox_NameOp.Text;

                fire_data.TypeShotFromStr(comboBoxShot.Text);

                fire_data.make_solved_data((textBoxVeer.Text == "  -") ? "" : textBoxVeer.Text);

                switch (type_next_form)
                {
                    case TypesForm.eNzr:
                        {
                            frmNZR = new FormNZR(fire_data);
                            frmNZR.Show();
                            break;
                        }
                    case TypesForm.eDist:
                        {
                            frmDalnomer = new FormDalnomer(fire_data);
                            frmDalnomer.Show();
                            break;
                        }
                    case TypesForm.eSN:
                        {
                            frmSN = new FormSN(fire_data);
                            frmSN.Show();
                            break;
                        }
                }   
            }
            //catch (Exception exc)
            {
            //    MessageBox.Show(exc.Message);
            }
        }

        private void textBox_FrontTarget_Validating(object sender, CancelEventArgs e)
        {
            DelUgl.ValidateDelUglControl(textBox_FrontTarget, e, toolStripStatusLabel1,
               "^[\\d]{1,2}-[\\d]{2}$");
        }

        private void textBoxVeer_Validating(object sender, CancelEventArgs e)
        {
            DelUgl.ValidateDelUglControl(textBoxVeer, e, toolStripStatusLabel1,
               "^[\\d]{1,2}-[\\d]{2}$");
        }

        private void textBox_DovFromOrient_Validating(object sender, CancelEventArgs e)
        {
            DelUgl.ValidateDelUglControl(textBox_DovFromOrient, e, toolStripStatusLabel1,
                "^[+-]{0,1}[\\d]{1,2}-[\\d]{2}$");
        }

        private void textBox_DirAngleCenter_trgt_knp_Validating(object sender, CancelEventArgs e)
        {
            DelUgl.ValidateDelUglControl(textBox_DirAngleCenter_trgt_knp, e, toolStripStatusLabel1,
               "^[\\d]{1,2}-[\\d]{2}$");
        }

        private void textBox_alpha_on_Validating(object sender, CancelEventArgs e)
        {
            DelUgl.ValidateDelUglControl(textBox_alpha_on, e, toolStripStatusLabel1,
                "^[\\d]{1,2}-[\\d]{2}$");
        }
    }
}
