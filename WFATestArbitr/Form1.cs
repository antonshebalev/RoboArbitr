using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.IO.MemoryMappedFiles;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WFATestArbitr
{
    public partial class Form1 : Form
    {
        //BidOffer1
        // Создаст, или подключится к уже созданной памяти с таким именем
        public MemoryMappedFile MemoryTerminalBidOffer1;
        // Создает поток для чтения
        StreamReader SR_TerminalBidOffer1;
        // Создает поток для записи
        StreamWriter SW_TerminalBidOffer1;

        //BidOffer2
        // Создаст, или подключится к уже созданной памяти с таким именем
        public MemoryMappedFile MemoryTerminalBidOffer2;
        // Создает поток для чтения
        StreamReader SR_TerminalBidOffer2;
        // Создает поток для записи
        StreamWriter SW_TerminalBidOffer2;

        // Создаст, или подключится к уже созданной памяти с таким именем
        public MemoryMappedFile MemoryQUIKCommand;
        // Создает поток для чтения
        StreamReader SR_QUIKCommand;
        // Создает поток для записи
        StreamWriter SW_QUIKCommand;

        StreamReader fileSettings;

        public string status = "Begin";

        public int sleep = 1;

        public double tekBasisBySpread = 0;
        public double tekBasisSellSpread = 0;

        public List<StrBasis> basisSpreadArr = new List<StrBasis>();

        public struct StrBasis
        {
            public double tekBasisBySpreadArr;
            public double tekBasisSellSpreadArr;
        }

        public double tekBid1 = 0;
        public double tekOffer1 = 0;
        public Int64 tekBid1Q = 0;
        public Int64 tekOffer1Q = 0;

        public double tekBid2 = 0;
        public double tekOffer2 = 0;
        public Int64 tekBid2Q = 0;
        public Int64 tekOffer2Q = 0;

        public static DateTime beginTime;

        public Char separator = System.Globalization.CultureInfo.CurrentCulture.NumberFormat.CurrencyDecimalSeparator[0];

        public bool trigger = false;

        public double max = 0;
        public double min = 100000;
        public double srednee = 0;
        public double priceBuyBasis = 0;
        public double priceSellBasis = 0;

        public double summ = 0;
        public double basisBuyBuff = 0;
        public double basisSellBuff = 0;
        //Флаг работы приложения (аналогично QLua) 
        //Ключевое слово "volatile" гарантирует, что в переменной в любое время и при обращении из любого потока будет актуальное значение
        public volatile bool Run = true;

        //Функция отправки команды в QUIK ( вызов: SetQUIKCommandData("Ваша команда"); ), или очистки памяти, при вызове без параметров ( вызов: SetQUIKCommandData(); )
        private void SetQUIKCommandData(string Data = "")
        {
            //Если нужно отправить команду
            if (Data != "")
            {
                //Дополняет строку команды "нулевыми байтами" до нужной длины
                for (int i = Data.Length; i < 256; i++) Data += "\0";
            }
            else //Если нужно очистить память
            {
                //Заполняет строку для записи "нулевыми байтами"
                for (int i = 0; i < 256; i++) Data += "\0";
            }
            //Встает в начало
            SW_QUIKCommand.BaseStream.Seek(0, SeekOrigin.Begin);
            //Записывает строку
            SW_QUIKCommand.Write(Data);
            //Сохраняет изменения в памяти
            SW_QUIKCommand.Flush();
        }


        private void SpredCalc()
        {
            //размер спрэда при его покупкe продаем 1ый инструмент, покупаем 2ой инструмент

            if (tekBid1 > 0 && tekBid2 > 0 && tekOffer1 > 0 && tekOffer2 > 0)
            {
                tekBasisBySpread = tekBid1 - tekOffer2;
                //размер спрэда при его продаже покупаем 1ый инструмент, продаем 2ой инструмент
                tekBasisSellSpread = tekOffer1 - tekBid2;


                if (tekBasisSellSpread > 0 && tekBasisBySpread > 0)
                {
                    if (Convert.ToBoolean(checkBoxAuto.Checked))
                    {
                        if (max < tekBasisBySpread) max = tekBasisBySpread;
                        //    if (max < tekBasisSellSpread) max = tekBasisSellSpread;

                        //    if (min > tekBasisBySpread) min = tekBasisBySpread;
                        if (min > tekBasisSellSpread) min = tekBasisSellSpread;

                        try
                        {
                            srednee = min + (max - min) / 2;
                            priceBuyBasis = (max - srednee) / 2 + srednee;
                            priceSellBasis = min + (srednee - min) / 2;
                        }
                        catch
                        { };
                    }
                    else
                    {
                        max = Convert.ToDouble(this.textBoxMax.Text);
                        min = Convert.ToDouble(this.textBoxMin.Text);
                        srednee = min + (max - min) / 2;
                        priceBuyBasis = (max - srednee) / 2 + srednee;
                        priceSellBasis = min + (srednee - min) / 2;
                    }
                    //SetQUIKCommandData("message Hello 2");\
                    try
                    {
                        if (!trigger)
                        {
                            if (tekBasisBySpread >= priceBuyBasis && status != "BuyBasis Open" && status != "SellBasis Open")
                            {
                                //Покупаем базис
                                if (priceBuyBasis - priceSellBasis >= Convert.ToDouble(this.textBoxMinKol.Text))
                                {
                                    SetQUIKCommandData("BuyBasis Open");
                                    status = "BuyBasis Open";
                                    this.labelStatus.Text = status;
                                    trigger = true;
                                    basisBuyBuff = tekBasisBySpread;
                                }

                            }

                            if (tekBasisSellSpread <= priceSellBasis && status != "BuyBasis Open" && status != "SellBasis Open")
                            {
                                //Продаем базис
                                if (priceBuyBasis - priceSellBasis >= Convert.ToDouble(this.textBoxMinKol.Text))
                                {
                                    SetQUIKCommandData("SellBasis Open");
                                    status = "SellBasis Open";
                                    this.labelStatus.Text = status;
                                    trigger = true;
                                    basisSellBuff = tekBasisSellSpread;
                                }
                            }
                        }
                        else
                        {
                            if (status == "BuyBasis Open") {
                                this.labelSumm.Text = (summ + basisBuyBuff - tekBasisSellSpread).ToString();
                                this.labelNet.Text = (basisBuyBuff - tekBasisSellSpread).ToString();
                            }
                            if (status == "SellBasis Open") {
                                this.labelSumm.Text = (summ + tekBasisBySpread - basisSellBuff).ToString();
                                this.labelNet.Text = (tekBasisBySpread - basisSellBuff).ToString();
                            }
                            if (tekBasisSellSpread <= priceSellBasis && status == "BuyBasis Open")
                            {
                                //Покупаем базис
                                if (priceBuyBasis - priceSellBasis >= Convert.ToDouble(this.textBoxMinKol.Text))
                                {
                                    SetQUIKCommandData("BuyBasis Close");
                                    status = "BuyBasis Close";
                                    this.labelStatus.Text = status;
                                    trigger = false;
                                    summ += basisBuyBuff - tekBasisSellSpread;
                                }

                            }

                            if (tekBasisBySpread >= priceBuyBasis && status == "SellBasis Open")
                            {
                                //Продаем базис
                                if (priceBuyBasis - priceSellBasis >= Convert.ToDouble(this.textBoxMinKol.Text))
                                {
                                    SetQUIKCommandData("SellBasis Close");
                                    status = "SellBasis Close";
                                    this.labelStatus.Text = status;
                                    trigger = false;
                                    summ += tekBasisBySpread - basisSellBuff;
                                }
                            }
                        }

                    }

                    catch { };
                    //  this.listBoxBase.Items.Add(tekBasisBySpread.ToString() + ";" + tekBasisSellSpread.ToString());
                    this.textBoxBasisBuy.Text = tekBasisBySpread.ToString();
                    this.textBoxBasisSell.Text = tekBasisSellSpread.ToString();
                }
            }

        }

        // Делегат нужен для того, чтобы безопасно обратиться к TextBox из другого потока
        private delegate void TB1(string Msg);
        private void AppTextBidOffer1(string Msg)
        {
            string[] list = Msg.Split(';', '-');

            try
            {
                tekBid1Q = Convert.ToInt64(list[0]);
                tekBid1 = Convert.ToDouble(list[1].Replace(',', separator).Replace('.', separator));
                tekOffer1Q = Convert.ToInt64(list[2]);
                tekOffer1 = Convert.ToDouble(list[3].Replace(',', separator).Replace('.', separator));
            }
            catch { };

            SpredCalc();
            textBoxB1Q.Text = tekBid1Q.ToString();
            textBoxB1.Text = tekBid1.ToString();
            textBoxO1Q.Text = tekOffer1Q.ToString();
            textBoxO1.Text = tekOffer1.ToString();

            this.textBoxMax.Text = max.ToString();
            this.textBoxMin.Text = min.ToString();
            this.textBoxSred.Text = srednee.ToString();
            this.textBoxPBuy.Text = priceBuyBasis.ToString();
            this.textBoxPSell.Text = priceSellBasis.ToString();
            this.textBoxTekKol.Text = (priceBuyBasis - priceSellBasis).ToString();
        }

        private delegate void TB2(string Msg);
        private void AppTextBidOffer2(string Msg)
        {
            string[] list = Msg.Split(';', '-');
            try
            {
                tekBid2Q = Convert.ToInt64(list[0]);
                tekBid2 = Convert.ToDouble(list[1].Replace(',', separator).Replace('.', separator));
                tekOffer2Q = Convert.ToInt64(list[2]);
                tekOffer2 = Convert.ToDouble(list[3].Replace(',', separator).Replace('.', separator));
            }
            catch { }
            SpredCalc();
            textBoxB2Q.Text = tekBid2Q.ToString();
            textBoxB2.Text = tekBid2.ToString();
            textBoxO2Q.Text = tekOffer2Q.ToString();
            textBoxO2.Text = tekOffer2.ToString();
        }

        // BidOffer1
        private void SetTerminalBidOffer1Data(string Data = "")
        {
            SW_TerminalBidOffer1.BaseStream.Seek(0, SeekOrigin.Begin);
            for (int i = 0; i < 256; i++) SW_TerminalBidOffer1.Write("\0");
            if (Data != "")
            {
                SW_TerminalBidOffer1.BaseStream.Seek(0, SeekOrigin.Begin);
                SW_TerminalBidOffer1.Write(Data);
            }
            SW_TerminalBidOffer1.Flush();
        }

        private void GettingBidOffer1()
        {
            //Запускает функцию получения стакана в отдельном потоке, чтобы приложение откликалось на действия пользователя
            //Чтобы остановить выполнение данного потока, нужно переменной "Run" присвоить значение "false"
            //Сделать это можно, либо в функции по событию закрытия приложения, либо при нажатии на кнопке и т.д.
            new Thread(() =>
            {
                string Str = "";

                //Постоянный цикл в отдельном потоке
                while (Run)
                {
                    //Получает BidOffer1 из памяти
                    
                    SR_TerminalBidOffer1.BaseStream.Seek(0, SeekOrigin.Begin);
                    Str = SR_TerminalBidOffer1.ReadToEnd().Trim('\0', '\r', '\n');

                    //Если  BidOffer1 получен, стирает запись, подтверждая это
                    if (Str != "00" && Str != "" && Str != "0" && Str != "-1")
                    {
                        //Стирает запись, подтверждая что стакан получен и будет обработан
                        SetTerminalBidOffer1Data();

                        //Что-то делает с полученным стаканом
                        // ...
                        // Потокобезопасно выводит сообщение в текстовое поле
                        BeginInvoke(new TB1(AppTextBidOffer1), Str);
                       

                    }
                    //Чтоб процесс не "забивал" одно из ядер процессора на 100% нужна пауза в 1 миллисекунду
                    Thread.Sleep(sleep);
                }
            }).Start();
        }

        // BidOffer1
        private void SetTerminalBidOffer2Data(string Data = "")
        {
            SW_TerminalBidOffer2.BaseStream.Seek(0, SeekOrigin.Begin);
            for (int i = 0; i < 256; i++) SW_TerminalBidOffer2.Write("\0");
            if (Data != "")
            {
                SW_TerminalBidOffer2.BaseStream.Seek(0, SeekOrigin.Begin);
                SW_TerminalBidOffer2.Write(Data);
            }
            SW_TerminalBidOffer2.Flush();
        }

        private void GettingBidOffer2()
        {
            //Запускает функцию получения стакана в отдельном потоке, чтобы приложение откликалось на действия пользователя
            //Чтобы остановить выполнение данного потока, нужно переменной "Run" присвоить значение "false"
            //Сделать это можно, либо в функции по событию закрытия приложения, либо при нажатии на кнопке и т.д.
            new Thread(() =>
            {
                string Str = "";

                //Постоянный цикл в отдельном потоке
                while (Run)
                {
                    //Получает BidOffer2 из памяти

                    SR_TerminalBidOffer2.BaseStream.Seek(0, SeekOrigin.Begin);
                    Str = SR_TerminalBidOffer2.ReadToEnd().Trim('\0', '\r', '\n');

                    //Если  BidOffer1 получен, стирает запись, подтверждая это
                    if (Str != "00" && Str != "" && Str != "0" && Str != "-1")
                    {
                        //Стирает запись, подтверждая что стакан получен и будет обработан
                        SetTerminalBidOffer2Data();

                        //Что-то делает с полученным стаканом
                        // ...
                        // Потокобезопасно выводит сообщение в текстовое поле
                        BeginInvoke(new TB2(AppTextBidOffer2), Str);
                        

                    }
                    //Чтоб процесс не "забивал" одно из ядер процессора на 100% нужна пауза в 1 миллисекунду
                    Thread.Sleep(1);
                }
            }).Start();
        }

        public Form1()
        {
            InitializeComponent();
            //BidOffer1
            //выделяет именованную память под получение СТАКАНА от QUIK, создает потоки чтения/записи
            MemoryTerminalBidOffer1 = MemoryMappedFile.CreateOrOpen("MyMemoryBidOffer1", 256, MemoryMappedFileAccess.ReadWrite);
            SR_TerminalBidOffer1 = new StreamReader(MemoryTerminalBidOffer1.CreateViewStream(), System.Text.Encoding.Default);
            SW_TerminalBidOffer1 = new StreamWriter(MemoryTerminalBidOffer1.CreateViewStream(), System.Text.Encoding.Default);

            //BidOffer2
            //выделяет именованную память под получение СТАКАНА от QUIK, создает потоки чтения/записи
            MemoryTerminalBidOffer2 = MemoryMappedFile.CreateOrOpen("MyMemoryBidOffer2", 256, MemoryMappedFileAccess.ReadWrite);
            SR_TerminalBidOffer2 = new StreamReader(MemoryTerminalBidOffer2.CreateViewStream(), System.Text.Encoding.Default);
            SW_TerminalBidOffer2 = new StreamWriter(MemoryTerminalBidOffer2.CreateViewStream(), System.Text.Encoding.Default);

            //выделяет именованную память размером 256 байт для отправки КОМАНД в QUIK, создает потоки чтения/записи
            MemoryQUIKCommand = MemoryMappedFile.CreateOrOpen("QUIKCommand", 256, MemoryMappedFileAccess.ReadWrite);
            SR_QUIKCommand = new StreamReader(MemoryQUIKCommand.CreateViewStream(), System.Text.Encoding.Default);
            SW_QUIKCommand = new StreamWriter(MemoryQUIKCommand.CreateViewStream(), System.Text.Encoding.Default);
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            trigger = false;
            SetQUIKCommandData("message Hello");
            this.labelStatus.Text = "Begin";
            
            
            Run = true;

            max = Convert.ToDouble(this.textBoxMax.Text);
            min = Convert.ToDouble(this.textBoxMin.Text);
            srednee = min + (max - min) / 2;
            priceBuyBasis = (max - srednee) / 2 + srednee;
            priceSellBasis = min + (srednee - min) / 2;

            this.textBoxSred.Text = srednee.ToString();
            this.textBoxPBuy.Text = priceBuyBasis.ToString();
            this.textBoxPSell.Text = priceSellBasis.ToString();
            this.textBoxTekKol.Text = (priceBuyBasis - priceSellBasis).ToString();

            GettingBidOffer1();
            GettingBidOffer2();
        }

        private void buttonStop_Click(object sender, EventArgs e)
        {
            Run = false;
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            Run = false;
            List <string> settingsStr = new List<string>();
            settingsStr.Add(this.textBoxMax.Text);
            settingsStr.Add(this.textBoxSred.Text);
            settingsStr.Add(this.textBoxMin.Text);
            settingsStr.Add(this.textBoxMinKol.Text);
            File.WriteAllLines("settings.txt", settingsStr);
        }

        private void textBoxB1_TextChanged(object sender, EventArgs e)
        {

        }

        private void label8_Click(object sender, EventArgs e)
        {

        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            

        }

        private void buttonClosePos_Click(object sender, EventArgs e)
        {
            if (status == "SellBasis Open")
            {
                SetQUIKCommandData("BuyBasis Open");
                status = "Begin";
            }
            else if (status == "BuyBasis Open")
            {
                SetQUIKCommandData("SellBasis Open");
                status = "Begin";
            }
            this.labelStatus.Text = status;
        }

        private void buttonBuyBasis_Click(object sender, EventArgs e)
        {
           
                SetQUIKCommandData("BuyBasis Open");
            
        }

        private void buttonSellBasis_Click(object sender, EventArgs e)
        {
           
                SetQUIKCommandData("SellBasis Open");
        }

        private void buttonGetStatus_Click(object sender, EventArgs e)
        {
            this.labelStatus.Text = status;
        }

        private void textBoxTimeRasch_TextChanged(object sender, EventArgs e)
        {

        }

        private void label14_Click(object sender, EventArgs e)
        {

        }

        private void Form1_Shown(object sender, EventArgs e)
        {

            string[] settingsStr = File.ReadAllLines("settings.txt");
            this.textBoxMax.Text = settingsStr[0];
            this.textBoxSred.Text = settingsStr[1];
            this.textBoxMin.Text = settingsStr[2];
            this.textBoxMinKol.Text = settingsStr[3];

            max = Convert.ToDouble(this.textBoxMax.Text);
            min = Convert.ToDouble(this.textBoxMin.Text);
            srednee = min + (max - min) / 2;
            priceBuyBasis = (max - srednee) / 2 + srednee;
            priceSellBasis = min + (srednee - min) / 2;

            this.textBoxSred.Text = srednee.ToString();
            this.textBoxPBuy.Text = priceBuyBasis.ToString();
            this.textBoxPSell.Text = priceSellBasis.ToString();
            this.textBoxTekKol.Text = (priceBuyBasis - priceSellBasis).ToString();
            
        }

        private void label20_Click(object sender, EventArgs e)
        {

        }
    }
}
