using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using System.IO;
using Flash.External;

using System.Data.OleDb;

namespace SUOTrening
{
    public partial class FormShowShooting : Form
    {
        private ExternalInterfaceProxy proxy;
        
        private ShootData shootData;
        private Random w_rnd;
        
        private List<ShootData.ShootCommand> AllShoots;
        private bool is_np;
        private List<int> NextVD;
        private int currentVdCount;
        //public static FormFinalDraw frmFinalDraw = new FormFinalDraw();

        List<FormFinalDraw.NodeFinalShot> PreNodeFinalList;

        private class OneZalpType
        {
            public OneZalpType(List<ShootData.ShootCommand> Shoots, int point)
            {
                ZalpShoots = new List<ShootData.ShootCommand>();
                for (int i = 0; i < 6; i++)
                    ZalpShoots.Add(Shoots[i + point * 6]);
            }
            public List<ShootData.ShootCommand> ZalpShoots;
        }
        List<OneZalpType> Zalps;

        public FormShowShooting(ref ShootData shootData, bool is_np = false, List<int> NextVD = null)
        {
            this.is_np     = is_np;
            this.shootData = shootData;
            w_rnd     = new Random(DateTime.Now.Millisecond);
            AllShoots = new List<ShootData.ShootCommand>();
            Zalps     = new List<OneZalpType>();
            if (NextVD != null)
                this.NextVD = NextVD;
            else
                this.NextVD = new List<int>();
            PreNodeFinalList = new List<FormFinalDraw.NodeFinalShot>();
            InitializeComponent();
        }

        private object proxy_ExternalInterfaceCall(object sender, ExternalInterfaceCallEventArgs e)
        {
            switch (e.FunctionCall.FunctionName)
            {
                case "Close":
                    this.Close();
                    break;
                case "BlockButton":
                    this.buttonCancel.Enabled = false;
                    this.buttonCancel.Visible = false;
                    break;
            }
            return null;
        }

        int GetDistFromPricerl_TS(int pricel, out int delta_x_mil)
        {
            string connectionStr = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=TSD30.mdb";
            OleDbConnection connection = new OleDbConnection(connectionStr);

            string queryString = "SELECT П_мех_тыс, delta_X_тыс, Дальность FROM OF540" +
                                 " WHERE Заряд = \"" + FormTrening.baseData.TypeZarToString() + "\" AND Тип_стрельбы = \"Р\"" +
                                    "AND ABS(П_мех_тыс - " + pricel.ToString() + ") = (" +
                                         "SELECT min(ABS(П_мех_тыс - " + pricel.ToString() + ")) FROM of540" +
                                         " WHERE Заряд =\"" + FormTrening.baseData.TypeZarToString() +
                                         "\" AND Тип_стрельбы = \"Р\")";

            OleDbCommand command = new OleDbCommand(queryString, connection);
            connection.Open();
            OleDbDataReader reader = command.ExecuteReader();

            delta_x_mil = 0;
            int dist = 0;
            if (reader.Read())
            {
                int pricel_ts = Convert.ToInt32(reader["П_мех_тыс"]);
                delta_x_mil = Convert.ToInt32(reader["delta_X_тыс"]);
                dist = (pricel - pricel_ts) * delta_x_mil + Convert.ToInt32(reader["Дальность"]);
            }
            connection.Close();
            return dist;
        }

        private void AddShotsItem(int time, int k, int high_cnt)
        {
            for (int listCnt = 0; listCnt < high_cnt; listCnt++)
            {
                if (k > 2) k = 2; 
                ShootData.ShootCommand cmd =
                    new ShootData.ShootCommand(shootData.Shoots[listCnt + k * high_cnt].Pricel,
                        shootData.Shoots[listCnt + k * high_cnt].Uroven,
                        shootData.Shoots[listCnt + k * high_cnt].Dovorot,
                        shootData.Shoots[listCnt + k * high_cnt].Veer, time);
                AllShoots.Add(cmd);
            }
        }

        private void AddOneZalpDefenceItem(OneZalpType obj, int time)
        {
            for (int i = 0; i < obj.ZalpShoots.Count; i++)
                    AllShoots.Add(new ShootData.ShootCommand(obj.ZalpShoots[i].Pricel,
                        obj.ZalpShoots[i].Uroven,
                        obj.ZalpShoots[i].Dovorot,
                        obj.ZalpShoots[i].Veer, time));
        }
        
        private void MakeShotsDefenceItem()
        {
            Zalps.Clear();
            for (int i = 0; i < shootData.Shoots.Count / 6; i++)
                Zalps.Add(new OneZalpType(shootData.Shoots, i));
        }

        private int time_from_start = 0;
        private void ShootsToAllShoots(bool isDefence)
        {
            AllShoots.Clear();
            time_from_start = 0;
            int high_cnt;
            if (isDefence) high_cnt = 6;
            else high_cnt = shootData.Shoots.Count;
            //int CountOfUstUglomer = ((FormTrening.CountOfUstUglomer > 0) ? FormTrening.CountOfUstUglomer : 1);
            //int ScachocPricel = ((FormTrening.ScachocPricel > 0) ? FormTrening.ScachocPricel : 1);

            if (isDefence) time_from_start += 100;
            
            if (!isDefence) //если стреляем не на поражение
            {
                for (int ZalpCnt = 0; ZalpCnt < shootData.count_of_shoots; ZalpCnt++)
                {
                    time_from_start += shootData.time;
                    AddShotsItem(time_from_start, 0, high_cnt);
                }
            }
            else
            {
                MakeShotsDefenceItem();
                for (int i = 0; i < Zalps.Count; i++)
                {
                    for (int ZalpCnt = 0; ZalpCnt < shootData.count_of_shoots; ZalpCnt++)
                    {
                        time_from_start += shootData.time;
                        AddOneZalpDefenceItem(Zalps[i], time_from_start);
                    }
                    time_from_start += shootData.time;
                }
            }
        }

        private int count_pristrelka = 1;
        private int count_fire_defence = 1;


        

        private void GetOgzPgz(int i, out TPGZ pgz, out TOGZ ogz, 
            out double delta_x_razr, out double delta_y_razr, 
            out int dist_r_op, out DelUgl otklonenie_po_napr_knp,
            out DelUgl DirAngleRazrOp, out long xrazr, out long yrazr, out int VD)
        {
            int delta_x_mil;
            int pricel;

            pricel = AllShoots[i].Pricel + AllShoots[i].Uroven.ToInt32() - 3000;

            if (currentVdCount < NextVD.Count)
            {
                VD = NextVD[currentVdCount];
                currentVdCount++;
            }
            else if ((shootData.isFireNaPorajenie) && (FormTrening.baseData.solved_data.PS.ToInt32() < 500))
                VD = w_rnd.Next(
                    -(int)(Math.Round((double)(FormTrening.baseData.solved_data.delta_x_mil * 0.7))),
                     (int)(Math.Round((double)(FormTrening.baseData.solved_data.delta_x_mil * 0.7)))
                    );
            else VD = w_rnd.Next(-shootData.VD, shootData.VD);
            dist_r_op = GetDistFromPricerl_TS(pricel, out delta_x_mil) + VD
                                  - (int)(FormTrening.baseData.solved_data.dist_correcture);
            
            pgz = new TPGZ();
            DirAngleRazrOp = 
                new DelUgl(Math.Abs(AllShoots[i].Dovorot.ToInt32())
                      - FormTrening.baseData.solved_data.angle_correcture.ToInt32());
            pgz.SolvePGZ(FormTrening.baseData.Op.x, FormTrening.baseData.Op.y, dist_r_op, DirAngleRazrOp);

            // теперь в pgz лежит x разрыва и y разрыва
            ogz = new TOGZ();

            xrazr = pgz.Xtarget;
            yrazr = pgz.Ytarget;

            if (!is_np)
                ogz.SolveOGZ(FormTrening.baseData.Knp.x, FormTrening.baseData.Knp.y,
                             pgz.Xtarget, pgz.Ytarget,   FormTrening.baseData.AlphaOn);
            else
                ogz.SolveOGZ(FormTrening.baseData.Np.x, FormTrening.baseData.Np.y,
                             pgz.Xtarget, pgz.Ytarget,  FormTrening.baseData.AlphaOn);

            

            if (FormTrening.isFirstFire)
            {
                PreNodeFinalList.Add(new FormFinalDraw.NodeFinalShot(
                    (int)pgz.Xtarget, (int)pgz.Ytarget,
                    "Первый выстрел", Color.DarkRed, 15));
            }
            else if (!shootData.isFireNaPorajenie)
            {
                PreNodeFinalList.Add(new FormFinalDraw.NodeFinalShot(
                    (int)pgz.Xtarget, (int)pgz.Ytarget,
                    "Пристрелка, выстрел " + count_pristrelka.ToString(),
                    Color.DarkCyan, 15));
                count_pristrelka++;
            }
            else
            {
                PreNodeFinalList.Add(new FormFinalDraw.NodeFinalShot(
                    (int)pgz.Xtarget, (int)pgz.Ytarget, "", Color.Blue, 7));
                count_fire_defence++;
            }
            // теперь в ogz лежит дирекционный разрыва и дальность разрыва с кнп 
            otklonenie_po_napr_knp = new DelUgl();
            
            if (!is_np)
            {
                otklonenie_po_napr_knp.dov_fromInt32(ogz.α_c_du.ToInt32() - FormTrening.baseData.DirectionAngleFromKnp.ToInt32());
                delta_x_razr = ogz.Dct * Math.Cos(otklonenie_po_napr_knp.ToRadians())
                    - FormTrening.baseData.DistCommander;
            }
            else
            {
                otklonenie_po_napr_knp.dov_fromInt32(ogz.α_c_du.ToInt32() - FormTrening.baseData.DirectionAngleFromNp.ToInt32());
                delta_x_razr = ogz.Dct * Math.Cos(otklonenie_po_napr_knp.ToRadians())
                    - FormTrening.baseData.DistCommanderNp;
            }
            delta_y_razr = ogz.Dct * Math.Sin(otklonenie_po_napr_knp.ToRadians());


            if (i == 0) shootData.deltaX = delta_x_mil;
        }


        bool verify_kill(double delta_x_razr, double delta_y_razr, int kill_radius)
        {
            return (Math.Abs(delta_x_razr) < (kill_radius + FormTrening.baseData.DepthTarget / 2)) // спросить у Шалагина про расстояние поражения
                 && (Math.Abs(delta_y_razr) < (kill_radius + FormTrening.baseData.solved_data.FrontTarget_m / 2));
        }

        void countPopadanii(double delta_x_razr, double delta_y_razr, ref int count_plus,
            ref int count_minus, ref int count_popadanii)
        {
            if (delta_x_razr > 0) count_plus++; else count_minus++;

            // если цель поражается
            if (
                ((!shootData.isFireNaPorajenie) && (verify_kill(delta_x_razr, delta_y_razr, 10))) ||
                ((shootData.isFireNaPorajenie) && (verify_kill(delta_x_razr, delta_y_razr, 25)))
                )
                count_popadanii++;
        }


        void parseNabludenie(int count_popadanii, int count_plus, int count_minus)
        {
            if ((count_popadanii > (AllShoots.Count * 2) / 3))
                shootData.nabludenie = ShootData.Nabludenie.KillTarget;
            else if (
                        (count_plus == AllShoots.Count) || 
                        (AllShoots.Count == (count_popadanii + count_plus))
                    )
                shootData.nabludenie = ShootData.Nabludenie.AllPerelet;
            else if (
                        (count_minus == AllShoots.Count) ||
                        (AllShoots.Count == (count_popadanii + count_minus))
                    )
                shootData.nabludenie = ShootData.Nabludenie.AllNedolet;
            else if (count_minus > count_plus)
                shootData.nabludenie = ShootData.Nabludenie.MoreNedolet;
            else if (count_minus < count_plus)
                shootData.nabludenie = ShootData.Nabludenie.MorePerelet;
            else if (count_minus == count_plus)
                shootData.nabludenie = ShootData.Nabludenie.HighOlklCenterZalp;
        }

        static int cnt_zalp = 0;



        class SrednData
        {
            public long  xrazr_curr, yrazr_curr, x_razr_sr, y_razr_sr;
            public int  dist_r_op;
            public DelUgl DirAngleRazrOp;
            public TOGZ ogz;

            public long x_razr_sum      = 0, 
                        y_razr_sum      = 0,
                        dist_expl_op    = 0,
                        sum_DalnostExpl = 0, 
                        sum_DirectExpl  = 0,
                        sum_otkl_op     = 0;
            public DelUgl curr_otkl = new DelUgl();
            public TPGZ pgz;
            public DelUgl OtklCenterExpl_op = new DelUgl();
            public DelUgl otklonenie_po_napr_knp;
            public double dx;
            public double dy;
            public int dr;
            public int dugl;
            public int min_dr;

            public void count()
            {
                x_razr_sum += xrazr_curr;
                y_razr_sum += yrazr_curr;

                dist_expl_op += dist_r_op;
                sum_DalnostExpl += (int)ogz.Dct;
                sum_DirectExpl += ogz.α_c_du.ToInt32();
                curr_otkl.dov_fromInt32(DirAngleRazrOp.ToInt32() - FormTrening.baseData.solved_data.TargetDirAngleFromOp.ToInt32());
                sum_otkl_op += (long)curr_otkl.ToInt32();
            }

            public void post_count(List<ShootData.ShootCommand> AllShoots)
            {
                x_razr_sr = (long)Math.Round((double)x_razr_sum / (double)AllShoots.Count);
                y_razr_sr = (long)Math.Round((double)y_razr_sum / (double)AllShoots.Count);
                
                OtklCenterExpl_op.dov_fromInt32((int)Math.Round(Convert.ToDouble(sum_otkl_op) / (double)AllShoots.Count));

                dist_expl_op = (int)Math.Round((double)dist_expl_op / (double)AllShoots.Count);

                dx = x_razr_sr - FormTrening.baseData.solved_data.Xc;
                dy = y_razr_sr - FormTrening.baseData.solved_data.Yc;

                dr = (int)dist_expl_op - (int)FormTrening.baseData.solved_data.Dct;
                dugl = OtklCenterExpl_op.ToInt32();
                if (FormTrening.baseData.solved_data.PS.ToInt32() < 500)
                    min_dr = (int)Math.Round((double)FormTrening.baseData.solved_data.delta_x_mil);
                else
                    min_dr = 25;
            }

            public void make_expl_data(ShootData shootData)
            {
                shootData.OtklCenterRazrFromOp.fromInt32(dugl);
                shootData.otkl_dist_expl_trgt = dr;
            }

        }

        private void FormShowShooting_Load(object sender, EventArgs e)
        {
            shootData.is_cancel = false;
            ShootsToAllShoots(shootData.isFireNaPorajenie);
            shootData.ResultShootsList.Clear();
            String swfPath = Directory.GetCurrentDirectory() + Path.DirectorySeparatorChar + "suo.swf";
            if (!is_np)
                this.axShockwaveFlash1.LoadMovie(0, swfPath);

            if (is_np) this.Text = "Наблюдение с НП";
            else this.Text = "Наблюдение с КНП";

            // создание прокси
            if (!is_np)
            {
                proxy = new ExternalInterfaceProxy(axShockwaveFlash1);
                proxy.ExternalInterfaceCall += new ExternalInterfaceCallEventHandler(proxy_ExternalInterfaceCall);
                proxy.Call("SetColorSmoke", 0x7B7668);
                proxy.Call("SetColorTarget", 0x330066);
            }

            if (is_np)
            {
                this.buttonCancel.Enabled = false;
                this.buttonCancel.Visible = false;
            }

            if (FormTrening.baseData.DepthTarget > 100)
            {
                /*if (is_np)
                {
                    proxy.Call("DrawTargetHighPS", FormTrening.baseData.FrontTarget.ToInt32(),
                        (int)FormTrening.baseData.DepthTarget,  (int)FormTrening.baseData.DistCommanderNp);
                }
                else*/
                if (!is_np)
                    proxy.Call("DrawTargetHighPS", FormTrening.baseData.FrontTarget.ToInt32(),
                        (int)FormTrening.baseData.DepthTarget, (int)FormTrening.baseData.DistCommander);
            }
            else
            {
                /*if (is_np)
                {
                    proxy.Call("DrawTargetLowPS", FormTrening.baseData.FrontTarget.ToInt32(),
                        (int)FormTrening.baseData.DepthTarget, (int)FormTrening.baseData.DistCommanderNp);
                }
                else*/
                if (!is_np)
                    proxy.Call("DrawTargetLowPS", FormTrening.baseData.FrontTarget.ToInt32(),
                        (int)FormTrening.baseData.DepthTarget, (int)FormTrening.baseData.DistCommander);
            }
            
            double delta_x_razr;
            double delta_y_razr;
            int VD;
            SrednData sr_dt = new SrednData();
            
            
            

            int count_plus = 0, count_minus = 0, count_popadanii = 0;
            // стрельба ведется на поражение
            if (shootData.isFireNaPorajenie) 
            {   
                double min_front_r_m = Double.MaxValue, max_front_r_m = Double.MinValue;
                

                for (int i = 0; i < AllShoots.Count; i++)
                {
                    GetOgzPgz(i, out sr_dt.pgz, out sr_dt.ogz, out delta_x_razr, out delta_y_razr,
                        out sr_dt.dist_r_op, out sr_dt.otklonenie_po_napr_knp,
                        out sr_dt.DirAngleRazrOp,
                        out sr_dt.xrazr_curr, out sr_dt.yrazr_curr, out VD);
                    sr_dt.count();   
                 
                    if (delta_y_razr < min_front_r_m) min_front_r_m = delta_y_razr;
                    if (delta_y_razr > max_front_r_m) max_front_r_m = delta_y_razr;

                    if (!is_np)
                        proxy.Call("DrawExplosionIntervals", sr_dt.otklonenie_po_napr_knp.ToInt32(),
                            (int)delta_x_razr, 1, AllShoots[i].time_from_start / 10);
                    countPopadanii(delta_x_razr, delta_y_razr, ref count_plus, ref count_minus, ref count_popadanii);
                }
               
                double front_r_m = max_front_r_m - min_front_r_m;
                shootData.FrontRazr = new DelUgl(
                    (int)(Math.Round((double)front_r_m / (0.001 * (double)FormTrening.baseData.DistCommander)))
                    );

                shootData.DalnostExplDefence 
                    = (int)Math.Round((double)sr_dt.sum_DalnostExpl / (double)AllShoots.Count);
                shootData.DirectExplDefence 
                    = new DelUgl((int)Math.Round((double)sr_dt.sum_DirectExpl / (double)AllShoots.Count));
                
                sr_dt.post_count(AllShoots);

                cnt_zalp++;
                PreNodeFinalList.Add(new FormFinalDraw.NodeFinalShot(
                    (int)sr_dt.x_razr_sr, (int)sr_dt.y_razr_sr,
                    "Центр залпов " + cnt_zalp.ToString(),
                    Color.Black, 15));
                
                sr_dt.make_expl_data(shootData);

                //FormFinalDraw frmFinalDraw = new FormFinalDraw();
                //frmFinalDraw.init_pane(FormTrening.baseData);
                //frmFinalDraw.Show();

                if (Math.Abs(sr_dt.dr) < sr_dt.min_dr)
                {
                    if (Math.Abs(sr_dt.dugl) <= 2)
                        shootData.nabludenie = ShootData.Nabludenie.KillTarget;
                    else shootData.nabludenie = ShootData.Nabludenie.HighOlklCenterZalp;
                }
                else if ((sr_dt.dr >= sr_dt.min_dr) && 
                    (sr_dt.dr <= FormTrening.baseData.solved_data.diag_length_half_target))
                    shootData.nabludenie = ShootData.Nabludenie.EqualsFringeHigh;
                else if ((sr_dt.dr <= -sr_dt.min_dr) && 
                    (sr_dt.dr >= -FormTrening.baseData.solved_data.diag_length_half_target))
                    shootData.nabludenie = ShootData.Nabludenie.EqualsFringeLow;
                else 
                     parseNabludenie(count_popadanii, count_plus, count_minus);

            }
            else //если не ведется стрельба на поражение
            {
                for (int i = 0; i < AllShoots.Count; i++)
                {
                    GetOgzPgz(i, out sr_dt.pgz, out sr_dt.ogz, out delta_x_razr, out delta_y_razr, 
                        out sr_dt.dist_r_op,
                        out sr_dt.otklonenie_po_napr_knp, out sr_dt.DirAngleRazrOp,
                        out sr_dt.xrazr_curr, out sr_dt.yrazr_curr, out VD);
                    sr_dt.count();

                    if ((Math.Abs(sr_dt.otklonenie_po_napr_knp.ToInt32()) > 100) ||
                    (Math.Abs(delta_x_razr) > 500))
                    {
                        MessageBox.Show("Не заметил!");
                        shootData.nabludenie = ShootData.Nabludenie.NotDetected;
                        this.Close();
                    }
                    else  
                    {
                        countPopadanii(delta_x_razr, delta_y_razr, ref count_plus, ref count_minus, ref count_popadanii);
                        parseNabludenie(count_popadanii, count_plus, count_minus);
                        if (!is_np)
                            proxy.Call("DrawExplosionIntervals", sr_dt.otklonenie_po_napr_knp.ToInt32(),
                                (int)delta_x_razr, 1, AllShoots[i].time_from_start / 10);
                    }
                    ShootData.ResultShoot item 
                        = new ShootData.ResultShoot((int)sr_dt.ogz.Dct, sr_dt.ogz.α_c_du, delta_x_razr, VD);
                    shootData.ResultShootsList.Add(item);

                    
                }
                sr_dt.post_count(AllShoots);
                sr_dt.make_expl_data(shootData);
            }
            if (!is_np)
                proxy.Call("CloseFromTimer", time_from_start / 10 + 5);

            if (is_np) this.Close();
            else this.WindowState = FormWindowState.Maximized;
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            PreNodeFinalList.Clear();
            shootData.is_cancel = true;
            this.Close();
        }

        private void FormShowShooting_FormClosing(object sender, FormClosingEventArgs e)
        {
            FormFinalDraw.add_point(PreNodeFinalList);
            
        }
    }
}
