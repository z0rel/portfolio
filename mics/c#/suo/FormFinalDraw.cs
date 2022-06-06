using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using ZedGraph;

namespace SUOTrening
{
    public partial class FormFinalDraw : Form
    {
        public class NodeFinalShot
        {
            public NodeFinalShot(int x, int y, string name, Color color, int sizeFont)
            {
                this.x = x;
                this.y = y;
                this.name = name;
                this.color = color;
                this.sizeFont = sizeFont;
            }
            
            public int x, y;
            public string name;
            public Color color;
            public int sizeFont;
        }

        public static List<NodeFinalShot> NodeFinalList = new List<NodeFinalShot>();
        

        public FormFinalDraw()//TBaseData baseData)
        {
            InitializeComponent();
            DrawGraph();
        }


        private double f(double x)
        {
            if (x == 0)
            {
                return 1;
            }

            return Math.Sin(x) / x;
        }


        private double normalize_degree(double angle)
        {
            while ((angle < 0) || (360 < angle))
            {
                if (360 < angle) 
                    angle -= 360;
                if (angle < 0) 
                    angle += 360;

            }
            return angle;
        }

        private void draw_target(LineObj[] lines, TBaseData baseData, GraphPane pane,
            int front, int depth, int xc, int yc, Color color, int border_dist = 0)
        {
            double k = Math.Tan(-baseData.DirectionAngleFromKnp.ToRadians() - Math.PI);
            double l = 0.5 * (double)front + border_dist;
            double dx = l / Math.Sqrt(k * k + 1);
            double dy = k * dx;

            k = Math.Tan(-baseData.DirectionAngleFromKnp.ToRadians() - Math.PI / 2.0);
            l = 0.5 * (double)depth + border_dist;
            double dx2 = l / Math.Sqrt(k * k + 1);
            double dy2 = k * dx2;

            lines[0] = new LineObj(xc + dx2 - dx, yc + dy2 - dy, xc + dx2 + dx, yc + dy2 + dy);
            lines[1] = new LineObj(xc - dx2 - dx, yc - dy2 - dy, xc - dx2 + dx, yc - dy2 + dy);
            lines[2] = new LineObj(xc - dx2 - dx, yc - dy2 - dy, xc + dx2 - dx, yc + dy2 - dy);
            lines[3] = new LineObj(xc - dx2 + dx, yc - dy2 + dy, xc + dx2 + dx, yc + dy2 + dy);


            for (int i = 0; i < 4; i++)
            {
                lines[i].Line.Color = color;
                pane.GraphObjList.Add(lines[i]);
            }
            // подпись - "Цель"
            TextObj text_c = new TextObj("Цель", xc, yc - 2);
            text_c.FontSpec.Border.IsVisible = false;
            text_c.FontSpec.Size = 20;
            pane.GraphObjList.Add(text_c);
        }

        public void init_pane(TBaseData baseData)
        {
            GraphPane pane = zedGraphControl1.GraphPane;

            PointPairList listPoints = new PointPairList();

            int yc = (int)baseData.solved_data.Xc;
            int xc = (int)baseData.solved_data.Yc;
            listPoints.Add(xc, yc);


            int yop = (int)baseData.Op.x;
            int xop = (int)baseData.Op.y;
            listPoints.Add(xop, yop);

            int front = (int)baseData.solved_data.FrontTarget_m;
            int depth = (int)baseData.DepthTarget;
            
            // рисуем ОН
            double dx, dy, l = 16000, k;
            if (baseData.AlphaOn.ToInt32() == 0)
            {
                dx = 0;
                dy = l;
            }
            else
            {
                double angle_new = normalize_degree(-baseData.AlphaOn.ToDegree()) + 90.0;

                k = Math.Tan(angle_new * Math.PI / 180.0);
                dx = l / Math.Sqrt(k * k + 1);
                dy = k * dx;

                if ( (90 < angle_new) && (angle_new < 270))
                {
                    dy = -dy;
                    dx = -dx;
                }
            }
            LineObj line_on = new LineObj(xop, yop, xop + dx, yop + dy);
            pane.GraphObjList.Add(line_on);
            listPoints.Add(xop + dx, yop + dy);

            //рисуем цель
            LineObj[] lines = new LineObj[4];

            draw_target(lines, baseData, pane, front, depth, xc, yc, Color.Black);
            draw_target(lines, baseData, pane, front, depth, xc, yc, Color.Red, 25);

            //Рисуем ОП
            TextObj text_op = new TextObj("Оп", xop, yop - 250);
            text_op.FontSpec.Border.IsVisible = false;
            text_op.FontSpec.Size = 15;
            pane.GraphObjList.Add(text_op);

            ZedGraph.EllipseObj op = new EllipseObj(xop - 500, yop + 500, 1000, 1000);
            op.Fill.IsVisible = false;
            pane.GraphObjList.Add(op);

            //рисуем Кнп
            LineObj[] linesKnp = new LineObj[3];
            
            int yknp = (int)baseData.Knp.x;
            int xknp = (int)baseData.Knp.y;
            TextObj text_knp = new TextObj("Кнп", xknp, yknp - 250);
            text_knp.FontSpec.Border.IsVisible = false;
            text_knp.FontSpec.Size = 15;
            pane.GraphObjList.Add(text_knp);
            listPoints.Add(xknp, yknp);

            //рисуем КНП
            LineObj line_nabl = new LineObj(xknp, yknp, xc, yc);
            pane.GraphObjList.Add(line_nabl);
            LineObj line_fire = new LineObj(xop, yop, xc, yc);
            pane.GraphObjList.Add(line_fire);

            linesKnp[0] = new LineObj(xknp - 500, yknp - 500, xknp + 500, yknp - 500);
            linesKnp[1] = new LineObj(xknp, yknp + 500, xknp - 500, yknp - 500);
            linesKnp[2] = new LineObj(xknp, yknp + 500, xknp + 500, yknp - 500);
            for (int i = 0; i < 3; i++) pane.GraphObjList.Add(linesKnp[i]);
       
            //рисуем НП
            if (baseData.Np.x > 0)
            {
                yknp = (int)baseData.Np.x;
                xknp = (int)baseData.Np.y;
                TextObj text_np = new TextObj("Нп", xknp, yknp - 250);
                text_np.FontSpec.Border.IsVisible = false;
                text_np.FontSpec.Size = 15;
                pane.GraphObjList.Add(text_np);

                LineObj[] linesNp = new LineObj[3];
                linesNp[0] = new LineObj(xknp - 500, yknp - 500, xknp + 500, yknp - 500);
                linesNp[1] = new LineObj(xknp, yknp + 500, xknp - 500, yknp - 500);
                linesNp[2] = new LineObj(xknp, yknp + 500, xknp + 500, yknp - 500);
                for (int i = 0; i < 3; i++) pane.GraphObjList.Add(linesNp[i]);
                listPoints.Add(xknp, yknp);
            }
            LineItem myCurve = pane.AddCurve("", listPoints, Color.Blue, SymbolType.Diamond);

            myCurve.Line.IsVisible = false;
            myCurve.Symbol.Fill.Color = Color.Blue;
            myCurve.Symbol.Fill.Type = FillType.Solid;
            myCurve.Symbol.Size = 7;

            pane.YAxis.MajorGrid.IsZeroLine = false;

            pane.XAxis.Scale.Format = "F3";
            pane.YAxis.Scale.Format = "F3";

            //масштабирование      
            pane.XAxis.Scale.Min = xc - front;
            pane.XAxis.Scale.Max = xc + front;
            pane.YAxis.Scale.Min = yc - front;
            pane.YAxis.Scale.Max = yc + front;

            zedGraphControl1.AxisChange();
            zedGraphControl1.Invalidate();
        }
        
        class point
        {
            public double x, y;
        }

        private int count_point = 0;

        point[] Pnt;

        private void DrawGraph()
        {
            GraphPane pane = zedGraphControl1.GraphPane;

            Pnt = new point[3];
            for (int i = 0; i < 3; i++) Pnt[i] = new point();

            pane.Title.Text = "";
            pane.XAxis.Title.Text = "Y";
            pane.YAxis.Title.Text = "X";
            
            pane.CurveList.Clear();

            // Нарисуем стрелку, указыающую на максимум функции
            // Координаты точки, куда указывает стрелка
            // Координаты привязаны к осям
            double xend = 0.0;
            double yend = f(0);

            // Координаты точки начала стрелки
            double xstart = xend + 5.0;
            double ystart = yend + 0.1;

            // Обновляем график
            pane.XAxis.MajorGrid.IsVisible = true;

            // Задаем вид пунктирной линии для крупных рисок по оси X:
            // Длина штрихов равна 10 пикселям, ... 
            pane.XAxis.MajorGrid.DashOn = 10;

            // затем 5 пикселей - пропуск
            pane.XAxis.MajorGrid.DashOff = 0;
            
            // Включаем отображение сетки напротив крупных рисок по оси Y
            pane.YAxis.MajorGrid.IsVisible = true;
            
            // Аналогично задаем вид пунктирной линии для крупных рисок по оси Y
            pane.YAxis.MajorGrid.DashOn = 10;
            pane.YAxis.MajorGrid.DashOff = 0;
            
            // Включаем отображение сетки напротив мелких рисок по оси X
            pane.YAxis.MinorGrid.IsVisible = true;

            // Задаем вид пунктирной линии для крупных рисок по оси Y: 
            // Длина штрихов равна одному пикселю, ... 
            pane.YAxis.MinorGrid.DashOn = 1;

            // затем 2 пикселя - пропуск
            pane.YAxis.MinorGrid.DashOff = 2;

            // Включаем отображение сетки напротив мелких рисок по оси Y
            pane.XAxis.MinorGrid.IsVisible = true;

            // Аналогично задаем вид пунктирной линии для крупных рисок по оси Y
            pane.XAxis.MinorGrid.DashOn = 1;
            pane.XAxis.MinorGrid.DashOff = 2;

            label_x.Text = "";
            label_y.Text = "";
          
            buttonClear_Click(null, null);
        }


        // Обработка события MouseClick - клик по графику
        private void zedGraph_MouseClick(object sender, MouseEventArgs e)
        {
            // Сюда будет сохранена кривая, рядом с которой был произведен клик
            CurveItem curve;

            // Сюда будет сохранен номер точки кривой, ближайшей к точке клика
            int index;

            GraphPane pane = zedGraphControl1.GraphPane;

            // Максимальное расстояние от точки клика до кривой в пикселях, 
            // при котором еще считается, что клик попал в окрестность кривой.
            GraphPane.Default.NearestTol = 10;

            bool result = pane.FindNearestPoint(e.Location, out curve, out index);

            if (result)
            {
                // Максимально расстояние от точки клика до кривой не превысило NearestTol

                label_x.Text = curve[index].Y.ToString();
                label_y.Text = curve[index].X.ToString();

                if (count_point < 3)
                {
                    Pnt[count_point].x = curve[index].Y;
                    Pnt[count_point].y = curve[index].X;
                    if (count_point == 0)
                    {
                        label_x1.Text = Pnt[count_point].x.ToString();
                        label_y1.Text = Pnt[count_point].y.ToString();
                    }
                    else if (count_point == 1)
                    {
                        label_x2.Text = Pnt[count_point].x.ToString();
                        label_y2.Text = Pnt[count_point].y.ToString();
                    }
                    else if (count_point == 2)
                    {
                        label_x3.Text = Pnt[count_point].x.ToString();
                        label_y3.Text = Pnt[count_point].y.ToString();
                    }
                    count_point++;
                }
                else count_point = 0;
            }
        }
        
        private void FormFinalDraw_Load(object sender, EventArgs e)
        {
            label_x1.Text       = "";
            label_x2.Text       = "";
            label_x3.Text       = "";
            label_y1.Text       = "";
            label_y2.Text       = "";
            label_y3.Text       = "";
            label_dist.Text     = "";
            label_angle.Text    = "";
            add_points_to_pane();
        }
        
        public void add_points_to_pane()
        {
            try
            {
                GraphPane pane = zedGraphControl1.GraphPane;
                foreach (NodeFinalShot exemplar in FormFinalDraw.NodeFinalList)
                {
                    TextObj text_knp = new TextObj(exemplar.name, exemplar.y, exemplar.x - 0.5);
                    text_knp.FontSpec.Border.IsVisible = false;
                    text_knp.FontSpec.Size = 10;
                    pane.GraphObjList.Add(text_knp);

                    PointPairList listPoints = new PointPairList();
                    listPoints.Add(exemplar.y, exemplar.x);
                    LineItem myCurve = pane.AddCurve("", listPoints,
                        exemplar.color, SymbolType.Diamond);
                    myCurve.Line.IsVisible = false;
                    myCurve.Symbol.Fill.Color = exemplar.color;
                    myCurve.Symbol.Fill.Type = FillType.Solid;
                    myCurve.Symbol.Size = exemplar.sizeFont;
                }
            }
            catch (Exception e)
            {

            }
        }

        public static void add_point(List<NodeFinalShot> PreNodeFinalList)
        {
            foreach (NodeFinalShot exemplar in PreNodeFinalList)
                FormFinalDraw.NodeFinalList.Add(exemplar);
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
            count_point = 0;
            label_x1.Text = "";
            label_y1.Text = "";
            label_x2.Text = "";
            label_y2.Text = "";
            label_x3.Text = "";
            label_y3.Text = "";
            for (int i = 0; i < 3; i++)
            {
                Pnt[i].x = 0;
                Pnt[i].y = 0;
            }
        }

        private void buttonSolved_Click(object sender, EventArgs e)
        {
            double ax = Pnt[0].x - Pnt[1].x;
            double ay = Pnt[0].y - Pnt[1].y;
            double bx = Pnt[2].x - Pnt[1].x;
            double by = Pnt[2].y - Pnt[1].y;
            double dist = Math.Sqrt(ax * ax + ay * ay);

            double cos_angle = (ax * bx + ay * by) /
                     Math.Sqrt((ax * ax + ay * ay) * (bx * bx + by * by));

            DelUgl angle = new DelUgl();
            angle.FromDegree(DelUgl.RadToGrad(Math.Acos(cos_angle)));

            label_dist.Text = ((int)Math.Round(dist)).ToString();
            label_angle.Text = angle.ToString(false);
        }

        private void button_max_Click(object sender, EventArgs e)
        {
            GraphPane myPane = zedGraphControl1.GraphPane;
            double size = 1.0 / Convert.ToDouble(maskedTextBox_koeff_size.Text);

            PointF mitte = new PointF();
            mitte.X = zedGraphControl1.ClientSize.Width/2;
            mitte.Y = zedGraphControl1.ClientSize.Height/2;

            zedGraphControl1.ZoomPane(myPane, size, mitte, true);
        }

        private void button_min_Click(object sender, EventArgs e)
        {
            GraphPane myPane = zedGraphControl1.GraphPane;
            double size = Convert.ToDouble(maskedTextBox_koeff_size.Text);

            PointF mitte = new PointF();
            mitte.X = zedGraphControl1.ClientSize.Width / 2;
            mitte.Y = zedGraphControl1.ClientSize.Height / 2;

            zedGraphControl1.ZoomPane(myPane, size, mitte, true);
        }

        private void button_up_Click(object sender, EventArgs e)
        {
            double size = Convert.ToDouble(maskedTextBox_koeff_move.Text);
            zedGraphControl1.GraphPane.YAxis.Scale.Max += size;
            zedGraphControl1.GraphPane.YAxis.Scale.Min += size;
            zedGraphControl1.AxisChange();
            zedGraphControl1.Invalidate();
        }

        private void button_right_Click(object sender, EventArgs e)
        {
            double size = Convert.ToDouble(maskedTextBox_koeff_move.Text);
            zedGraphControl1.GraphPane.XAxis.Scale.Max += size;
            zedGraphControl1.GraphPane.XAxis.Scale.Min += size;
            zedGraphControl1.AxisChange();
            zedGraphControl1.Invalidate();
        }

        private void button_down_Click(object sender, EventArgs e)
        {
            double size = Convert.ToDouble(maskedTextBox_koeff_move.Text);
            zedGraphControl1.GraphPane.YAxis.Scale.Min -= size;
            zedGraphControl1.GraphPane.YAxis.Scale.Max -= size;
            zedGraphControl1.AxisChange();
            zedGraphControl1.Invalidate();
        }

        private void button_left_Click(object sender, EventArgs e)
        {
            double size = Convert.ToDouble(maskedTextBox_koeff_move.Text);
            zedGraphControl1.GraphPane.XAxis.Scale.Min -= size;
            zedGraphControl1.GraphPane.XAxis.Scale.Max -= size;
            zedGraphControl1.AxisChange();
            zedGraphControl1.Invalidate();
        }

        private void zedGraphControl_KeyPress(object sender, KeyPressEventArgs e)
        {
            if ((e.KeyChar == '+') || (e.KeyChar == '='))
                button_max_Click(null, null);
            else if (e.KeyChar == '-')
                button_min_Click(null, null);
            else if ((e.KeyChar == 'w') || (e.KeyChar == 'W') || (e.KeyChar == 'ц') || (e.KeyChar == 'Ц'))
                button_up_Click(null, null);
            else if ((e.KeyChar == 's') || (e.KeyChar == 'S') || (e.KeyChar == 'ы') || (e.KeyChar == 'Ы'))
                button_down_Click(null, null);
            else if ((e.KeyChar == 'a') || (e.KeyChar == 'A') || (e.KeyChar == 'ф') || (e.KeyChar == 'Ф'))
                button_left_Click(null, null);
            else if ((e.KeyChar == 'd') || (e.KeyChar == 'D') || (e.KeyChar == 'в') || (e.KeyChar == 'В'))
                button_right_Click(null, null);
        }


    }
}
