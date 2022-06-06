using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Data.OleDb;
using System.IO;
using System.Diagnostics;
using System.Threading;

namespace SUOTrening
{
    public partial class FormTrening : Form
    {
        public static FormTrening thisFrmTreining;

        public FormTrening()
        {
            InitializeComponent();
            
            Log.Visible = false;
            ButtonShowOrientList.Visible = false;
            ButtonCommand.Visible = false;
            ButtonReady.Visible = false;
            GroupBoxCommand.Visible = false;
            TextBoxCommand.Visible = false;
            LabelCommandText.Visible = false;
            MistakesCount = 0;
            OPName = "\"Лена\"";
            CNPName = "\"Дон\"";
            NACHName = "\"Нева\"";
            NACHSHTABname = "\"Висла\"";
            toolStripStatusLabel1.Text = "";
            thisFrmTreining = this;
        }

        public void initialize_callback(int ScachPricel, int CountOfUglUst, int Pricel, bool is_sign_pricel,
            string veer, string Uroven, string Dovorot)
        {
            if (ScachPricel != 0)
                this.numericUpDownScachPricel.Value = ScachPricel;
            if (CountOfUglUst != 0) //&& numericUpDownCountOfUglUst.Visible)
                this.numericUpDownCountOfUglUst.Value = CountOfUglUst;
            if (Pricel != 0)
                this.TexBoxPricel.Text = ((is_sign_pricel) ? ((0 < Pricel) ? "+" : "") : "") + Pricel.ToString();
            
            this.textBoxVeer.Text = veer;
            //if (textBoxUroven.Visible)
                this.textBoxUroven.Text  = Uroven;
            this.textBoxDovorot.Text = Dovorot;
        }

        public const int VD_defence = 25;
        public const int VD_shootNZR = 15;
        public const int VD_shootDalnomer = 15;
        public const int VD_shootSN = 15;
        public const int VD_firstFire = 50;
        
        FormOrientirsList formOrientires; //Форма для отображения списка ориениров
        FormShowShooting formShowShooting; //Форма отрисовки наблюдений стрельбы
        FormBaseData formBaseData;
        StreamWriter fileWriter; //Запись в файл
        Correctures correctures, correctures_copy;

        Stopwatch stopWatch = new Stopwatch();
        Stopwatch dopWatch = new Stopwatch();
        //int dopSeconds = 0;


        private DelUgl get_r(DelUgl l, DelUgl r)
        {
            if (baseData.solved_data.PS.ToInt32() >= 500)
                 return l;
            else return r;   
        }

        private DelUgl get_l(DelUgl l, DelUgl r)
        {
            if (baseData.solved_data.PS.ToInt32() >= 500)
                 return r;
            else return l;
        }

        //Тип задачи
        public enum TaskType
        {
            DMC = 0,
            DBC = 1,
            NMC = 2,
            NBC = 3,
            SMC = 4,
            SBC = 5
        }

        //Вид поданной команды
        public enum CommandType
        {
            Osn1Snar = 0, //Основному 1 снаряд огонь
            Osn3Snar = 1, //Основному 3 снаряда 30 секунд выстрел огонь
            Batr2Snar = 2, //Батарея 2 снаряда беглым огонь
            Batr4Snar = 3, //Батарея 4 снаряда беглым огонь
            StopWrite = 4 //Стой. Записать
        }

        public int Count_of_shoots, Count_of_shoots_current; //Расход снарядов
        public int Count_of_shoots_for_pristrelka, Count_of_shoots_for_pristrelka_current; //Расход снарядов на пристрелку
        public int Count_of_zalps, Count_of_zalps_current;  //Число залпов
        public int Count_of_good_zalps; //Число точных залпов
        public int MistakesCount, MistakesCount_current; //Количество ошибок
        public int MistakesUstUglScachPricelCount; //Количество ошибок в определении скачка прицела и установок угломера

        public Int64 DistFromCommanderToTarget;     //Дальность командира
        public DelUgl DirectAngleFromKNPToTarget; //Дирекционный по цели 

        public bool isCommandForDalnomershikToZasechkaCeli; //Подана ли команда Засечь и обслужить пристрелку
        public bool isCommandForKVUToDokladFrontAndVisota; //Подана ли команда Доложить фронт и высоту цели
        public bool isCommandForKNPandNPToDokladDirectAnglesOfTarger; //Подана ли команда доложить дирекционные углы с КНП и НП

        public static bool isFirstFire; //Первый выстрел производится или последующие
        public bool isFireNaPoroajenie; //Ведётся ли стрельба на поражение: был ли скомандован до этого залп
        public bool isSecoundFire;      //Был произведен только первый выстрел
        public bool isFinish;           //Окончить стрельбу или нет

        public bool isFireNaPoroajenie_copy; 
        public bool isSecoundFire_copy;      
        public bool isFinish_copy;

        public bool isVeerEntered, isVeerEntered_copy;      //Был ли введен веер

        public bool WriteUstUglomer;    //Писать ли об установках угломера
        public bool WriteScachocPricel; //Писать ли об установках прицела
        public bool WriteEndIfTreningInFile = true; //Записывать ли в файл сообщение об окончании тренировки

        public int MistakeUroven; //Ошибка в определении уровня в малых делениях угломера со своим знаком
        public int MistakeCorrectureDistance; //Ошибка в определении корректур по дальности (м)
        public DelUgl MistakeCorrectureNapravlenie; //Ошибка в определении корректур по направлению (д. у.)
        public DelUgl MistakeCorrectureVeer; //Ошибка в определении корректур веера (д. у.)
        public int MistakeFireDistance; //отклонение точности огня по дальности (м) (при переходе к стрельбе на поражение)
        public DelUgl MistakeFireNapravlenie = new DelUgl(); //отклонение точности огня по направлению (д. у.) (при переходе к стрельбе на поражение)

        public int CorrecturePricel; //Текущая корректура прицела
        public DelUgl CorrectureDovorot; //Текущая корректура доворота
        public DelUgl CorrectureVeer = new DelUgl(); //Текущая корректура веера

        public int NablDistRazr, NablDistRazr_copy;  //Наблюдаемая дальность до разрыва от КНП
        public DelUgl NablDovrazr, NablDovrazr_copy; //Наблюдаемое с КНП отклонение разрыва по направлению
        public DelUgl NablDovrazrNP; //Наблюдаемое с НП отклонение по направлению
        public DelUgl NablFrontRazr, NablFrontRazr_copy; //Наблюдаемый фронт разрывов с КНП
        public double Otnoshenie_FrontRazr_To_FrontTarget = 0.0; //Отношение фронта разрывов к фронту цели

        public ShootData.Nabludenie OldNabl = ShootData.Nabludenie.KillTarget;
        public ShootData.Nabludenie OldNabl_copy;

        public string Surname;
        public static bool is_controlnaja; //Сдается ли контрольная стрельба
        public static TaskType taskType; //Тип решаемой задачи
        CommandType Command, PrevCommand;  //Текущая и предыдущая команды
        CommandType PrevCommand_copy; //Копия предыдущей команды
        public static TBaseData baseData; //Входные данные по задаче
        public ShootData shootData; //Данные по производимому выстрелу
        public ShootData shootDataNP; //Данные по производимому выстрелу относительно НП (при пристрелке СН)
        public ShootData shootData_copy = new ShootData(); //Копия данных по последнему произведенному выстрелу выстрелу
        public int Pricel;
        public DelUgl Uroven;
        public DelUgl Dovorot;
        public DelUgl Veer;
        public static int CountOfUstUglomer;
        public static int ScachocPricel;

        public int minutes0, seconds0, hours0, minutes, seconds, hours; //Минтуты, секунды, часы начала и конца отсечки времени
        public int minutesSumm, secondsSumm; //Минуты и секунды общего времени выполнения 

        public static string OPName;
        public static string CNPName;
        public static string NACHName;
        public static string NACHSHTABname;

        public string commandText; //Текст команды, подаваемой пользователем

        public string filename; //имя файла лога

        public bool is_first_defence_shoot = false;
        public bool is_first_defence_shoot_copy = false;

        //Посчитать оценку по времени выполнения задачи
        public string GetMarkByTime(int seconds)
        {
            int minForOtl, secForOtl, minForHor, secForHor, minForUdvl, secForUdvl;
            minForOtl = 4;
            minForHor = 5;
            minForUdvl = 6;
            secForOtl = secForHor = secForUdvl = 30;
            if (taskType == TaskType.DBC || taskType == TaskType.NBC || taskType == TaskType.SBC)
            {
                minForOtl++;
                minForHor++;
                minForUdvl++;
            }
            if (taskType == TaskType.SBC || taskType == TaskType.SMC)
            {
                minForOtl++;
                minForHor++;
                minForUdvl++;
            }
            secForOtl += 60 * minForOtl + 30 * (Count_of_zalps - 1);
            secForOtl += secForOtl / 2;

            secForHor += 60 * minForHor + 30 * (Count_of_zalps - 1);
            secForHor += secForHor / 2;

            secForUdvl += 60 * minForUdvl + 30 * (Count_of_zalps - 1);
            secForUdvl += secForUdvl / 2;

            if (seconds <= secForOtl)
                return "отлично";
            else
                if (seconds <= secForHor)
                    return "хорошо";
                else
                    if (seconds <= secForUdvl)
                        return "удовлетворительно";
                    else
                        if (seconds > secForUdvl)
                            return "неудовлетворительно";
                        else
                            return "произошла ошибка подсчета времени!";
        }

        //Посчитать оценку по решениям командира
        public string GetMarkByMistakes()
        {
            if (MistakesCount <= 2)
                return "отлично";
            if (MistakesCount == 3)
                return "хорошо";
            if (MistakesCount == 4)
                return "удовлетворительно";
            if (MistakesCount > 4)
                return "неудовлетворительно";
            return "произошла ошибка при выставлении оценки";
        }

        //Посчитать оценку по точности огня
        public string GetMarkByFire()
        {
            if (taskType == TaskType.DBC || taskType == TaskType.DMC || taskType == TaskType.SBC || taskType == TaskType.SMC)
            {
                int markDist, markNapr;
                if (MistakeFireDistance <= (int)Math.Round(0.008 * (double)DistFromCommanderToTarget))
                    markDist = 5;
                else
                    if (MistakeFireDistance <= (int)Math.Round(0.01 * (double)DistFromCommanderToTarget))
                        markDist = 4;
                    else
                        if (MistakeFireDistance <= (int)Math.Round(0.015 * (double)DistFromCommanderToTarget))
                            markDist = 3;
                        else
                            markDist = 2;
                if (MistakeFireNapravlenie.ToInt32() <= 3)
                    markNapr = 5;
                else
                    if (MistakeFireNapravlenie.ToInt32() <= 6)
                        markNapr = 4;
                    else
                        if (MistakeFireNapravlenie.ToInt32() <= 9)
                            markNapr = 3;
                        else
                            markNapr = 2;

                int mark;
                if (markDist < markNapr)
                    mark = markDist;
                else
                    mark = markNapr;

                switch (mark)
                {
                    case 5: return "отлично";
                    case 4: return "хорошо";
                    case 3: return "удовлетворительно";
                    case 2: return "неудовлетворительно";
                }
            }
            else
            {
                int markDist, markNapr;
                if (MistakeFireDistance <= (int)Math.Round(0.012 * (double)DistFromCommanderToTarget))
                    markDist = 5;
                else
                    if (MistakeFireDistance <= (int)Math.Round(0.015 * (double)DistFromCommanderToTarget))
                        markDist = 4;
                    else
                        if (MistakeFireDistance <= (int)Math.Round(0.023 * (double)DistFromCommanderToTarget))
                            markDist = 3;
                        else
                            markDist = 2;
                if (MistakeFireNapravlenie.ToInt32() <= 5)
                    markNapr = 5;
                else
                    if (MistakeFireNapravlenie.ToInt32() <= 9)
                        markNapr = 4;
                    else
                        if (MistakeFireNapravlenie.ToInt32() <= 14)
                            markNapr = 3;
                        else
                            markNapr = 2;

                int mark;
                if (markDist < markNapr)
                    mark = markDist;
                else
                    mark = markNapr;

                switch (mark)
                {
                    case 5: return "отлично";
                    case 4: return "хорошо";
                    case 3: return "удовлетворительно";
                    case 2: return "неудовлетворительно";
                }
            }

            return "произошла ошибка при выставлении оценки";
        }

        //перевести оценку в текстовой форме в числовую форму
        public int MarkToInt(string mark)
        {
            switch (mark)
            {
                case "отлично": return 5;
                case "хорошо": return 4;
                case "удовлетворительно": return 3;
                case "неудовлетворительно": return 2;
            }
            return 0;
        }

        //Посчитать итоговую оценку
        public string GetFinalMark()
        {
            int m1 = MarkToInt(GetMarkByMistakes());
            TimeSpan tmp = new TimeSpan(dopSeconds.Ticks);
            tmp += stopWatch.Elapsed;
            int m2 = MarkToInt(GetMarkByTime((int)(tmp.TotalSeconds)));
           
            int m3 = MarkToInt(GetMarkByFire());
            int min;
            if (m1 <= m2) min = m1;
            else min = m2;
            if (m3 <= min) min = m3;
            switch (min)
                {
                    case 5: return "отлично";
                    case 4: return "хорошо";
                    case 3: return "удовлетворительно";
                    case 2: return "неудовлетворительно";
                }
            return "произошла ошибка при выставлении оценки";
        }

        //Отображение списка ориетиров   
        private void ButtonShowOrientList_Click(object sender, EventArgs e)
        {
            OleDbConnection connection = new OleDbConnection("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=OrientirsDB.mdb;");
            try
            {
                connection.Open();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
                return;
            }

            formOrientires = new FormOrientirsList(connection, baseData.solved_data.PS.ToInt32());
            formOrientires.Show();        
        }

        //После выбора типа задачи и ввода фамилии. По нажатии стартовой кнопки Готов к выполнению задачи!
        private void ButtonStart_Click(object sender, EventArgs e)
        {
            if (TextBoxSurname.Text == "")
            {
                MessageBox.Show("Вы не ввели фамилию!", "Ошибка!");
            }
            else
            {
                Surname = TextBoxSurname.Text;
                if (Directory.Exists("Отчеты по стрельбе"))
                    filename = Directory.GetCurrentDirectory() + "\\Отчеты по стрельбе\\" + Surname;
                else
                {
                    Directory.CreateDirectory("Отчеты по стрельбе");
                    filename = Directory.GetCurrentDirectory() + "\\Отчеты по стрельбе\\" + Surname;
                }
                filename += " " + DateTime.Now.Date.ToShortDateString() + " " + DateTime.Now.Hour.ToString()
                    + "ч. " + DateTime.Now.Minute.ToString() + "мин..txt";
                try
                {
                    fileWriter = File.CreateText(filename);
                }
                catch
                {
                    filename = Directory.GetCurrentDirectory() + "\\" + Surname;
                    filename += " " + DateTime.Now.Date.ToShortDateString() + " " + DateTime.Now.Hour.ToString()
                        + "ч. " + DateTime.Now.Minute.ToString() + "мин..txt";
                    fileWriter = File.CreateText(filename);
                }
                if (RadioButtonDMC.Checked) taskType = TaskType.DMC;
                else if (RadioButtonDBC.Checked) taskType = TaskType.DBC;
                else if (RadioButtonNMC.Checked) taskType = TaskType.NMC;
                else if (RadioButtonNBC.Checked) taskType = TaskType.NBC;
                else if (RadioButtonSMC.Checked) taskType = TaskType.SMC;
                else if (RadioButtonSBC.Checked) taskType = TaskType.SBC;

                this.Text += " " + TaskTypeToString(taskType);

                is_controlnaja = CheckBoxControlnaja.Checked;

                TaskGenerator gen = new TaskGenerator();
                baseData = gen.MakeTask(taskType);
                TypesForm typeSolveForm = TypesForm.eDist;

                if      (taskType == TaskType.DBC || taskType == TaskType.DMC)
                    typeSolveForm = TypesForm.eDist;
                else if (taskType == TaskType.NBC || taskType == TaskType.NMC)
                    typeSolveForm = TypesForm.eNzr;
                else if (taskType == TaskType.SBC || taskType == TaskType.SMC)
                    typeSolveForm = TypesForm.eSN;

                formBaseData = new FormBaseData(typeSolveForm, baseData);
                if (!is_controlnaja)
                    formBaseData.Show();
   
                GroupBoxStartMenu.Hide();
                ButtonStart.Hide();
                Log.Show();
                ButtonReady.Show();

                //Определяем дальность командира и дирекционный угол по цели
                DirectAngleFromKNPToTarget = new DelUgl();
                DirectAngleFromKNPToTarget.fromDelUgl(baseData.DirectionAngleFromKnp);
                DistFromCommanderToTarget = baseData.DistCommander;
                //----------------------------------------------------------

                Log.Text += "Старший офицер батареи доложил:\n================================\n"
                            + CNPName + ", " + OPName + " к ведению огня готова.\n" +
                            "Координаты:   Х = " + baseData.Op.x.ToString() + ",  Y = " + baseData.Op.y.ToString() +
                            ", Высота: " + baseData.Op.h.ToString() + "\nОсновное направление: " + baseData.AlphaOn.ToString(false) +
                            "\nНаименьшие прицелы: заряды полный, третий, шестой; вправо: 36, 64, 98; прямо: 48, 76, 114; влево: 40, 72, 104" +
                            " Осколочно-фугасных: заряд уменьшеный переменный - 240, заряд полный - 48. Отклонение начальной скорости: минус 0,8%" +
                            ". Температура зарядов минус 10. Я " + OPName + "." + 
                            "\n\nКомандир взвода управления доложил:\n================================\n" +
                            "Товарищ капитан, КНП к работе готов.\nКоординаты пункта: Х = " + baseData.Knp.x.ToString() +
                            " Y = " + baseData.Knp.y.ToString() + " Высота: " + baseData.Knp.h.ToString();
                if (taskType == TaskType.SBC || taskType == TaskType.SMC)
                    Log.Text += "\nНП к работе готов.\nКоординаты пункта: Х = " + baseData.Np.x.ToString() +
                            " Y = " + baseData.Np.y.ToString() + " Высота: " + baseData.Np.h.ToString();
                Log.Text += "\n\nРаспоряжение начальника штаба дивизиона:" +
                            "\n================================\n" + 
                            "Внимание! Принять рассчитанные поправки. ОФ-462 Ж.\n" +
                            "Заряд " + baseData.TypeZarToString(true)+": ";

                foreach (TBaseData.NodeCorrectue corr in baseData.listNodeCorrectue)
                {
                    Log.Text += corr.SupportDist.ToString() + "   ";
                }

                Log.Text.Remove(Log.Text.Length - 4);
                Log.Text += " м. Поправки дальности: ";

                foreach (TBaseData.NodeCorrectue corr in baseData.listNodeCorrectue)
                {
                    char sign = ' ';
                    if (corr.CorrDist > 0) sign = '+';
                    Log.Text += sign + corr.CorrDist.ToString() + "   ";
                }

                Log.Text.Remove(Log.Text.Length - 4);
                Log.Text += ". Поправки направления: ";

                foreach (TBaseData.NodeCorrectue corr in baseData.listNodeCorrectue)
                {
                    Log.Text += corr.CorrAngle.ToString() + "   ";
                }

                Log.Text.Remove(Log.Text.Length - 4);
                Log.Text += ". Построить график рассчитаных поправок. Я " + NACHSHTABname + ".\n\n";

                Log.Select(Log.Text.Length - 1, Log.Text.Length - 1);
                Log.ScrollToCaret();
            }
        }

        //При докладе о готовностию При нажатии кнопки Готов!
        private void ButtonReady_Click(object sender, EventArgs e)
        {
            isCommandForDalnomershikToZasechkaCeli = false;
            isCommandForKVUToDokladFrontAndVisota = false;
            isCommandForKNPandNPToDokladDirectAnglesOfTarger = false;
            isVeerEntered = false;
            ButtonReady.Hide();
            LabelCommandText.Show();
            ButtonCommand.Show();
            TextBoxCommand.Show();
            ButtonShowOrientList.Show();
            isFinish = false;
            isFirstFire = true;
            isFireNaPoroajenie = false;
            isSecoundFire = false;
            Count_of_shoots = 0;
            Count_of_shoots_for_pristrelka = 0;
            Count_of_zalps = 0;
            Count_of_good_zalps = 0;
            //Запускаем таймер
            StartTimer();
            Log.Text += "Команда командира мотострелковой роты\n===========================\n" +
                        CNPName + ", подавить цель " + baseData.NumTarget.ToString() + ". " +
                        baseData.character+". На высоте Фигурная.\n";
            if (baseData.NumOrientir > 0) //Если по ориентиру
            {
                DelUgl deltaDistOrientire = new DelUgl();
                deltaDistOrientire.fromInt32((int)Math.Round(
                        (double)(baseData.listNodeOrientir[0].DeltaDist * 1000) /
                                     ((double)DistFromCommanderToTarget)));
                Log.Text += "Ориентир " + baseData.NumOrientir.ToString() + ", ";
                if (baseData.listNodeOrientir[0].DeltaAngle.sign < 0)
                    Log.Text += "влево " + baseData.listNodeOrientir[0].DeltaAngle.ToString(false);
                else
                    Log.Text += "вправо " + baseData.listNodeOrientir[0].DeltaAngle.ToString(false);
                Log.Text += ", ";
                string sign = "";
                if (baseData.listNodeOrientir[0].DeltaDist > 0) sign = "дальше ";
                else if (baseData.listNodeOrientir[0].DeltaDist < 0) sign = "ближе ";
                else sign = "дальность: ";
                if (taskType == TaskType.DBC || taskType == TaskType.DMC)
                    Log.Text += sign + (Math.Abs(baseData.listNodeOrientir[0].DeltaDist)).ToString()+ "\n";
                else
                    Log.Text += sign + deltaDistOrientire.ToString(false) + "\n";
                Log.Text += " Я " + NACHName + "\n";
                TextBoxCommand.Text = "Командир взвода управления, должить фронт и высоту цели!";
                isCommandForKVUToDokladFrontAndVisota = true;
            }
            else   //Если не по ориентиру
            {
                Log.Text += " Я " + NACHName + "\n";
                if (taskType == TaskType.SMC || taskType == TaskType.SBC)
                {//Если нужно приказывать КНП и НП доложить дирекционные по цели
                    TextBoxCommand.Text += "Доложить дирекционные улгы по цели с наблюдательных пунктов!";
                    isCommandForKNPandNPToDokladDirectAnglesOfTarger = true;
                    isCommandForDalnomershikToZasechkaCeli = false;
                    isCommandForKVUToDokladFrontAndVisota = false;
                }
                else //Если нужно приказывать дальномерщику
                {
                    TextBoxCommand.Text = "Дальномерщик, цель в перекрестьи, засечь и обслужить пристрелку! \n";
                    isCommandForDalnomershikToZasechkaCeli = true;
                    isCommandForKVUToDokladFrontAndVisota = false;
                    isCommandForKNPandNPToDokladDirectAnglesOfTarger = false;
                }
            }
            Log.Select(Log.Text.Length - 1, Log.Text.Length - 1);
            Log.ScrollToCaret();
            //Выводим в файл входные данные и исчисленные установки
            WriteIschislInFile(fileWriter, baseData, DistFromCommanderToTarget, DirectAngleFromKNPToTarget, taskType);
        }

        //По нажатии кнопки Подать команду
        private void ButtonCommand_Click(object sender, EventArgs e)
        {
            if (isCommandForDalnomershikToZasechkaCeli) //Если надо Засечь и обслужить пристрелку
            {
                Log.Text += "Доклад дальномерщика\n========================\n"+
                            "Цель вижу. Дирекционный: "+baseData.DirectionAngleFromKnp.ToString(false)+
                            ". Дальность: "+baseData.DistCommander.ToString();
                Log.Select(Log.Text.Length - 1, Log.Text.Length - 1);
                Log.ScrollToCaret();
                isCommandForDalnomershikToZasechkaCeli = false;
                TextBoxCommand.Text = "Командир взвода управления, должить фронт и высоту цели!";
                isCommandForKVUToDokladFrontAndVisota = true;
            }
            else
            {
                if (isCommandForKNPandNPToDokladDirectAnglesOfTarger) //если надо доложить дирекционные с КНП и НП
                {
                    Log.Text += "Доклад с КНП\n========================\n" +
                            "Цель вижу. Дирекционный: " + baseData.DirectionAngleFromKnp.ToString(false) + "\n";
                    Log.Text += "Доклад с НП\n========================\n" +
                            "Цель вижу. Дирекционный: " + baseData.DirectionAngleFromNp.ToString(false) + "\n";
                    Log.Select(Log.Text.Length - 1, Log.Text.Length - 1);
                    Log.ScrollToCaret();
                    isCommandForDalnomershikToZasechkaCeli = false;
                    isCommandForKNPandNPToDokladDirectAnglesOfTarger = false;
                    TextBoxCommand.Text = "Командир взвода управления, должить фронт и высоту цели!";
                    isCommandForKVUToDokladFrontAndVisota = true;
                }
                else
                if (isCommandForKVUToDokladFrontAndVisota) //Если надо доложить фронт и высоту
                {
                    Log.Text += "\nДоклад командира взвода управления\n========================\n" +
                            "Цель вижу. Фронт: " + baseData.FrontTarget.ToString(false) +
                            ". Глубина: " + baseData.DepthTarget.ToString() +
                            ". Угол места: " + baseData.EpsTarget.ToString();
                    Log.Select(Log.Text.Length - 1, Log.Text.Length - 1);
                    Log.ScrollToCaret();
                    GroupBoxCommand.Show();
                    LabelDovorot.Text += " OH ";
                    /*if ((taskType == TaskType.DBC) || (taskType == TaskType.DMC))
                        Log.Text += "\n\nВ вашем распоряжении дальномер.\n";
                    else  if ((taskType == TaskType.NBC) || (taskType == TaskType.NMC))
                            Log.Text += "\n\nВ вашем распоряжении разведчик.\n";
                    else  if (taskType == TaskType.SBC || taskType == TaskType.SMC)
                        Log.Text += "\n\nВ вашем распоряжении разведчик на КНП и разведчик на НП.\n";
                    */
                    //После доклада фронта и высоты подается команда на первый выстрел
                    commandText = OPName + ", стой! Цель " + baseData.NumTarget.ToString() + ", " +
                                          baseData.character + ". ОФ. Взрыватель ";
                    if (!baseData.isRicoshet()) commandText += "осколочный и фугасный";
                    else commandText += "замедленный";
                    commandText += ". Заряд " + baseData.TypeZarToString(true) + ". ";                  
                    commandText += "Шкала тысячных. ";
                    TextBoxCommand.Text = commandText;

                    isCommandForKVUToDokladFrontAndVisota = false;
                    return;
                }
            }

//############################### Если подается команда на выстрел #################################################################
            
            if ((!isCommandForDalnomershikToZasechkaCeli)&&(!isCommandForKVUToDokladFrontAndVisota))                  
            {
                //---------------------если производится первый выстрел----------------------------------------------------------

                    if (isFirstFire)                    
                    {
                        LabelUroven.Show();
                        textBoxUroven.Show();
                        
                        CorrectureDovorot = new DelUgl();
                        Pricel = 0;
                        Uroven = new DelUgl("00-00");
                        Dovorot = new DelUgl("00-00");
                        Veer = new DelUgl("00-00");

                        if ((!RadioButtonStopWrite.Checked) && (!RadioButton4snarFast.Checked) &&
                            (!RadioButton2snarFast.Checked) && (!RadioButton3snar30sec.Checked) &&
                            (!RadioButton1snar.Checked) && (!RadioButtonFire.Checked))
                        {
                            MessageBox.Show("Не выбран тип команды!");
                            return;
                        }

                        if (RadioButtonFire.Checked)
                        {
                            MessageBox.Show("Команда \"Огонь!\" подается, если ранее была подана другая команда!");
                            return;
                        }

                        if (!RadioButton1snar.Checked)
                        {
                            MessageBox.Show("Вы неверно производите начало пристрелки!",
                                            "Вы допустили ошибку!");
                            WriteErrorInFile("Попытка назначить более 1 выстрела на исчисленных установких!!!",true);
                            MistakesCount++;
                            return;
                        }

                        //Считываем установки для первого выстрела, введенные пользователем
                        if (ControlEnterUstanovkiForFirstFire() != 0) //Если некорректно введены установки
                            return;

                        isFireNaPoroajenie = false;
                        shootData.commandType = CommandType.Osn1Snar;

                        //Передаем установки пользователя для первого выстрела в shootData.UserCommand
                        SetUstanovkiToUserCommandInShootData(Pricel, Uroven, Dovorot, Veer, isFireNaPoroajenie, ref shootData);

                        MakeListOfShootsToSingleFire(1, 50); //назначаем первый выстрел с задержкой 50 сек.

                        //Считываем число установок угломера и скачок прицела и проверяем их на ошибочность 
                        SetAndCheckCountOfUglUstAndScachPricel();    

                        //Останавливаем таймер
                        StopTimer();


                        StartDopTimer();
                        //---------Показываем первый выстрел------------------------
                        formShowShooting = new FormShowShooting(ref shootData);
                        formShowShooting.ShowDialog(this);

                        StopDopTime(shootData.is_cancel);
                        if (shootData.is_cancel)
                        {
                            //BackUp();
                            return;
                        }

                        if (taskType == TaskType.SBC || taskType == TaskType.SMC)
                        {
                            shootDataNP = new ShootData();
                            CopyShootdata(shootData, ref shootDataNP,false);
                            List<int> vd_list = new List<int>();
                            vd_list.Add(shootData.ResultShootsList[0].VD);
                            formShowShooting = new FormShowShooting(ref shootDataNP, true, vd_list);
                            formShowShooting.ShowDialog(this);
                        }

                        TexBoxPricel.Focus();
                        //----------------------------------------------------------

                        Command = CommandType.Osn1Snar;
                        PrevCommand = CommandType.Osn1Snar;
                        Count_of_shoots++;
                        Count_of_shoots_for_pristrelka++;
                        MistakesCount += MistakesUstUglScachPricelCount;
                        LabelDovorot.Text = "Доворот:";
                        LabelUroven.Hide();
                        textBoxUroven.Hide();

                        NablDistRazr = shootData.ResultShootsList[0].DalnostExpl;
                        NablDovrazr = new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32() -
                            DirectAngleFromKNPToTarget.ToInt32());
                        if (taskType == TaskType.SBC || taskType == TaskType.SMC)
                            NablDovrazrNP = new DelUgl(shootDataNP.ResultShootsList[0].DirectExpl.ToInt32() -
                                baseData.DirectionAngleFromNp.ToInt32());
                        NablFrontRazr = new DelUgl();

                        //--------Определяем идельные установки для первого выстрела--------------------------------
                        correctures = new Correctures(baseData);
                        correctures_copy = new Correctures(baseData);
                        correctures.first_shot();
                        shootData.VerifyCommand.Pricel = (int)correctures.П;
                        shootData.VerifyCommand.Uroven.fromDelUgl(correctures.У);
                        shootData.VerifyCommand.Dovorot.fromDelUgl(new DelUgl(baseData.AlphaOn.ToInt32()+
                                                                                   correctures.dov.ToInt32()+6000));
                        shootData.VerifyCommand.Veer.fromInt32(baseData.solved_data.ShotVeer.ToInt32());
                        shootData.deltaX = (int)baseData.solved_data.delta_x_mil;
                        //--------------------------------------------------------------------------------------------

                        //-----------------------Вывод в файл первый выстрел-------------------------------------------
                        TextBoxCommand.Text = commandText;
                        //Выводим в файл текст команды на первый выстрел (со всеми установками)
                        WriteFireCommandInFile(Pricel, Uroven, Dovorot, CorrectureDovorot, Veer, correctures, fileWriter);

                        MistakeUroven = shootData.UserCommand.Uroven.ToInt32() - shootData.VerifyCommand.Uroven.ToInt32();
                        
                        //Выводим в файл наблюдения
                        WriteNabludenieOfFirstFireInFile();

                        //---------Выводим доклад дальномерщика или разведчика по первому выстрелу------------------------
                        OutputNabludenieOfFirstFire();
                        //-------------------------------------------------------------------------------------------------

                        OldNabl = shootData.nabludenie;

                        TexBoxPricel.Text = "";
                        textBoxUroven.Text = "";
                        textBoxDovorot.Text = "";
                        textBoxVeer.Text = "";
                        TextBoxCommand.Text = "";

                        //------------Пересчитываем время и запускаем таймер----------------------
                        StartTimer();
                        //------------------------------------------------------------------------
                           
                        isFirstFire = false;
                        isSecoundFire = true; //Теперь будет производиться второй выстрел
                    }

//############################ Если производится не первый выстрел #################################################################

                    else
                    {
                        //------------------------Ввод данных------------------------------------------------------
                        
                        //Проверяем корректность выбора типа подаваемой команды
                        int command_error_code = ControlCommandAfterFirstFire();
                        if (command_error_code != 0) //если была подана команда неверного типа
                        {
                            if (command_error_code == -1)
                                MessageBox.Show("Не выбран тип команды!");
                            return;
                        }

                        if (RadioButtonStopWrite.Checked && isFinish) //Если команда Стой. Записать
                        {
                            Finish();
                            return;
                        }

                        //Резервная копия isVeerEntered
                        isVeerEntered_copy = isVeerEntered;

                        //Проверяем на корректность и считываем введенные корректуры
                        if (ControlEnterUstanovkiAfterFirstFire() != 0) //если какая-то из корректур введена неверно
                            return;

                        
                        //----------------Ввод данных окончен-------------------------------------------------------------

                        //-------------------Производим резервное копирование---------------------------------------------
                        //Делаем резервную копию shootData (копируем shootData в shootData_copy)
                        CopyShootdata(shootData, ref shootData_copy, true);
                        //Делаем резервную копию correctures (копируем correctures в correctures_copy)
                        CopyCorrectures(correctures, ref correctures_copy);
                        //Делаем резервную копию данных по наблюдению прошлого выстрела
                        NablDistRazr_copy = NablDistRazr;
                        NablDovrazr_copy = new DelUgl();
                        NablFrontRazr_copy = new DelUgl();
                        NablDovrazr_copy.fromDelUgl(NablDovrazr);
                        NablFrontRazr_copy.fromDelUgl(NablFrontRazr);

                        //Делаем резервную копию предыдущей команды
                        PrevCommand_copy = PrevCommand;
                        //Делаем резервную копию OldNabl
                        OldNabl_copy = OldNabl;
                        //Делаем резервные копии логичеких переменных
                        CopyBool();
                        //-----------------------Резервное копирование окончено--------------------------------------------

                        //Считываем число установок угломера и скачок прицела и проверяем их на ошибочность 
                        SetAndCheckCountOfUglUstAndScachPricel();

                        //---------------------------Формируем команду--------------------------
                        //Назначаем тип команды
                        SetCommandType();
                        //Тип команды назначен
                        
                        //------------------Назначаем введенные корректуры----------------------------------------------------- 
                        //кладем новые установки, ВД и isFireNaPorajenie в shootData.UserCommand
                        SetUstanovkiToUserCommandInShootData(Pricel, Uroven, Dovorot, Veer, isFireNaPoroajenie, ref shootData);
                        //-----------------Введенные корректуры назначены------------------------------------------------------
  
                        if (shootData.commandType == CommandType.Osn1Snar)
                        {
                            MakeListOfShootsToSingleFire(1, 20); //Назначаем 1 выстрел с задержкой 20 секунд
                        }
                        if (shootData.commandType == CommandType.Osn3Snar)
                        {
                            MakeListOfShootsToSingleFire(3, 30);//Назначаем 3 выстрела по 30 секунд
                            isSecoundFire = false;
                        }
                        if (shootData.commandType == CommandType.Batr2Snar || shootData.commandType == CommandType.Batr4Snar)
                        {
                            //Формируем список выстрелов при стрельбе на поражение
                            MakeListOfShootsToFireNaPorajenie();
                        }
                        //----------------------Команда сформирована-------------------------------------------------

                        //Останавливаем таймер
                        StopTimer();

                        NablDistRazr = 0;
                        NablDovrazr = new DelUgl();
                        NablFrontRazr = new DelUgl();

                        StartDopTimer();
                        
                        //---------------------------Показываем выстрел или залп---------------------------------------------
                        formShowShooting = new FormShowShooting(ref shootData);
                        formShowShooting.ShowDialog(this);

                        StopDopTime(shootData.is_cancel);
                        if (shootData.is_cancel)
                        {
                            BackUp();
                            return;
                        }

                        if ((taskType == TaskType.SBC || taskType == TaskType.SMC) && (!isFireNaPoroajenie))
                        {
                            CopyShootdata(shootData, ref shootDataNP,false);
                            List<int> vd_list = new List<int>();
                            for (int i = 0; i < shootData.ResultShootsList.Count; i++ )
                                vd_list.Add(shootData.ResultShootsList[i].VD);
                            formShowShooting = new FormShowShooting(ref shootDataNP, true,vd_list);
                            formShowShooting.ShowDialog(this);
                        }
                        TexBoxPricel.Focus();
                        //--------------Выстрел или залп показан--------------------------------------------------------------

                        //-------------------------Считаем ошибки при определениии корректур-------------------------------------

                        MistakesCount += MistakesUstUglScachPricelCount;
                        MistakesCount += MistakesCount_current;
                        MistakeCorrectureNapravlenie = new DelUgl();
                        MistakeCorrectureVeer = new DelUgl();

                        MistakeCorrectureDistance = Math.Abs(shootData.otkl_dist_expl_trgt);
                        MistakeCorrectureNapravlenie.fromInt32(Math.Abs(shootData.OtklCenterRazrFromOp.ToInt32()));
                        //При пристрелке по НЗР считаем ошибки в корректурах, как отклонение от идеальных
                        if ((taskType == TaskType.NBC || taskType == TaskType.NMC) && 
                            ((!isFireNaPoroajenie) || is_first_defence_shoot))
                        {
                            MistakeCorrectureDistance =
                                (int)Math.Abs(shootData.UserCommand.Pricel - shootData.VerifyCommand.Pricel) * shootData.deltaX;
                            MistakeCorrectureNapravlenie.fromInt32((int)Math.Abs(shootData.UserCommand.Dovorot.ToInt32() -
                                                                                 shootData.VerifyCommand.Dovorot.ToInt32()));
                        }
                        MistakeCorrectureVeer.fromInt32((int)Math.Abs(shootData.UserCommand.Veer.ToInt32() -
                                                                             shootData.VerifyCommand.Veer.ToInt32()));
                        if (!isFireNaPoroajenie)
                        {
                            if (MistakeCorrectureDistance > 50)
                                MistakesCount++;
                            if (MistakeCorrectureNapravlenie.ToInt32() > 5)
                                MistakesCount++;
                        }
                        else
                        {
                            if (((MistakeCorrectureDistance > 25) && (baseData.DepthTarget < 100)) ||
                                ((MistakeCorrectureDistance > 50) && (baseData.DepthTarget >= 100)))
                                MistakesCount++;
                            if (MistakeCorrectureNapravlenie.ToInt32() > 3)
                                MistakesCount++;
                            if (MistakeCorrectureVeer.ToInt32() > 2)
                                MistakesCount++;
                        }

                        if (is_first_defence_shoot)
                        {
                            MistakeFireDistance = Math.Abs(shootData.otkl_dist_expl_trgt);
                            MistakeFireNapravlenie.fromInt32(Math.Abs(shootData.OtklCenterRazrFromOp.ToInt32()));
                        }
                        //-----------------Ошибки в корректурах посчитаны---------------------------------------------------

                        //---------Пересчитываем количество выстрелов и залпов-------------------------
                        Count_of_shoots += Count_of_shoots_current;
                        Count_of_shoots_for_pristrelka += Count_of_shoots_for_pristrelka_current;
                        Count_of_zalps += Count_of_zalps_current;
                        //--------Залпы и выстрелы пересчитаны-----------------------------------------

                        //--------------Считываем наблюдения по разрыву или залпу----------------------------------
                        
                        if (isFireNaPoroajenie)
                        {
                            NablDistRazr = shootData.DalnostExplDefence;
                            NablDovrazr.dov_fromInt32(shootData.DirectExplDefence.ToInt32() - DirectAngleFromKNPToTarget.ToInt32());
                            NablFrontRazr.fromDelUgl(shootData.FrontRazr);
                        }
                        else
                        {
                            NablDistRazr = 0;
                            NablDovrazr.fromInt32(0);
                            NablDovrazrNP.fromInt32(0);
                            foreach (ShootData.ResultShoot nabl in shootData.ResultShootsList)
                            {
                                NablDistRazr += nabl.DalnostExpl;
                                NablDovrazr.dov_fromInt32(NablDovrazr.ToInt32() +
                                     (nabl.DirectExpl.ToInt32() - baseData.DirectionAngleFromKnp.ToInt32()));
                            }
                            NablDistRazr = NablDistRazr / shootData.ResultShootsList.Count;
                            NablDovrazr.dov_fromInt32(NablDovrazr.ToInt32() / shootData.ResultShootsList.Count);
                            if (taskType == TaskType.SMC || taskType == TaskType.SBC)
                            {
                                foreach (ShootData.ResultShoot nabl in shootDataNP.ResultShootsList)
                                    NablDovrazrNP.dov_fromInt32(NablDovrazrNP.ToInt32() +
                                                (nabl.DirectExpl.ToInt32() - baseData.DirectionAngleFromNp.ToInt32()));
                                NablDovrazrNP.dov_fromInt32(NablDovrazrNP.ToInt32() / shootDataNP.ResultShootsList.Count);
                            }
                                
                        }
                        
                        //-------------------------------наблюдения по разрыву или залпу считаны--------------------------------

                        //-------------------------выводим в файл текст поданной команды------------------------------------
                        WriteFireCommandInFile(Pricel, Uroven, Dovorot, CorrectureDovorot, Veer, correctures, fileWriter);
                        //------------------------------------------------------------------------------------------------------

                        //Выводим доклад дальномерщика или разведчика и записываем его в файл
                        fileWriter.WriteLine("Наблюдения:");
                        OutputNabludenieAndWriteInFileAfterFirstFire();    

                        //-----------Выстрел или залп показан и выведен в файл и на экран------------------------------------
 
                        TexBoxPricel.Text = "";
                        textBoxDovorot.Text = "";
                        textBoxVeer.Text = "";
                        TextBoxCommand.Text = "";

//++++++++++++++++++++++++++Проверка на окончание стрельбы++++++++++++++++++++++++++++++++++++++++++++++++++++++
                        //Проверяем, поражена ли цель
                        if (CheckTargetKilled()) return; 

                        //Проверяем, не потрачины ли все выстрелы, положенные для пристрелки
                        CheckOverShootsForPristrelka();
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                        //------------пересчитываем время время и запускаем таймер----------------------
                        StartTimer();
                        //-------------------------------------------------------------------
                        
                    }
           
            }
        }

 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        //При закрытии главной формы. Работа с файлом протокола.
        private void FormTrening_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (WriteEndIfTreningInFile)
                WriteEndOfTreningInFile();
        }



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++Рабочие функции+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        //Запустить таймер
        public void StartTimer()
        {
            stopWatch.Start();
        }

        //Остановить таймер таймер
        public void StopTimer()
        {
            stopWatch.Stop();
        }

        //Начать отсчет времени до команды отставить
        public void StartDopTimer()
        {
            dopWatch.Start();
        }

        TimeSpan dopSeconds = new TimeSpan();
        //Остановить отсчет дополнительного времени. Если была команда Отставить, то доп. время прибавляется к общему времени.
        public void StopDopTime(bool is_censel)
        {
            dopWatch.Stop();
            if (is_censel)
                dopSeconds += dopWatch.Elapsed;
            dopWatch.Reset();
        }

        //Считываем число установок угломера и скачок прицела и проверяем их на ошибочность 
        public void SetAndCheckCountOfUglUstAndScachPricel()
        {
            MistakesUstUglScachPricelCount = 0;
            if (numericUpDownCountOfUglUst.Visible)
            {
                CountOfUstUglomer = (int)numericUpDownCountOfUglUst.Value;
                if (CountOfUstUglomer > 0)
                {
                    if ((CountOfUstUglomer == 1) && (baseData.solved_data.PS_Front >= 150))
                        MistakesUstUglScachPricelCount++;
                    if ((CountOfUstUglomer == 2) && (baseData.solved_data.PS_Front < 150))
                        MistakesUstUglScachPricelCount++;
                    LabelCountOfUglUst.Hide();
                    numericUpDownCountOfUglUst.Hide();
                    WriteUstUglomer = true;
                }
            }
            if (numericUpDownScachPricel.Visible)
            {
                WriteScachocPricel = (ScachocPricel != ((int)numericUpDownScachPricel.Value));
                ScachocPricel = (int)numericUpDownScachPricel.Value;
                if (isFireNaPoroajenie || is_first_defence_shoot)
                {
                    if (ScachocPricel != 0)
                        WriteScachocPricel = true;
                    if (ScachocPricel != baseData.solved_data.SkPricel)
                        MistakesUstUglScachPricelCount++;
                    //LabelScachPricel.Hide();
                    //numericUpDownScachPricel.Hide();
                }
            }
        }

        //Передаем прицел, уровень, доворот, веер, ВД и признак стрельбы на поражение из команд пользователя в объект shootData 
        public void SetUstanovkiToUserCommandInShootData(int Pricel, DelUgl Uroven, DelUgl Dovorot,
                                                         DelUgl Veer, bool isFireNaPoroajenie, ref ShootData shootData)
        {
            shootData.UserCommand.Pricel = Pricel;
            if (textBoxUroven.Visible)
                shootData.UserCommand.Uroven.fromDelUgl(Uroven);
            shootData.UserCommand.Dovorot.fromDelUgl(Dovorot);
            if (isVeerEntered)
                shootData.UserCommand.Veer.fromDelUgl(Veer);
            shootData.isFireNaPorajenie = isFireNaPoroajenie;
            if      (isFireNaPoroajenie)
                    shootData.VD = VD_defence;
            else if (isFirstFire)
                    shootData.VD = VD_firstFire;
            else if (taskType == TaskType.DBC || taskType == TaskType.DMC)
                    shootData.VD = VD_shootDalnomer;
            else if (taskType == TaskType.NBC || taskType == TaskType.NMC)
                    shootData.VD = VD_shootNZR;
            else if (taskType == TaskType.SBC || taskType == TaskType.SMC)
                    shootData.VD = VD_shootSN;
        }

        //Кладем в список выстрелов данные по одиночному выстрелу (если не ведется стрельба на поражение)
        public void MakeListOfShootsToSingleFire(int count_of_shoots, int time)
        {
            shootData.count_of_shoots = count_of_shoots;
            shootData.time = time;
            shootData.Shoots.Clear();
            ShootData.ShootCommand cmd = new ShootData.ShootCommand();
            cmd.Pricel = shootData.UserCommand.Pricel;
            cmd.Uroven.fromDelUgl(shootData.UserCommand.Uroven);
            cmd.Dovorot.fromDelUgl(shootData.UserCommand.Dovorot);
            cmd.Veer.fromDelUgl(shootData.UserCommand.Veer);
            shootData.Shoots.Add(cmd);
        }
        
        //Формируем список выстрелов при стрельбе на поражение
        public void MakeListOfShootsToFireNaPorajenie()
        {
            if (shootData.commandType == CommandType.Batr2Snar)
                shootData.count_of_shoots = 2;
            if (shootData.commandType == CommandType.Batr4Snar)
                shootData.count_of_shoots = 4;
            shootData.time = 20;
            shootData.Shoots.Clear();

            ShootData.ShootCommand cmd = new ShootData.ShootCommand();
            cmd.Pricel = shootData.UserCommand.Pricel;
            cmd.Uroven.fromDelUgl(shootData.UserCommand.Uroven);
            cmd.Dovorot.fromDelUgl(shootData.UserCommand.Dovorot);
            cmd.Veer.fromDelUgl(shootData.UserCommand.Veer);
            shootData.Shoots.Add(cmd);
            int Iveer = shootData.UserCommand.Veer.ToInt32();

            cmd = new ShootData.ShootCommand();
            cmd.Pricel = shootData.UserCommand.Pricel;
            cmd.Uroven.fromDelUgl(shootData.UserCommand.Uroven);
            cmd.Veer.fromDelUgl(shootData.UserCommand.Veer);
            cmd.Dovorot.fromInt32(shootData.UserCommand.Dovorot.ToInt32() + Iveer + 6000);
            shootData.Shoots.Add(cmd);
            cmd = new ShootData.ShootCommand();
            cmd.Pricel = shootData.UserCommand.Pricel;
            cmd.Uroven.fromDelUgl(shootData.UserCommand.Uroven);
            cmd.Veer.fromDelUgl(shootData.UserCommand.Veer);
            cmd.Dovorot.fromInt32(shootData.UserCommand.Dovorot.ToInt32() - Iveer + 6000);
            shootData.Shoots.Add(cmd);
            cmd = new ShootData.ShootCommand();
            cmd.Pricel = shootData.UserCommand.Pricel;
            cmd.Uroven.fromDelUgl(shootData.UserCommand.Uroven);
            cmd.Veer.fromDelUgl(shootData.UserCommand.Veer);
            cmd.Dovorot.fromInt32(shootData.UserCommand.Dovorot.ToInt32() + Iveer * 2 + 6000);
            shootData.Shoots.Add(cmd);
            cmd = new ShootData.ShootCommand();
            cmd.Pricel = shootData.UserCommand.Pricel;
            cmd.Uroven.fromDelUgl(shootData.UserCommand.Uroven);
            cmd.Veer.fromDelUgl(shootData.UserCommand.Veer);
            cmd.Dovorot.fromInt32(shootData.UserCommand.Dovorot.ToInt32() - Iveer * 2 + 6000);
            shootData.Shoots.Add(cmd);
            cmd = new ShootData.ShootCommand();
            cmd.Pricel = shootData.UserCommand.Pricel;
            cmd.Uroven.fromDelUgl(shootData.UserCommand.Uroven);
            cmd.Veer.fromDelUgl(shootData.UserCommand.Veer);
            cmd.Dovorot.fromInt32(shootData.UserCommand.Dovorot.ToInt32() - Iveer * 3 + 6000);
            shootData.Shoots.Add(cmd);

            //Если стреляем на 3 установках прицела
            if (numericUpDownScachPricel.Value > 0)
            {
                //Добавляем выстрелы на второй установке прицела (с прибавленым скачком)
                for (int i = 0; i < 6; i++)
                {
                    cmd = new ShootData.ShootCommand();
                    cmd.Pricel = shootData.UserCommand.Pricel + (int)numericUpDownScachPricel.Value;
                    cmd.Uroven.fromDelUgl(shootData.UserCommand.Uroven);
                    cmd.Veer.fromDelUgl(shootData.UserCommand.Veer);
                    cmd.Dovorot.fromInt32(shootData.Shoots[i].Dovorot.ToInt32());
                    shootData.Shoots.Add(cmd);
                }

                //Добавляем выстрелы на третьей установке прицела (с отнятым скачком)
                for (int i = 0; i < 6; i++)
                {
                    cmd = new ShootData.ShootCommand();
                    cmd.Pricel = shootData.UserCommand.Pricel - (int)numericUpDownScachPricel.Value;
                    cmd.Uroven.fromDelUgl(shootData.UserCommand.Uroven);
                    cmd.Veer.fromDelUgl(shootData.UserCommand.Veer);
                    cmd.Dovorot.fromInt32(shootData.Shoots[i].Dovorot.ToInt32());
                    shootData.Shoots.Add(cmd);
                }
            }

            //Если стреляем на 2 установках прицела, то дублируем все выстрелы с доворотом вправо
            if (numericUpDownCountOfUglUst.Value == 2)
            {
                int count = shootData.Shoots.Count;
                for (int i = 0; i < count; i++)
                {
                    cmd = new ShootData.ShootCommand();
                    cmd.Pricel = shootData.Shoots[i].Pricel;
                    cmd.Uroven.fromDelUgl(shootData.Shoots[i].Uroven);
                    cmd.Veer.fromDelUgl(shootData.Shoots[i].Veer);
                    cmd.Dovorot.fromInt32(shootData.Shoots[i].Dovorot.ToInt32() + Iveer / 2 + 6000);
                    shootData.Shoots.Add(cmd);
                }
            }

        }

        //Назначаем тип команды по выбранной пользователем команде. Используется при всех выстрелах, кроме первого.
        //Подсчитываются идеальные корректуры и ошибки в определении установок при переходе к стрельбе на поражение.
        //Подсчитывается расход снарядов.
        public void SetCommandType()
        {
            Count_of_shoots_current = 0;
            Count_of_shoots_for_pristrelka_current = 0;
            Count_of_zalps_current = 0;
            MistakesCount_current = 0;
            //---------------------Если назначен 1 выстрел---------------------------------
            if ((RadioButton1snar.Checked) ||
                ((RadioButtonFire.Checked) && (PrevCommand == CommandType.Osn1Snar)))
            {              
                shootData.commandType = CommandType.Osn1Snar;
                PrevCommand = CommandType.Osn1Snar;
                Count_of_shoots_current++;
                Count_of_shoots_for_pristrelka_current++;
                if ((taskType == TaskType.NBC || taskType == TaskType.NMC))
                    if (correctures.vilka <= 50)
                        MistakesCount_current++;
                if (taskType == TaskType.DBC || taskType == TaskType.DMC)
                    correctures.next_shotDalnomer(NablDovrazr, NablDistRazr);
                else
                {
                    if (taskType == TaskType.NBC || taskType == TaskType.NMC)
                    {
                        correctures.next_shotNZR(NablDovrazr, shootData.nabludenie, OldNabl, false);
                        OldNabl = shootData.nabludenie;
                    }
                    else if (taskType == TaskType.SBC || taskType == TaskType.SMC)
                        correctures.next_shotSN(NablDovrazrNP, NablDovrazr);
                }

                //----------------Определяем идеальные корректуры------------------------------------------------------
                CountVerifiCommand(ref shootData, ref correctures);//Кладем идеальные корректуры в shootData.VerifiCommand
                //-----------------Идеальные корректуры определены-------------------------------------------------------
            }
            else
                //--------------------Если назначены 3 снаряда 30 секунд------------------------
                if ((RadioButton3snar30sec.Checked) ||
                ((RadioButtonFire.Checked) && (PrevCommand == CommandType.Osn3Snar)))
                {
                    shootData.commandType = CommandType.Osn3Snar;
                    PrevCommand = CommandType.Osn3Snar;
                    Count_of_shoots_current += 3;
                    Count_of_shoots_for_pristrelka_current += 3;
                    isSecoundFire = false;
                    if (taskType == TaskType.DBC || taskType == TaskType.DMC)
                        correctures.next_shotDalnomer(NablDovrazr, NablDistRazr);
                    else
                        if (taskType == TaskType.SMC || taskType == TaskType.SBC)
                            correctures.next_shotSN(NablDovrazrNP, NablDovrazr);

                    //----------------Определяем идеальные корректуры------------------------------------------------------
                    CountVerifiCommand(ref shootData, ref correctures);//Кладем идеальные корректуры в shootData.VerifiCommand
                    //-----------------Идеальные корректуры определены-------------------------------------------------------
                }
                else
                    //--------------Если назначен залп-----------------------------------------------
                    if ((
                        (RadioButton2snarFast.Checked) ||
                        ((RadioButtonFire.Checked) && (PrevCommand == CommandType.Batr2Snar))
                        )
                        ||
                        (
                        (RadioButton4snarFast.Checked) ||
                        ((RadioButtonFire.Checked) && (PrevCommand == CommandType.Batr4Snar))
                        ))
                    {
                        int shoots = 0;
                        if ((RadioButton2snarFast.Checked) ||
                            ((RadioButtonFire.Checked) && (PrevCommand == CommandType.Batr2Snar)))
                        {
                            shootData.commandType = CommandType.Batr2Snar;
                            PrevCommand = CommandType.Batr2Snar;
                            shoots = 12;
                        }
                        if ((RadioButton4snarFast.Checked) ||
                            ((RadioButtonFire.Checked) && (PrevCommand == CommandType.Batr4Snar)))
                        {
                            shootData.commandType = CommandType.Batr4Snar;
                            PrevCommand = CommandType.Batr4Snar;
                            shoots = 24;
                        }
                        if (!isFireNaPoroajenie)
                            if (!isVeerEntered)
                                MistakesCount_current++;

                        if (ScachocPricel > 0) shoots = shoots * 3;
                        if (CountOfUstUglomer == 2) shoots = shoots * 2;
                        Count_of_shoots_current += shoots;
                        Count_of_zalps_current++;
                        
                        //------------Определяем отклонения точности огня при переходе к стрельбе на поражение-------
                        if (!isFireNaPoroajenie)  //-------------Если совершается переход к стрельбе на поражение------------------------
                        {
                            is_first_defence_shoot = true; //Это первый залп
                            
                            NablFrontRazr = new DelUgl();

                            if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                                correctures.next_shotDalnomer(new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32()
                                                                         - baseData.DirectionAngleFromKnp.ToInt32()),
                                                                         shootData.ResultShootsList[0].DalnostExpl);
                            else
                                if (taskType == TaskType.DBC || taskType == TaskType.DMC)
                                    correctures.next_shotDalnomer(NablDovrazr, NablDistRazr);
                                else if (taskType == TaskType.SMC || taskType == TaskType.SBC)
                                    correctures.next_shotSN(NablDovrazrNP, NablDovrazr);
                                else if (taskType == TaskType.NBC || taskType == TaskType.NMC)
                                {
                                    correctures.next_shotNZR(NablDovrazr, shootData.nabludenie, OldNabl, false);

                                    if ((baseData.solved_data.PS_Depth <= 100) && (correctures.vilka > 50))
                                        MistakesCount_current++;
                                    if ((baseData.solved_data.PS_Depth > 100) && (correctures.vilka > 100))
                                        MistakesCount_current++;
                                    OldNabl = shootData.nabludenie;
                                }

                            //----------------Определяем идеальные корректуры------------------------------------------------------
                            CountVerifiCommand(ref shootData, ref correctures);//Кладем идеальные корректуры в shootData.VerifiCommand
                            //-----------------Идеальные корректуры определены-------------------------------------------------------

                            //shootData.VerifyCommand.Pricel = shootData.UserCommand.Pricel + (int)correctures.ΔП;
                            
                           /* MistakeFireDistance = (int)Math.Abs(Pricel -
                                                                shootData.VerifyCommand.Pricel + MistakeUroven)
                                                                * shootData.deltaX;*/

                            //shootData.VerifyCommand.Dovorot.fromInt32(Dovorot.ToInt32() +
                            //                                          correctures.Δβ.ToInt32() + 6000);
                           /* MistaceFireNapravlenie.fromInt32((int)Math.Abs(Dovorot.ToInt32() -
                                                                          shootData.VerifyCommand.Dovorot.ToInt32()));*/

                            if (CountOfUstUglomer == 0)
                                MistakesCount_current++;

                        }
                        //-------------------------------------------------------------------------------------------
                        else
                        {//------------------------------Если это не первый залп---------------------------------------------------
                            int ΔД = correctures.getDefenceDeltaD(shootData.nabludenie, shootData.otkl_dist_expl_trgt);
                            correctures.next_shotDefence(shootData.OtklCenterRazrFromOp, ΔД, NablFrontRazr, shootData.nabludenie);
                            //----------------Определяем идеальные корректуры------------------------------------------------------
                            CountVerifiCommand(ref shootData, ref correctures);//Кладем идеальные корректуры в shootData.VerifiCommand
                            //-----------------Идеальные корректуры определены-------------------------------------------------------
                            is_first_defence_shoot = false; //Это не первый залп
                        }
                        isFireNaPoroajenie = true; //Залп был произведен. Идет стрельба на поражение
                        isSecoundFire = false;
                    }
        }

        //Вывод в файл исчисленных данных
        public void WriteIschislInFile(StreamWriter fileWriter, TBaseData baseData,
                                       long DistFromCommanderToTarget,
                                       DelUgl DirectAngleFromKNPToTarget, TaskType taskType)
        {
            //Выводим в файл входные данные
            fileWriter.WriteLine("========Подготовка исчисленных установок===========");
            fileWriter.WriteLine("Основное направление: " + baseData.AlphaOn.ToString(false));
            fileWriter.WriteLine("Дальность командира: " + DistFromCommanderToTarget.ToString());
            fileWriter.WriteLine("Дирекционный угол по цели: " + DirectAngleFromKNPToTarget.ToString(false));
            if (taskType == TaskType.SBC || taskType == TaskType.SMC)
            {
                fileWriter.WriteLine("Дальность до цели с НП: " + baseData.DistCommanderNp.ToString());
                fileWriter.WriteLine("Дирекционный угол по цели с НП: " + baseData.DirectionAngleFromNp.ToString(false));
            }
            fileWriter.WriteLine("Дальность цели топографическая: " + baseData.solved_data.Dct.ToString());
            fileWriter.WriteLine("Доворот цели топографический: " + baseData.solved_data.dov_ct.ToString());
            fileWriter.WriteLine("Дальность цели исчисленная: " + baseData.solved_data.Dci.ToString());
            fileWriter.WriteLine("Доворот цели исчисленный: " + baseData.solved_data.dov_ci.ToString());
            fileWriter.WriteLine("ПС: " + baseData.solved_data.PS.ToString(false));
            if (taskType == TaskType.SBC || taskType == TaskType.SMC)
            {
                fileWriter.WriteLine("Угол засечки: " + baseData.solved_data.Gamma.ToString(false));
                fileWriter.WriteLine("Кл: " + Math.Round(baseData.solved_data.Kl).ToString());
                fileWriter.WriteLine("Кп: " + Math.Round(baseData.solved_data.Kp).ToString());
            }
            fileWriter.WriteLine("Ky: " + baseData.solved_data.Ku.ToString());
            fileWriter.WriteLine("Шу: " + baseData.solved_data.Shu100.ToString(false));
            fileWriter.WriteLine("Дельта Х тыс.: " + baseData.solved_data.delta_x_mil.ToString());
            fileWriter.WriteLine("Фронт цели: " + baseData.FrontTarget.ToString(false));
            fileWriter.WriteLine("Фронт цели (м): " + baseData.solved_data.FrontTarget_m.ToString());
            fileWriter.WriteLine("Глубина цели: " + baseData.DepthTarget.ToString());
            fileWriter.WriteLine("Высота КНП: " + baseData.Knp.h.ToString());
            fileWriter.WriteLine("Высота ОП: " + baseData.Op.h.ToString());
            fileWriter.WriteLine("Высота цели: " + baseData.solved_data.Hc.ToString());
            if (taskType == TaskType.DMC || taskType == TaskType.NMC || taskType == TaskType.SMC)
                fileWriter.WriteLine("ОП слева");
            else
                fileWriter.WriteLine("ОП справа");
            fileWriter.WriteLine("======================================================");
            fileWriter.WriteLine();
        }

        //Вывод в файл команды на выстрел
        public void WriteFireCommandInFile(int Pricel, DelUgl Uroven, DelUgl Dovorot, DelUgl CorrectureDovorot,
                                           DelUgl Veer, Correctures correctures, StreamWriter fileWriter)
        {
            //-----------------------выводим в файл текст команды------------------------------------------------------
            if (!isFirstFire)
                commandText = "";
            else
            {
                commandText = TextBoxCommand.Text;
                fileWriter.WriteLine("Первый выстрел:");
            }

            for (int i = 0; i < GroupBoxCommand.Controls.Count; i++)
                if (GroupBoxCommand.Controls[i].GetType().FullName == "System.Windows.Forms.RadioButton")
                    if ((GroupBoxCommand.Controls[i] as RadioButton).Checked)
                    {
                        commandText += " " + (GroupBoxCommand.Controls[i] as RadioButton).Text;
                        break;
                    }
            fileWriter.WriteLine();
            fileWriter.WriteLine("Текст команды:");
            fileWriter.WriteLine(commandText);
            
            string currenttStr;
            //-----------------------------Выводим поданные и идеальные установки-----------------------------------
            //выводим прицел
            currenttStr = "Прицел " + Pricel.ToString() + "(" + shootData.VerifyCommand.Pricel.ToString() + ")" +
                            "    (Отклонение по дальности: " + shootData.otkl_dist_expl_trgt.ToString() +  ")";
            if (Pricel != shootData.VerifyCommand.Pricel)
                WriteErrorInFile(currenttStr,(!isFirstFire)&&(((!isFireNaPoroajenie) && (MistakeCorrectureDistance > 50)) || 
                                                              (( isFireNaPoroajenie) && 
                                                               (
                                                                ((MistakeCorrectureDistance > 25) && (baseData.DepthTarget < 100)) ||
                                                                ((MistakeCorrectureDistance > 50) && (baseData.DepthTarget >= 100))
                                                               )
                                                              )));
            else
                fileWriter.WriteLine(currenttStr);
            //выводим уровень
            if (isFirstFire)
            {
                currenttStr = "Уровень " + Uroven.ToString(false) + "(" + shootData.VerifyCommand.Uroven.ToString(false) + ")";
                if (Uroven.ToInt32() != shootData.VerifyCommand.Uroven.ToInt32())
                    WriteErrorInFile(currenttStr,false);
                else
                    fileWriter.WriteLine(currenttStr);
            }
            //-------------------выводим доворот---------------------------------------------
            if (isFirstFire)
            {   //Если выводим доворот по первому выстрелу
                //DelUgl dovorotFromON = new DelUgl(Dovorot.ToInt32() - baseData.AlphaOn.ToInt32());
                DelUgl dovorotFromON = new DelUgl(CorrectureDovorot);
                DelUgl verifyDovorotFromON = new DelUgl(shootData.VerifyCommand.Dovorot.ToInt32() - baseData.AlphaOn.ToInt32());
                if (Math.Abs(verifyDovorotFromON.ToInt32()) > 3000)
                {
                    if (verifyDovorotFromON.sign > 0)
                        verifyDovorotFromON.fromInt32(verifyDovorotFromON.ToInt32() - 6000);
                    else
                        verifyDovorotFromON.dov_fromInt32(verifyDovorotFromON.ToInt32() + 6000);
                }
                currenttStr = "ОН " + CorrectureDovorot.ToString() + "(OH " + verifyDovorotFromON.ToString() + ")" +
                                "    (Отклонение по направлению: " + shootData.OtklCenterRazrFromOp.ToString() + ")";
                if (CorrectureDovorot.ToInt32() != verifyDovorotFromON.ToInt32())
                    WriteErrorInFile(currenttStr,false);
                fileWriter.WriteLine(currenttStr);
            }
            else
            {   //Если выводим доворот не по первому выстрелу
                if (CorrectureDovorot.ToInt32() != 0) //Если доворот корректировали
                {
                    currenttStr = CorrectureDovorot.ToString() + "(" + correctures.Δβ.ToString() + ")" +
                                    "    (Отклонение по направлению: " + shootData.OtklCenterRazrFromOp.ToString() + ")";
                    if (CorrectureDovorot.ToInt32() != correctures.Δβ.ToInt32())
                        WriteErrorInFile(currenttStr, ((!isFireNaPoroajenie) && (MistakeCorrectureNapravlenie.ToInt32() > 5)) ||
                                                      ((isFireNaPoroajenie) && (MistakeCorrectureNapravlenie.ToInt32() > 3)));
                    else
                        fileWriter.WriteLine(currenttStr);
                }
                else
                {//Если доворот не корректировали
                    if (correctures.Δβ.ToInt32() != 0) //Если корректуру нужно было вводить
                        WriteErrorInFile("Не подана корректура доворота " + "(" + correctures.Δβ.ToString() + ")" +
                                         "    (Отклонение по направлению: " + shootData.OtklCenterRazrFromOp.ToString() + ")",
                                         ((!isFireNaPoroajenie) && (MistakeCorrectureNapravlenie.ToInt32() > 5)) ||
                                         ((isFireNaPoroajenie) && (MistakeCorrectureNapravlenie.ToInt32() > 3)));
                    else //Если доворот и не надо было корректировать
                        fileWriter.WriteLine("Доророт прежний (доворот прежний)" +
                                             "    (Отклонение по направлению: " + shootData.OtklCenterRazrFromOp.ToString() + ")");
                }
            }
            //--------------------доворот выведен-----------------------------------------------
            
            //-----------------------выводим веер----------------------------------------------
            if (isVeerEntered)
            {
                if (Veer.ToInt32() == 0)
                    currenttStr = "Веер сосредоточенный ";
                else
                    currenttStr = "Веер " + Veer.ToString(false) + " ";
                if (shootData.VerifyCommand.Veer.ToInt32() == 0)
                    currenttStr += "(веер сосредоточенный)";
                else
                    currenttStr += "(" + shootData.VerifyCommand.Veer.ToString(false) + ")";
                if (Veer.ToInt32() != shootData.VerifyCommand.Veer.ToInt32())
                    WriteErrorInFile(currenttStr,(isFireNaPoroajenie) && (MistakeCorrectureVeer.ToInt32() > 2));
                else
                    fileWriter.WriteLine(currenttStr);

                //Если веер корректировали при разнице фронта разрывов и цели менеее, чем в 1.5 раза
                if ((Otnoshenie_FrontRazr_To_FrontTarget != 0.0) &&
                    ((Otnoshenie_FrontRazr_To_FrontTarget > (1.0 / 1.5)) && (Otnoshenie_FrontRazr_To_FrontTarget < 1.5)) &&
                    (CorrectureVeer.ToInt32() != 0))
                {
                    WriteErrorInFile("Веер был скорректирован при разнице фронта разрывов и фронта цели менее, чем в 1.5 раза!!!",true);
                    MistakesCount++;
                }
            }
            else
                if (isFireNaPoroajenie)
                {
                    currenttStr = "Веер не подан!!! ";
                    if (shootData.VerifyCommand.Veer.ToInt32() == 0)
                        currenttStr +="(веер сосредоточенный)";
                    else
                        currenttStr += "(" + shootData.VerifyCommand.Veer.ToString(false) + ")";
                    WriteErrorInFile(currenttStr,true);
                }
            //----------------------веер выведен-------------------------------------------------

            //-------------------выводим число установок угломера и скачок прицела------------------------
            if (CountOfUstUglomer != 0 && WriteUstUglomer)
                fileWriter.WriteLine("Установок " + CountOfUstUglomer.ToString() +
                    "(" + baseData.solved_data.UstUgl.ToString() + ")" + ". ");
            if (WriteScachocPricel)
                fileWriter.WriteLine("Скачок прицела " + ScachocPricel.ToString() +
                    "(" + baseData.solved_data.SkPricel.ToString() + ")" + ". ");
            fileWriter.WriteLine();
            if (!numericUpDownCountOfUglUst.Visible)
            {
                if ((CountOfUstUglomer != baseData.solved_data.UstUgl) && (WriteUstUglomer))
                    WriteErrorInFile("Ошибка в определении числа установок угломера!",true);
                WriteUstUglomer = false;
            }
            if (isFireNaPoroajenie || is_first_defence_shoot)
            {
                if ((ScachocPricel != baseData.solved_data.SkPricel) && (WriteScachocPricel))
                {
                    if (ScachocPricel == 0 || baseData.solved_data.SkPricel == 0)
                        WriteErrorInFile("Ошибка в определении числа установок прицела!", true);
                    else
                        WriteErrorInFile("Ошибка в определении величины скачка прицела!", true);
                    WriteScachocPricel = false;
                }
            }
            if (isFireNaPoroajenie && CountOfUstUglomer == 0 && WriteUstUglomer)
            {
                WriteErrorInFile("Не подано количество установок угломера!",true);
                WriteUstUglomer = false;
            }
            //------------------число установок угломера и скачок прицела выведено----------------------------------------
            if (isFireNaPoroajenie)
            {
                numericUpDownCountOfUglUst.Hide();
                LabelCountOfUglUst.Hide();
            }
            fileWriter.WriteLine();
        }

        //Выводим на экран доклад дальномерщика или разведчика по первому выстрелу
        public void OutputNabludenieOfFirstFire()
        {
            if (taskType == TaskType.DMC || taskType == TaskType.DBC)
            {
                Log.Text += "\nДоклад дальномерщика\n==========================\n";
                if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                    Log.Text += "Попадание в цель. Дирекционный: " +
                                shootData.ResultShootsList[0].DirectExpl.ToString(false) +
                                ". Дальность: "
                                + shootData.ResultShootsList[0].DalnostExpl.ToString() + " .\n";
                else
                    if (shootData.nabludenie != ShootData.Nabludenie.NotDetected)
                        Log.Text += "Есть разрыв. Дальность: " + shootData.ResultShootsList[0].DalnostExpl.ToString() +
                                    ". Дирекционный: " + shootData.ResultShootsList[0].DirectExpl.ToString(false) + "\n";
                    else
                        Log.Text += "Не заметил!";
                Log.Select(Log.Text.Length - 1, Log.Text.Length - 1);
                Log.ScrollToCaret();

                if (formBaseData.frmDalnomer != null)
                {
                    if (shootData.nabludenie == ShootData.Nabludenie.NotDetected)
                    {
                        formBaseData.frmDalnomer.setNotDetected(
                            shootData.ResultShootsList[0].DirectExpl);
                    }
                    else if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                    {
                        formBaseData.frmDalnomer.setShootKillInPristrelka(
                            shootData.ResultShootsList[0].DirectExpl,
                            shootData.ResultShootsList[0].DalnostExpl);
                    }
                    else
                    {
                        formBaseData.frmDalnomer.setShoot1(shootData.ResultShootsList[0].DirectExpl,
                            shootData.ResultShootsList[0].DalnostExpl);
                    }
                }
            }
            else
            {
                Log.Text += "\nДоклад командира взвода управления:\n=============================\n";
                if (shootData.nabludenie != ShootData.Nabludenie.NotDetected)
                {
                    if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                        Log.Text += "Попадание в цель!\n";
                    else
                        Log.Text += "Есть разрыв!\n" +
                            ((taskType == TaskType.SMC || taskType == TaskType.SBC) &&
                             (shootDataNP.nabludenie == ShootData.Nabludenie.NotDetected) ?
                                 "\nДоклад с НП:\n==================\nНе заметил!" : "");
                }
                else
                    Log.Text += "Не заметил!";
                Log.Select(Log.Text.Length - 1, Log.Text.Length - 1);
                Log.ScrollToCaret();

                if (shootData.nabludenie == ShootData.Nabludenie.NotDetected)
                {
                    if ((taskType == TaskType.NBC || taskType == TaskType.NMC) &&
                        (formBaseData.frmNZR != null))
                    {
                         formBaseData.frmNZR.setNotDetected(
                                new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32()
                                           - DirectAngleFromKNPToTarget.ToInt32()));
                    }
                    if ((taskType == TaskType.SBC || taskType == TaskType.SMC) &&
                        (formBaseData.frmSN != null))
                    {
                         formBaseData.frmSN.setNotDetected(
                                new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32()
                                       - DirectAngleFromKNPToTarget.ToInt32()));
                    }
                }
                else if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                {
                    if ((taskType == TaskType.NBC || taskType == TaskType.NMC) && 
                        (formBaseData.frmNZR != null))
                    {
                        formBaseData.frmNZR.setShootKillInPristrelka(
                                new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32()
                                           - DirectAngleFromKNPToTarget.ToInt32()),
                                shootData.ResultShootsList[0].delta_x_razr);
                    }
                    if ((taskType == TaskType.SBC || taskType == TaskType.SMC) &&
                        (formBaseData.frmSN != null))
                    {
                        DelUgl r_min_ps = new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32()
                                       - DirectAngleFromKNPToTarget.ToInt32());
                        DelUgl l_min_ps = new DelUgl(shootDataNP.ResultShootsList[0].DirectExpl.ToInt32()
                                       - baseData.DirectionAngleFromNp.ToInt32());

                        formBaseData.frmSN.setShootKillInPristrelka(l_min_ps, r_min_ps);
                    }
                }
                else  //Если разрыв заметили, но в цель при пристрелке не попали
                {
                    if ((taskType == TaskType.NBC || taskType == TaskType.NMC) &&
                        (formBaseData.frmNZR != null))
                        formBaseData.frmNZR.setShoot(
                            new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32()
                            - DirectAngleFromKNPToTarget.ToInt32()), shootData.nabludenie, OldNabl);
                    if ((taskType == TaskType.SBC || taskType == TaskType.SMC) &&
                        (formBaseData.frmSN != null))
                    {
                        DelUgl r_min_ps = new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32()
                                       - DirectAngleFromKNPToTarget.ToInt32());
                        DelUgl l_min_ps = new DelUgl(shootDataNP.ResultShootsList[0].DirectExpl.ToInt32()
                                       - baseData.DirectionAngleFromNp.ToInt32());

                        formBaseData.frmSN.setShoot1(l_min_ps, r_min_ps);
                    }
                }
            }
        }
        
        //Выводим в файл наблюдения по первому выстрелу
        public void WriteNabludenieOfFirstFireInFile()
        {
            NablDistRazr = shootData.ResultShootsList[0].DalnostExpl;
            NablDovrazr = new DelUgl();
            NablDovrazrNP = new DelUgl();
            NablDovrazr.dov_fromInt32(shootData.ResultShootsList[0].DirectExpl.ToInt32() - baseData.DirectionAngleFromKnp.ToInt32());
            if (taskType == TaskType.SBC || taskType == TaskType.SMC)
                NablDovrazrNP.fromInt32(shootDataNP.ResultShootsList[0].DirectExpl.ToInt32() - baseData.DirectionAngleFromNp.ToInt32());
            fileWriter.WriteLine("Наблюдения:");
            if (taskType == TaskType.DBC || taskType == TaskType.DMC)
            {
                //Если наблюдали разрыв
                if (shootData.nabludenie != ShootData.Nabludenie.NotDetected)
                {
                    if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                        fileWriter.Write("Попадание в цель. Дирекционный: " +
                                          shootData.ResultShootsList[0].DirectExpl.ToString(false) +
                                          ". Дальность: "
                                           + shootData.ResultShootsList[0].DalnostExpl.ToString() + " .");
                    else
                        fileWriter.WriteLine("Есть разрыв. Дальность: " + NablDistRazr.ToString() +
                                            ". Дирекционный: " +
                                            shootData.ResultShootsList[0].DirectExpl.ToString(false));
                }
                else
                    fileWriter.WriteLine("Дирекционный: " + shootData.ResultShootsList[0].DirectExpl.ToString(false) + ". Не заметил");
            }
            else if (taskType == TaskType.NBC || taskType == TaskType.NMC)
            {
                //Если наблюдали разрыв
                if (shootData.nabludenie != ShootData.Nabludenie.NotDetected)
                {
                    if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                    {
                        char sgn = '+';
                        if (shootData.ResultShootsList[0].delta_x_razr < 0.0)
                            sgn = '-';
                        fileWriter.Write("Попадание в цель. " + NablDovrazr.ToStringNablOtklonenie() +
                            " Глазомерная оценка отклонения по дальности от центра цели: " +
                            +sgn + shootData.otkl_dist_expl_trgt.ToString() + " .");
                    }
                    else
                    {
                        fileWriter.Write("Есть разрыв. " + NablDovrazr.ToString(true));
                        if (baseData.DistCommander - NablDistRazr > 0)
                            fileWriter.Write(". Недолет.");
                        else
                            fileWriter.Write(". Перелет.");
                    }
                }
                else
                {
                    fileWriter.WriteLine(NablDovrazr.ToString(true) + ". Не заметил.");
                }
            }
            else if (taskType == TaskType.SMC || taskType == TaskType.SBC)
            {
                //Если наблюдали разрыв
                if (shootData.nabludenie != ShootData.Nabludenie.NotDetected)
                {
                    if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                    {
                        char sgn = '+';
                        if (shootData.ResultShootsList[0].delta_x_razr < 0.0)
                            sgn = '-';
                        fileWriter.Write("Попадание в цель. " + NablDovrazr.ToStringNablOtklonenie() +
                            " Глазомерная оценка отклонения по дальности от центра цели: " +
                            +sgn + shootData.ResultShootsList[0].delta_x_razr.ToString() + " .");
                    }
                    else
                    {
                        fileWriter.WriteLine("ЛЕВЫЙ (КНП)           ПРАВЫЙ (НП)");
                        fileWriter.WriteLine("Есть разрыв. " + NablDovrazr.ToStringNablOtklonenie() + "  |  " +
                                             "Есть разрыв. " + NablDovrazrNP.ToStringNablOtklonenie());
                        
                    }
                }
                else
                {
                    fileWriter.WriteLine(NablDovrazr.ToStringNablOtklonenie() + ". Не заметил.");
                }
            }

            fileWriter.WriteLine();
            fileWriter.WriteLine("--------------------------------------------------------------------------------------------");
        }

        //Вывод на экран и в файл докладов дальномерщика и разведчика по выстрелам и залпам, кроме первого выстрела
        public void OutputNabludenieAndWriteInFileAfterFirstFire()
        {
            if ((taskType == TaskType.DMC || taskType == TaskType.DBC) && (!isFireNaPoroajenie)) // пристрелка с дальномером
            {
                foreach (ShootData.ResultShoot nabl in shootData.ResultShootsList)
                {
                    Log.Text += "\nДоклад дальномерщика\n==========================\n";
                    //Если наблюдали разрыв
                    if (shootData.nabludenie != ShootData.Nabludenie.NotDetected)
                    {
                        if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                            Log.Text += "Попадание в цель. Дирекционный: " +
                                        nabl.DirectExpl.ToString(false) +
                                        ". Дальность: "
                                        + nabl.DalnostExpl.ToString() + " .\n";
                        else
                            Log.Text += "Есть разрыв. Дальность: " + nabl.DalnostExpl.ToString() +
                                        ". Дирекционный: " + nabl.DirectExpl.ToString(false) + "\n";
                    }
                    else
                        Log.Text += "Не заметил!";
                    Log.Select(Log.Text.Length - 1, Log.Text.Length - 1);
                    Log.ScrollToCaret();

                    //Если наблюдали разрыв
                    if (shootData.nabludenie != ShootData.Nabludenie.NotDetected)
                    {
                        if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                            fileWriter.Write("Попадание в цель. Дирекционный: " +
                                shootData.ResultShootsList[0].DirectExpl.ToString(false) +
                                ". Дальность: "
                                + shootData.ResultShootsList[0].DalnostExpl.ToString() + " .");
                        else
                            fileWriter.WriteLine("Есть разрыв. Дальность: " + nabl.DalnostExpl.ToString() +
                                    ". Дирекционный: " + nabl.DirectExpl.ToString(false));
                    }
                    else
                        fileWriter.WriteLine("Дирекционный: " + nabl.DirectExpl.ToString(false) + ". Не заметил!");
                }

                if (formBaseData.frmDalnomer != null)
                {
                    if (shootData.nabludenie == ShootData.Nabludenie.NotDetected)
                        formBaseData.frmDalnomer.setNotDetected(shootData.ResultShootsList[0].DirectExpl);
                    else if (shootData.ResultShootsList.Count == 1)
                        formBaseData.frmDalnomer.setShoot1(shootData.ResultShootsList[0].DirectExpl, shootData.ResultShootsList[0].DalnostExpl);
                    else
                        formBaseData.frmDalnomer.setShoot234(shootData.ResultShootsList[0].DirectExpl,
                                                         shootData.ResultShootsList[1].DirectExpl,
                                                         shootData.ResultShootsList[2].DirectExpl,
                                                         shootData.ResultShootsList[0].DalnostExpl,
                                                         shootData.ResultShootsList[1].DalnostExpl,
                                                         shootData.ResultShootsList[2].DalnostExpl);
                }
            }
            else 
            {
                if ((taskType == TaskType.NMC || taskType == TaskType.NBC) && (!isFireNaPoroajenie)) // пристрелка по НЗР
                {
                    Log.Text += "\nДоклад командира взвода управления:\n===========================\n";
                    if (shootData.nabludenie != ShootData.Nabludenie.NotDetected)
                    {
                        if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                            Log.Text += "Попадание в цель.\n";
                        else
                            Log.Text += "Есть разрыв!\n";
                    }
                    else
                        Log.Text += "Не заметил!";
                    Log.Select(Log.Text.Length - 1, Log.Text.Length - 1);
                    Log.ScrollToCaret();

                    //Если наблюдали разрыв
                    if (shootData.nabludenie != ShootData.Nabludenie.NotDetected)
                    {
                        if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                        {
                            char sgn = '+';
                            if (shootData.ResultShootsList[0].delta_x_razr < 0.0)
                                sgn = '-';
                            fileWriter.Write("Попадание в цель. " + NablDovrazr.ToStringNablOtklonenie() +
                                " Глазомерная оценка отклонения по дальности от центра цели: " +
                                +sgn + shootData.otkl_dist_expl_trgt.ToString() + " .");
                        }
                        else
                        {
                            fileWriter.Write("Есть разрыв. " + NablDovrazr.ToStringNablOtklonenie());
                            if (shootData.nabludenie == ShootData.Nabludenie.AllNedolet)
                                fileWriter.Write(". Недолет.");
                            else if (shootData.nabludenie == ShootData.Nabludenie.AllPerelet)
                                fileWriter.Write(". Перелет.");
                        }
                    }
                    else
                    {
                        fileWriter.WriteLine(NablDovrazr.ToStringNablOtklonenie() + ". Не заметил");
                    }
                    fileWriter.WriteLine();

                    if (formBaseData.frmNZR != null)
                    {
                        if (shootData.nabludenie == ShootData.Nabludenie.NotDetected)
                            formBaseData.frmNZR.setNotDetected(
                                new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32()
                                - DirectAngleFromKNPToTarget.ToInt32()));
                        else if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                            formBaseData.frmNZR.setShootKillInPristrelka(
                                new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32()
                                - DirectAngleFromKNPToTarget.ToInt32()), shootData.ResultShootsList[0].delta_x_razr);
                        else
                            formBaseData.frmNZR.setShoot(
                                new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32()
                                - DirectAngleFromKNPToTarget.ToInt32()), shootData.nabludenie, OldNabl);
                    }
                }

                if ((taskType == TaskType.SBC || taskType == TaskType.SMC) && (!isFireNaPoroajenie)) //Пристрелка СН
                {
                    for (int i = 0; i < shootData.ResultShootsList.Count; i++)
                    {
                        //Если наблюдали разрыв
                        if (shootData.nabludenie != ShootData.Nabludenie.NotDetected)
                        {
                            if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                                Log.Text += "\nДоклад с КНП\n==========================\nПопадание в цель!\n";
                            else
                                Log.Text += "\nДоклад с КНП\n==========================\nЕсть разрыв!\n" +
                                            "\nДоклад с НП\n==========================\n"+
                                            (shootDataNP.nabludenie != ShootData.Nabludenie.NotDetected ?
                                                    "Есть разрыв!\n":"Не заметил!\n");
                        }
                        else
                            Log.Text += "Не заметил!";
                        Log.Select(Log.Text.Length - 1, Log.Text.Length - 1);
                        Log.ScrollToCaret();

                        //Если наблюдали разрыв
                        if (shootData.nabludenie != ShootData.Nabludenie.NotDetected)
                        {
                            if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                            {
                                fileWriter.Write("Попадание в цель. Наблюдение с КНП: " + 
                                    (new DelUgl(shootData.ResultShootsList[i].DirectExpl.ToInt32() 
                                                 - DirectAngleFromKNPToTarget.ToInt32())).ToStringNablOtklonenie() +
                                    " Наблюдение с НП: " + 
                                    (new DelUgl(shootDataNP.ResultShootsList[i].DirectExpl.ToInt32()
                                        - baseData.DirectionAngleFromNp.ToInt32())).ToStringNablOtklonenie() + " .");
                            }
                            else
                            {
                                fileWriter.WriteLine("ЛЕВЫЙ (КНП)           ПРАВЫЙ (НП)");
                                fileWriter.WriteLine(
                                    "Есть разрыв. " +
                                    (new DelUgl(shootData.ResultShootsList[i].DirectExpl.ToInt32()
                                        - DirectAngleFromKNPToTarget.ToInt32())).ToStringNablOtklonenie()
                                    + "  |  " +
                                    "Есть разрыв. " +
                                    (new DelUgl(shootDataNP.ResultShootsList[i].DirectExpl.ToInt32()
                                        - baseData.DirectionAngleFromNp.ToInt32())).ToStringNablOtklonenie());

                            }
                        }
                        else
                            fileWriter.WriteLine("Дирекционный: " + shootData.ResultShootsList[i].DirectExpl.ToString(false) +
                                                 ". Не заметил!");
                    }
                    fileWriter.WriteLine();
                    if (formBaseData.frmSN != null)
                    {
                        if (shootData.nabludenie == ShootData.Nabludenie.NotDetected)
                            formBaseData.frmSN.setNotDetected(
                                new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32() -
                                           DirectAngleFromKNPToTarget.ToInt32()));
                        else if (shootData.ResultShootsList.Count == 1)
                        {
                            DelUgl r_min_ps = new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32()
                                                                 - DirectAngleFromKNPToTarget.ToInt32());
                            DelUgl l_min_ps = new DelUgl(shootDataNP.ResultShootsList[0].DirectExpl.ToInt32()
                                                                 - baseData.DirectionAngleFromNp.ToInt32());
                            formBaseData.frmSN.setShoot1(l_min_ps, r_min_ps);
                        }
                        else if (shootData.ResultShootsList.Count == 3)
                        {
                            DelUgl r1 = new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32()
                                                   - DirectAngleFromKNPToTarget.ToInt32());
                            DelUgl r2 = new DelUgl(shootData.ResultShootsList[1].DirectExpl.ToInt32()
                                                   - DirectAngleFromKNPToTarget.ToInt32());
                            DelUgl r3 = new DelUgl(shootData.ResultShootsList[2].DirectExpl.ToInt32()
                                                   - DirectAngleFromKNPToTarget.ToInt32());
                            DelUgl l1 = new DelUgl(shootDataNP.ResultShootsList[0].DirectExpl.ToInt32()
                                                   - baseData.DirectionAngleFromNp.ToInt32());
                            DelUgl l2 = new DelUgl(shootDataNP.ResultShootsList[1].DirectExpl.ToInt32()
                                                   - baseData.DirectionAngleFromNp.ToInt32());
                            DelUgl l3 = new DelUgl(shootDataNP.ResultShootsList[2].DirectExpl.ToInt32()
                                                   - baseData.DirectionAngleFromNp.ToInt32());

                            formBaseData.frmSN.setShoot234(l1, l2, l3, r1, r2, r3);
                        }
                    }
                }
            }
            if (isFireNaPoroajenie) // стрельба на поражение
            {
                Log.Text += "\nДоклад командира взвода управления\n====================================\n";
                Log.Text += "Есть залп!";
                if (shootData.nabludenie == ShootData.Nabludenie.KillTarget)
                    Log.Text += " Цель поражается!";
                Log.Text += "\n";
                Log.Select(Log.Text.Length - 1, Log.Text.Length - 1);
                Log.ScrollToCaret();
                fileWriter.WriteLine("Есть залп!");
                if (shootData.nabludenie != ShootData.Nabludenie.KillTarget)
                {
                    fileWriter.Write(NablDovrazr.ToStringNablOtklonenie() + "  ");
                    fileWriter.Write(shootData.NabludenieToString());
                }
                else
                    fileWriter.Write("Цель поражается.");
                fileWriter.WriteLine("  Фронт разрывов: " + NablFrontRazr.ToString(false));
                fileWriter.WriteLine();

                if (((taskType == TaskType.DMC) || (taskType == TaskType.DBC)) &&
                    (formBaseData.frmDalnomer != null))
                    formBaseData.frmDalnomer.setShootDefence(shootData.FrontRazr,
                                                            shootData.OtklCenterRazrFromOp,
                                                            shootData.nabludenie,
                                                            shootData.otkl_dist_expl_trgt);
                else if (((taskType == TaskType.NMC) || (taskType == TaskType.NBC)) &&
                         (formBaseData.frmNZR != null))
                    formBaseData.frmNZR.setShootDefence(shootData.FrontRazr,
                                                        shootData.OtklCenterRazrFromOp,
                                                        shootData.nabludenie,
                                                        shootData.otkl_dist_expl_trgt);
                else if ((taskType == TaskType.SBC || taskType == TaskType.SMC) &&
                         (formBaseData.frmSN != null))
                    formBaseData.frmSN.setShootDefence(shootData.FrontRazr,
                                                       shootData.OtklCenterRazrFromOp,
                                                       shootData.nabludenie,
                                                       shootData.otkl_dist_expl_trgt);

            }

            fileWriter.WriteLine("----------------------------------------------------------------------------------");
        }
        
        //Проверяем, поражена ли цель, считаем число удачных залпов.
        public bool CheckTargetKilled()
        {
            Otnoshenie_FrontRazr_To_FrontTarget =
                     (double)shootData.FrontRazr.ToInt32() /
                     (double)baseData.FrontTarget.ToInt32();
            if (isFireNaPoroajenie && shootData.nabludenie == ShootData.Nabludenie.KillTarget)
            {
                if (!(Otnoshenie_FrontRazr_To_FrontTarget < 0.5 || Otnoshenie_FrontRazr_To_FrontTarget > 2))
                    Count_of_good_zalps++;
            }
            if (Count_of_good_zalps >= 2)
            {
                Log.Text += "Три красные ракеты. Сигнал на прекращение стрельбы.";
                Log.Select(Log.Text.Length - 1, Log.Text.Length - 1);
                Log.ScrollToCaret();
                isFinish = true;
                //------------Отсекаем время и запускаем таймер----------------------
                StartTimer();
                //-------------------------------------------------------------------
                MessageBox.Show("Три красные ракеты. Сигнал на прекращение стрельбы.");
                return true;
            }
            return false;
        }

        //Проверяем, не потрачeны ли все выстрелы, положенные для пристрелки
        public void CheckOverShootsForPristrelka()
        {
            if (taskType == TaskType.DMC || taskType == TaskType.NMC || taskType == TaskType.SMC)
            {
                if (Count_of_shoots_for_pristrelka > 5)
                {
                    MessageBox.Show("Произведено больше 5 выстрелов на пристрелку!",
                        "Огневая задача не выполнена!");
                    this.Close();

                }
            }
            else
            {
                if (Count_of_shoots_for_pristrelka > 6)
                {
                    MessageBox.Show("Произведено больше 6 выстрелов на пристрелку!",
                        "Огневая задача не выполнена!");
                    this.Close();

                }
            }
        }

        //Вводятся поданные пользователем установки для первого выстрела. Возвращает 0, если установки введены правильно.
        public int ControlEnterUstanovkiForFirstFire()
        {
            if (TexBoxPricel.Text == "")
            {
                MessageBox.Show("Не подан прицел!");
                return 1;
            }
            if (!int.TryParse(TexBoxPricel.Text, out Pricel))
            {
                MessageBox.Show("Неверно введен прицел. Введите число!");
                return 2;
            }
            if (Pricel < 76)
            {
                MessageBox.Show("Вы подали слишком малый прицел!", "Вы допустили ошибку!");
                WriteErrorInFile("Назначен прицел меньше наименьшего!!! (" + Pricel.ToString() + ")",true);
                MistakesCount++;
                return 3;
            }

            if (textBoxUroven.Text == "")
            {
                MessageBox.Show("Не подан уровень!");
                return 4;
            }

            if (textBoxVeer.Text != "")
            {
                if ((textBoxVeer.Text[0] == '+' || textBoxVeer.Text[0] == '-'))
                {
                    MessageBox.Show("Неверно подан веер! " +
                                    "До того, как был подан веер его корректура не вводится!");
                    return 8;
                }
                if (textBoxVeer.Text.Length > 6)
                {
                    MessageBox.Show("Hеверно введен веер!");
                    return 9;
                }
                Veer.fromString(textBoxVeer.Text);
                //shootData.UserCommand.Veer = new DelUgl();
                //shootData.UserCommand.Veer.fromDelUgl(Veer);
                isVeerEntered = true;
            }

            Uroven.fromString(textBoxUroven.Text);
            shootData = new ShootData();
            //----Пересчет доворота от основного направления-----
            if (textBoxDovorot.Text == "")
                CorrectureDovorot.dov_fromInt32(0);
            else
                CorrectureDovorot.fromString(textBoxDovorot.Text);
            Dovorot.fromInt32(CorrectureDovorot.ToInt32() + baseData.AlphaOn.ToInt32() + 6000);

            //Если все установки введены корректно
            return 0;
        }
        
        //Проверка подаваемой команды после производства первого выстрела. Возвращает 0, если выбран верный тип команды
        public int ControlCommandAfterFirstFire()
        {
            if ((!RadioButtonStopWrite.Checked) && (!RadioButton4snarFast.Checked) && (!RadioButton2snarFast.Checked) &&
                (!RadioButton3snar30sec.Checked) && (!RadioButton1snar.Checked) && (!RadioButtonFire.Checked))
                return -1;

            CommandType current_command;

            getCurrentCommandType(out current_command);

            if ((shootData.nabludenie == ShootData.Nabludenie.KillTarget) && (!isFireNaPoroajenie) &&
                (current_command != CommandType.Batr2Snar) && (current_command != CommandType.Batr4Snar))
            {
                MessageBox.Show("Вы получили попадание в цель при пристрелке!\n"+
                                "Необходимо перейти к стрельбе на поражение!", "Вы допустили ошибку!");
                WriteErrorInFile("Попытка продолжить пристрелку после получения попадания в цель при пристрелке!!!",true);
                MistakesCount++;
                return 1;
            }
            
            if (shootData.nabludenie == ShootData.Nabludenie.NotDetected && (current_command != CommandType.Osn1Snar))
            {
                MessageBox.Show("Вы должны вывести разрыв на линию наблюдения!", "Вы допустили ошибку!");
                WriteErrorInFile("Попытка продолжать стрельбу, не выводя разрыв на линию наблюдения!!!",true);
                MistakesCount++;
                return 1;
            }

            if (shootData.nabludenie != ShootData.Nabludenie.KillTarget)
            if (isSecoundFire) //Если был произведен только первый выстрел
            {
                if (shootData.nabludenie != ShootData.Nabludenie.NotDetected)
                    if (((taskType == TaskType.DBC) || (taskType == TaskType.DMC) || 
                         (taskType == TaskType.SBC) || (taskType == TaskType.SMC)) &&
                                (current_command != CommandType.Osn3Snar))
                    {
                        MessageBox.Show("Вы неверно производите пристрелку с дальномером!",
                                        "Вы допустили ошибку");
                        WriteErrorInFile("Попытка неверного выполнения пристрелки с дальномером!!!",true);
                        MistakesCount++;
                        return 2;
                    }
            }

            if (
                    (!isSecoundFire && (!isFirstFire)) && 
                    (
                    ((taskType == TaskType.DBC) || (taskType == TaskType.DMC) || 
                     (taskType == TaskType.SMC) || (taskType == TaskType.SBC)) &&
                     (
                      (current_command != CommandType.Batr2Snar) && 
                      (!isFireNaPoroajenie) && 
                      (current_command != CommandType.Batr4Snar)
                     )
                    )
                )
            {
                MessageBox.Show("Вы неверно производите пристрелку с дальномером или пристрелку СН!\n" +
                                "Необходимо перейти к стрельбе на поражение!",
                                "Вы допустили ошибку");
                WriteErrorInFile("Попытка продолжить пристрелку с далномером или пристрелку СН" +
                                 " вместо перехода к стрельбе на поражение!!!",true);
                MistakesCount++;
                return 3;
            }
            if (!isFinish)
            {
                int new_vilka = correctures.get_new_vilka(shootData.nabludenie,OldNabl,false);
                if ((taskType == TaskType.NBC || taskType == TaskType.NMC) &&
                    ((new_vilka <= 50) && (baseData.solved_data.PS_Depth < 100) &&
                    (current_command != CommandType.Batr2Snar) && (current_command != CommandType.Batr4Snar)))
                {
                    MessageBox.Show("Вы неверно производите пристрелку по НЗР!\n" +
                                    "Необходимо перейти к стрельбе на поражение!",
                                    "Вы допустили ошибку");
                    WriteErrorInFile("Попытка продолжить пристрелку по НЗР вместо перехода к стрельбе на поражение!!!",true);
                    MistakesCount++;
                     return 4;
                }
                if ((taskType == TaskType.NBC || taskType == TaskType.NMC) &&
                    ((new_vilka <= 100) && (baseData.solved_data.PS_Depth >= 100) &&
                    (current_command != CommandType.Batr2Snar) && (current_command != CommandType.Batr4Snar)))
                {
                    MessageBox.Show("Вы неверно производите пристрелку по НЗР!\n" +
                                    "Необходимо перейти к стрельбе на поражение!",
                                    "Вы допустили ошибку");
                    WriteErrorInFile("Попытка продолжить пристрелку по НЗР вместо перехода к стрельбе на поражение!!!",true);
                    MistakesCount++;
                     return 5;
                }
                if (shootData.nabludenie != ShootData.Nabludenie.KillTarget && (!isFireNaPoroajenie))
                {
                    if ((taskType == TaskType.NBC || taskType == TaskType.NMC) &&
                        ((new_vilka > 50) && (baseData.solved_data.PS_Depth < 100) &&
                        (current_command != CommandType.Osn1Snar)))
                    {
                        MessageBox.Show("Вы неверно производите пристрелку по НЗР!\n",
                        "Вы допустили ошибку");
                        WriteErrorInFile("Попытка неверно продолжить пристрелку по НЗР " +
                                          "или перейти к стрельбе на поражение до выполнения соответствующих условий!!!",true);
                        MistakesCount++;
                        return 6;
                    }

                    if ((taskType == TaskType.NBC || taskType == TaskType.NMC) &&
                        ((new_vilka > 100) && (baseData.solved_data.PS_Depth > 100) &&
                        (current_command != CommandType.Osn1Snar)))
                    {
                        MessageBox.Show("Вы неверно производите пристрелку по НЗР!\n",
                        "Вы допустили ошибку");
                        WriteErrorInFile("Попытка неверно продолжить пристрелку по НЗР " +
                                          "или перейти к стрельбе на поражение до выполнения соответствующих условий!!!",true);
                        MistakesCount++;
                        return 7;
                    }
                }
            }
            if (taskType == TaskType.NBC || taskType == TaskType.NMC)
            {
                if (current_command == CommandType.Osn3Snar)
                {
                    MessageBox.Show("Вы неверно производите пристрелку по НЗР!",
                        "Вы допустили ошибку");
                    WriteErrorInFile("Попытка назначить 3 снаряда 30 секунд - выстрел при пристрелке по НЗР!!!",true);
                    MistakesCount++;
                    return 8;
                }
            }

            if ((isFireNaPoroajenie) &&
                ((current_command == CommandType.Osn1Snar) || (current_command == CommandType.Osn3Snar)))
            {
                MessageBox.Show("Вы неверно производите стрельбу на поражение!", "Вы допустили ошибку");
                WriteErrorInFile("Попытка совершать пристрелку после перехода к стрельбе на поражение!!!",true);
                MistakesCount++;
                return 9;
            }
            if ((!isFireNaPoroajenie) && (current_command == CommandType.StopWrite))
            {
                MessageBox.Show("Вы еще не поразили цель!", "Вы допустили ошибку");
                WriteErrorInFile("Попытка подать команду \"Стой! Записать!\" до перехода к стрельбе на поражение!!!",true);
                MistakesCount++;
                return 10;
            }

            if (isFinish && (current_command != CommandType.StopWrite))
            {
                MessageBox.Show("Вы обязаны прекратить стрельбу!", "Вы допустили ошибку!");
                WriteErrorInFile("Попытка продолжать стрельбу после того, как цель поражена!!!",true);
                MistakesCount++;
                return 11;
            }
            if (!isFinish && (current_command == CommandType.StopWrite))
            {
                MessageBox.Show("Вы еще не поразили цель!", "Вы допустили ошибку");
                WriteErrorInFile("Попытка подать команду \"Стой! Записать!\" до того, как цель поражена!!!",true);
                MistakesCount++;
                return 12;
            }

            return 0;
        }

        //Парсинг Радиобаттонов для типов команды
        public void getCurrentCommandType(out CommandType current_command)
        {
            if (RadioButtonFire.Checked)
                current_command = PrevCommand;
            else if (RadioButton1snar.Checked)
                current_command = CommandType.Osn1Snar;
            else if (RadioButton3snar30sec.Checked)
                current_command = CommandType.Osn3Snar;
            else if (RadioButton2snarFast.Checked)
                current_command = CommandType.Batr2Snar;
            else if (RadioButton4snarFast.Checked)
                current_command = CommandType.Batr4Snar;
            else if (RadioButtonStopWrite.Checked)
                current_command = CommandType.StopWrite;
            else
            current_command = PrevCommand;
        }

        //Вводятся поданные пользователем после первого выстрела корректуры. Возвращает 0, если корректуры введены верно.
        public int ControlEnterUstanovkiAfterFirstFire()
        {
            //-----------------------Контроль ввода--------------------------------------------
            CorrecturePricel = 0; //Обнуляем текущую корректуру прицела
            if (TexBoxPricel.Text != "")
            {
                if (!(TexBoxPricel.Text[0] == '+' || TexBoxPricel.Text[0] == '-'))
                {//Если вводится новый прицел, а не корректура 
                    int newPricel;
                    if (!int.TryParse(TexBoxPricel.Text, out newPricel))
                    {
                        MessageBox.Show("Неверно введен прицел. Введите число!");
                        return 1;
                    }
                    CorrecturePricel = newPricel - Pricel;
                    Pricel = newPricel;
                }
                else //Если вводится корректура прицела
                {
                    if (TexBoxPricel.Text[0] == '+')
                    {
                        string str = TexBoxPricel.Text.Substring(1);
                        CorrecturePricel = int.Parse(str);
                        Pricel += CorrecturePricel;
                    }
                    if (TexBoxPricel.Text[0] == '-')
                    {
                        string str = TexBoxPricel.Text.Substring(1);
                        CorrecturePricel = int.Parse(str);
                        Pricel -= CorrecturePricel;
                    }
                }
            }
            if (Pricel < 76)
            {
                MessageBox.Show("Вы подали слишком малый прицел!", "Вы допустили ошибку!");
                WriteErrorInFile("Назначен прицел меньше наименьшего!!! (" + Pricel.ToString() +")",true);
                MistakesCount++;
                return 2;
            }

            //--------------------------Вводим доворот и запоминаем введенную корректуру-----------------------
            CorrectureDovorot.fromInt32(0); //Обнуляем старое значение введенной корректуры
            if (textBoxDovorot.Text != "")
                if (!(textBoxDovorot.Text[0] == '+' || textBoxDovorot.Text[0] == '-'))
                {//Если подается новый доворот, а не корректура
                    DelUgl newDovorot = new DelUgl(textBoxDovorot.Text); //Новое значение доворота (в форме дирекционного угла)
                    //В Dovorot пока что еще лежит старое значение доворота (в форме дирекционного угла)
                    //В CorrectureDovorot кладем разность нового и старого доворотов
                    CorrectureDovorot.fromInt32(newDovorot.ToInt32() - Dovorot.ToInt32());
                    Dovorot.fromDelUgl(newDovorot); //В Dovorot кладем новое значене доворота
                }
                else //Если вводится корректура в направлении
                {
                    CorrectureDovorot.fromString(textBoxDovorot.Text);
                    //-----------Пересчет доворота с прибавленной корректурой----------
                    Dovorot.fromInt32(Dovorot.ToInt32() + CorrectureDovorot.ToInt32() + 6000);
                }
            //---------Введен доворот---------------------------------------------

            //----------------Если веер подан, вводим его в команду---------------------------------
            CorrectureVeer.fromInt32(0); //Обнуляем текущую корректуру веера
            if (textBoxVeer.Text != "")
            {
                //Подача первой установки веерa
                if (!isVeerEntered)
                {
                    if ((textBoxVeer.Text[0] == '+' || textBoxVeer.Text[0] == '-'))
                    {
                        MessageBox.Show("Неверно подан веер!" +
                                        "До того, как был подан веер его корректура не вводится!");
                        return 4;
                    }
                    Veer.fromString(textBoxVeer.Text);
                    isVeerEntered = true;
                }
                //Ввод корректуры поданного ранее веера
                else
                {
                    //Если вводится новое значение веера, а не корректура
                    if ((textBoxVeer.Text[0] != '+') && (textBoxVeer.Text[0] != '-'))
                    {
                        DelUgl newVeer = new DelUgl(textBoxVeer.Text);
                        if (isVeerEntered)
                            CorrectureVeer = new DelUgl(newVeer.ToInt32() - Veer.ToInt32());
                        //Если ПС > 5-00
                        if (isVeerEntered && (CorrectureVeer.ToInt32() != 0) &&
                            (taskType == TaskType.DBC || taskType == TaskType.NBC || taskType == TaskType.SBC))
                        {
                            MessageBox.Show("При ПС > 5-00 веер не корректируется!",
                                            "Вы допустили ошибку!");
                            WriteErrorInFile("Попытка скорректировать веер при ПС > 5-00!!!",true);
                            MistakesCount++;
                            return 6;    
                        }
                        if (isVeerEntered && ((int)Math.Abs(CorrectureVeer.ToInt32()) == 1))
                        {
                            MessageBox.Show("Корректрура веераа 0-01 не вводится!",
                                            "Вы допустили ошибку!");
                            WriteErrorInFile("Попытка ввести корректуру веера 0-01!!!",true);
                            MistakesCount++;
                            return 6;
                        }
                        Veer.fromDelUgl(newVeer);
                        isVeerEntered = true;
                    }
                    else //Если вводится корректура веера
                    {
                        //Если ПС > 5-00
                        if (taskType == TaskType.DBC || taskType == TaskType.NBC || taskType == TaskType.SBC)
                        {
                            MessageBox.Show("При ПС > 5-00 веер не корректируется!",
                                "Вы допустили ошибку!");
                            WriteErrorInFile("Попытка скорректировать веер при ПС > 5-00!!!",true);
                            MistakesCount++;
                            return 6;
                        }
                        CorrectureVeer = new DelUgl(textBoxVeer.Text);
                        if ((int)Math.Abs(CorrectureVeer.ToInt32()) == 1)
                        {
                            MessageBox.Show("Корректрура веераа 0-01 не вводится!",
                                            "Вы допустили ошибку!");
                            WriteErrorInFile("Попытка ввести корректуру веера 0-01!!!",true);
                            MistakesCount++;
                            return 6;
                        }
                        //-----------Пересчет веера с прибавленной корректурой----------
                        Veer.fromInt32(Veer.ToInt32() + CorrectureVeer.ToInt32());
                        if (Veer.sign < 0) Veer.fromInt32(0);

                        isVeerEntered = true;
                    }
                }
            }
            //----------------Веер введен--------------------------------------------
            
            //Все корректуры введены
            return 0;
        }

        //Вычислить идеальные корректуры стрельбы и поместить их в shootData.VerifyCommand
        public void CountVerifiCommand(ref ShootData shootData, ref Correctures correctures)
        {
            if (shootData.nabludenie == ShootData.Nabludenie.NotDetected)
                correctures.output_to_line_visible(new DelUgl(shootData.ResultShootsList[0].DirectExpl.ToInt32()
                                                                            -baseData.DirectionAngleFromKnp.ToInt32()));

            shootData.VerifyCommand.Pricel = shootData.UserCommand.Pricel + (int)correctures.ΔП;
            shootData.VerifyCommand.Dovorot.fromInt32(shootData.UserCommand.Dovorot.ToInt32() + correctures.Δβ.ToInt32() + 6000);
            if ((!isFireNaPoroajenie)||is_first_defence_shoot)
                shootData.VerifyCommand.Veer.fromInt32(baseData.solved_data.ShotVeer.ToInt32());
            else
                shootData.VerifyCommand.Veer.fromInt32(shootData.UserCommand.Veer.ToInt32() + correctures.ΔI_veer_razr.ToInt32());
            shootData.deltaX = (int)baseData.solved_data.delta_x_mil;
        }

        //Окончание выполнения огневой задачи. Открытие интерфейса ввода расхода снарядов.
        public void Finish()
        {
            ButtonCommand.Visible = false;
            LabelRashodSnar.Show();
            TextBoxRashodSnar.Show();
            ButtonEnterRashodSnar.Show();
            TextBoxRashodSnar.Focus();
        }
        
        //Выполнение команды Стой! Записать!
        public void StopAndWrite(int Rashod)
        {

            MessageBox.Show(CNPName + ", стой! Записать! Цель " + baseData.NumTarget.ToString() + ", " +
                                baseData.character + ". Расход: " + Rashod.ToString() + " (" + Count_of_shoots.ToString() + ")");

            //Останавливаем таймер
            StopTimer();

            double otnosh_rashod = ((double)Math.Abs(Rashod - Count_of_shoots)) / ((double)Count_of_shoots);

            TimeSpan tmp = new TimeSpan(dopSeconds.Ticks);
            tmp += stopWatch.Elapsed;
            int finalSecondsTotal = (int)(tmp.TotalSeconds);
            int finalMinutes = finalSecondsTotal / 60;
            int finalSeconds = finalSecondsTotal % 60;
            string minutes_string = finalMinutes.ToString();
            string seconds_string = finalSeconds.ToString();
            if (seconds_string.Length < 2)
                seconds_string = '0' + seconds_string;

            fileWriter.WriteLine();

            fileWriter.WriteLine(CNPName + ", стой! Записать! Цель " + baseData.NumTarget.ToString() + ", " +
                baseData.character + ". Расход: " + Rashod.ToString() + "(" + Count_of_shoots.ToString() + ")");

            if (otnosh_rashod > 0.2)
            {
                MistakesCount++;
                WriteErrorInFile("Расход снарядов доложен неверно!", true);
            }
            else if (otnosh_rashod > 0.00001)
                WriteErrorInFile("Расход снарядов доложен неверно!", false);

            fileWriter.WriteLine();
            fileWriter.WriteLine("Оценка по первому условию: " + GetMarkByMistakes() +
                                 " (ошибок: " + MistakesCount.ToString() + ")");
            fileWriter.WriteLine("Оценка по второму условию: " + GetMarkByTime(finalSecondsTotal) +
                                 " (время: " + minutes_string + ":" + seconds_string + ")");
            fileWriter.WriteLine("Оценка по третьему условию: " + GetMarkByFire() +
                                " (ошибка по дальности: " + MistakeFireDistance.ToString() +
                                " ошибка по направлению: " + MistakeFireNapravlenie.ToString(false) + ")");
            fileWriter.WriteLine("Общая оценка:  " + GetFinalMark());
            fileWriter.WriteLine();
            fileWriter.WriteLine("-----------------------------------------------------------------------------------------");
            fileWriter.WriteLine();

            WriteEndOfTreningInFile();
            WriteEndIfTreningInFile = false;

            //Вывод финального сообщения
            FormResultShooting frmResultShooting = new FormResultShooting
                (
                    "Оценка по первому условию: " + GetMarkByMistakes() +
                    " (ошибок: " + MistakesCount.ToString() + ")" +
                    "\n\rОценка по второму условию: " + GetMarkByTime((int)(stopWatch.Elapsed.TotalSeconds)) +
                    " (время: " + minutes_string + ":" + seconds_string + ")" +
                    "\n\rОценка по третьему условию: " + GetMarkByFire() +
                    " (ошибка по дальности: " + MistakeFireDistance.ToString() +
                    " ошибка по направлению: " + MistakeFireNapravlenie.ToString(false) + ")" +
                    "\n\rОбщая оценка:  " + GetFinalMark() +
                    "\n\r\n\rТип решаемой задачи: " + TaskTypeToString(taskType) +
                    "\n\r\n\rВы можете просмотреть файл с отчетом: " + filename,
                    baseData,
                    is_controlnaja
                );
            frmResultShooting.Show();
        }

        //Вывести в файл сообщение о допущенной ошибке или неточности isError = true, если говорится об ошибке, а не о неточности
        public void WriteErrorInFile(string message, bool isError)
        {
            fileWriter.WriteLine();
            string str = "         #";
            for (int i = 0; i <= message.Length; i++)
                str += '#';
            fileWriter.WriteLine(str);
            if (isError)
            {
                fileWriter.Write("         #" + "ОШИБКА!!!");
                for (int i = 0; i < message.Length - 9; i++)
                    fileWriter.Write(" ");
                fileWriter.WriteLine("#");
            }
                fileWriter.WriteLine("         #" + message + '#');
            fileWriter.WriteLine(str);
        }

        //Вывести в файл наблюдения и доклады и сообщение об окончаниии тренировки
        public void WriteEndOfTreningInFile()
        {
            if (fileWriter != null)
            {
                fileWriter.WriteLine();
                fileWriter.WriteLine(" Наблюдения и доклады:");
                int j = 0;
                for (int i = 0; i < Log.Text.Length; i++)
                {
                    if (Log.Text[i] == '\n')
                    {
                        fileWriter.WriteLine(Log.Text.Substring(j, i - j));
                        j = i + 1;
                    }
                }
                fileWriter.WriteLine();
                if (!isFinish)
                {
                    if (is_controlnaja)
                        fileWriter.Write(" Контрольная стрельба прервана: ");
                    else
                        fileWriter.Write(" Тренеровка прервана: ");
                }
                else
                {
                    if (is_controlnaja)
                        fileWriter.Write(" Контрольная стрельба окончена: ");
                    else
                        fileWriter.Write(" Тренеровка окончена: ");
                }
                string minutes_str = DateTime.Now.Minute.ToString();
                string seconds_str = DateTime.Now.Second.ToString();
                if (minutes_str.Length < 2)
                    minutes_str = "0" + minutes_str;
                if (seconds_str.Length < 2)
                    seconds_str = "0" + seconds_str;
                fileWriter.WriteLine(DateTime.Now.Hour.ToString() + " : " + minutes_str + " : " + seconds_str);

                fileWriter.Close();
                System.Diagnostics.Process.Start("notepad.exe", filename);
            }
        }
        
        //Возвращает текстовое представление типа задачи
        public string TaskTypeToString(TaskType taskType)
        {
            switch (taskType)
            {
                case TaskType.DBC: return "Пристрелка с дальномером. ПС > 5-00";
                case TaskType.DMC: return "Пристрелка с дальномером. ПС < 5-00";
                case TaskType.NBC: return "Пристрелка по НЗР. ПС > 5-00";
                case TaskType.NMC: return "Пристрелка по НЗР. ПС < 5-00";
                case TaskType.SBC: return "Пристрелка СН. ПС > 5-00";
                case TaskType.SMC: return "Пристрелка СН. ПС < 5-00";
            }
            return "";
        }

        //Скопировать из shootData1 в shootData2 все параметры, если isFullCopy = true,
        //скопировать только значимые для СОВЕРШЕНИЯ (не для наблюдения) выстрела параметры, если isFullCopy = false
        public void CopyShootdata(ShootData shootData1, ref ShootData shootData2, bool isFullCopy)
        {
            shootData2.commandType = shootData1.commandType;
            shootData2.count_of_shoots = shootData1.count_of_shoots;
            shootData2.deltaX = shootData1.deltaX;
            shootData2.isFireNaPorajenie = shootData1.isFireNaPorajenie;
            shootData2.Shoots.Clear();
            foreach (ShootData.ShootCommand cmd in shootData1.Shoots)
            {
                shootData2.Shoots.Add(new ShootData.ShootCommand(cmd.Pricel,cmd.Uroven,cmd.Dovorot,cmd.Veer,cmd.time_from_start));
            }
            shootData2.ResultShootsList.Clear();
            foreach (ShootData.ResultShoot rzlt in shootData1.ResultShootsList)
            {
                shootData2.ResultShootsList.Add(new ShootData.ResultShoot(rzlt.DalnostExpl,rzlt.DirectExpl,rzlt.delta_x_razr,rzlt.VD));
            }
            shootData2.time = shootData1.time;
            shootData2.VD = shootData1.VD;
            shootData2.UserCommand.Dovorot = shootData1.UserCommand.Dovorot;
            shootData2.UserCommand.Pricel = shootData1.UserCommand.Pricel;
            shootData2.UserCommand.time_from_start = shootData1.UserCommand.time_from_start;
            shootData2.UserCommand.Uroven = shootData1.UserCommand.Uroven;
            shootData2.UserCommand.Veer = shootData1.UserCommand.Veer;
            shootData2.VerifyCommand.Dovorot = shootData1.VerifyCommand.Dovorot;
            shootData2.VerifyCommand.Pricel = shootData1.VerifyCommand.Pricel;
            shootData2.VerifyCommand.time_from_start = shootData1.VerifyCommand.time_from_start;
            shootData2.VerifyCommand.Uroven = shootData1.VerifyCommand.Uroven;
            shootData2.VerifyCommand.Veer = shootData1.VerifyCommand.Veer;
            if (!isFullCopy)
            {
                shootData2.DalnostExplDefence = 0;
                shootData2.DirectExplDefence = new DelUgl();
                shootData2.FrontRazr = new DelUgl();
            }
            else
            {
                shootData2.OtklCenterRazrFromOp.fromDelUgl(shootData1.OtklCenterRazrFromOp);
                //shootData2.DirAngleCenterRazrFromOp.fromDelUgl(shootData1.DirAngleCenterRazrFromOp);
                shootData2.otkl_dist_expl_trgt = shootData1.otkl_dist_expl_trgt;
                shootData2.DirectExplDefence.fromDelUgl(shootData1.DirectExplDefence);
                shootData2.DalnostExplDefence = shootData1.DalnostExplDefence;
                shootData2.FrontRazr.fromDelUgl(shootData1.FrontRazr);
                shootData2.nabludenie = shootData1.nabludenie;
            }
        }

        //Скопировать значения текущих полей correctures1 в correctures2
        public void CopyCorrectures(Correctures correcrures1, ref Correctures correctures2)
        {
            correctures2.dov.fromDelUgl(correcrures1.dov);
            correctures2.vilka = correcrures1.vilka;
            correctures2.ΔI_veer_razr.fromDelUgl(correcrures1.ΔI_veer_razr);
            correctures2.Δβ.fromDelUgl(correcrures1.Δβ);
            correctures2.ΔД = correcrures1.ΔД;
            correctures2.ΔП = correcrures1.ΔП;
            correctures2.П = correcrures1.П;
            correctures2.У.fromDelUgl(correcrures1.У);
        }

        public void CopyBool()
        {
            is_first_defence_shoot_copy = is_first_defence_shoot;
            isFinish_copy = isFinish;
            isFireNaPoroajenie_copy = isFireNaPoroajenie;
            isSecoundFire_copy = isSecoundFire;
        }
        
        //Восстановленеи текущих данных после отмены команды
        public void BackUp()
        {
            is_first_defence_shoot = is_first_defence_shoot_copy;
            isFinish = isFinish_copy;
            isFireNaPoroajenie = isFireNaPoroajenie_copy;
            isSecoundFire = isSecoundFire_copy;
            isVeerEntered = isVeerEntered_copy;
            PrevCommand = PrevCommand_copy;
            OldNabl = OldNabl_copy;
            Pricel -= CorrecturePricel;
            Dovorot.fromInt32(Dovorot.ToInt32() - CorrectureDovorot.ToInt32());
            Veer.fromInt32(Veer.ToInt32() - CorrectureVeer.ToInt32());
            CopyShootdata(shootData_copy, ref shootData, true);
            CopyCorrectures(correctures_copy, ref correctures);
            numericUpDownCountOfUglUst.Show();
            numericUpDownScachPricel.Show();
            if (!isFireNaPoroajenie || is_first_defence_shoot)
            {
                LabelCountOfUglUst.Show();
                LabelScachPricel.Show();
            }
            else
            {
                LabelCountOfUglUst.Hide();
                //LabelScachPricel.Hide();
            }
            NablDistRazr = NablDistRazr_copy;
            NablDovrazr.fromDelUgl(NablDovrazr_copy);
            NablFrontRazr.fromDelUgl(NablFrontRazr_copy);
        }

        private void textBoxVeer_Validating(object sender, CancelEventArgs e)
        {
            DelUgl.ValidateDelUglControl(textBoxVeer, e, toolStripStatusLabel1,
                "^[+-]{0,1}[\\d]{1,2}-[\\d]{2}$");
        }

        private void textBoxDovorot_Validating(object sender, CancelEventArgs e)
        {
            DelUgl.ValidateDelUglControl(textBoxDovorot, e, toolStripStatusLabel1,
                "^[+-]{0,1}[\\d]{1,2}-[\\d]{2}$");
        }

        private void textBoxUroven_Validating(object sender, CancelEventArgs e)
        {
            DelUgl.ValidateDelUglControl(textBoxUroven, e, toolStripStatusLabel1,
                "^[\\d]{1,2}-[\\d]{2}$");
        }

        //Обработка ввода расхода снарядов
        private void ButtonEnterRashodSnar_Click(object sender, EventArgs e)
        {
            if (TextBoxRashodSnar.Text == "")
            {
                MessageBox.Show("Вы не ввели расход снарядов!");
                return;
            }
            int Rashod;

            if (!int.TryParse(TextBoxRashodSnar.Text, out Rashod))
            {
                MessageBox.Show("Введено некорректное значение расхода снарядов!\nВведите целое число!");
                return;
            }

            if (Rashod <= 0)
            {
                MessageBox.Show("Расход снарядов может быть только положительным числом!");
                return;
            }

            ButtonEnterRashodSnar.Hide();
            TextBoxRashodSnar.ReadOnly = true;

            StopAndWrite(Rashod);            
        }
    }
}
