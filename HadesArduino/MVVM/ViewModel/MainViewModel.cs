using HadesArduino.Core;
using HadesArduino.MVVM.Model;
using HadesArduino.UserControl;
using HandyControl.Controls;
using HandyControl.Tools.Extension;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Markup;

namespace HadesArduino.MVVM.ViewModel
{
    public class MainViewModel : ObservableObject
    {
        public GlobalViewModel GlobalViewModel { get; } = GlobalViewModel.Instance;
        public ObservableCollection<string>? SerialPortCollection { get; set; }
        public ObservableCollection<UserModel>? UserRegisteredCollection { get; set; }

        private string? _selectedPort;
        
        public string? SelectedPort
        {
            get { return _selectedPort; }
            set { _selectedPort = value; OnPropertyChanged(); }
        }

        private Dialog? _registerDialog;

        public Dialog? RegisterDialog
        {
            get { return _registerDialog; }
            set { _registerDialog = value; OnPropertyChanged(); }
        }


        public RelayCommand? OpenPortCommand { get; set; }
        public RelayCommand? RefreshPortCommand { get; set; }
        public RelayCommand? OpenLogWindowCommand { get; set; }
        public RelayCommand? TurnLedCommand { get; set; }
        public RelayCommand? RegisterCommand { get; set; }
        SerialPort? serialPort { get; set; }
        UserService userService { get; set; }

        void ResetConfig()
        {
            GlobalViewModel.IsPortOpen = false;
            SerialPortCollection?.Clear();
        }

        public MainViewModel()
        {
            GlobalViewModel.IsPortOpen = false;
            serialPort = new SerialPort();
            userService = new UserService();

            SerialPortCollection = new ObservableCollection<string>();
            UserRegisteredCollection = userService.Read();

            PopulatePortCollection();

            GlobalViewModel.RoomTemperature = "0°C";
            GlobalViewModel.RoomHumidity = "0%";
            GlobalViewModel.RegisterState = false;
            GlobalViewModel.FullNameRegister = "";


            RegisterCommand = new RelayCommand(o =>
            {
                serialPort.Write("#REGI\n");
                GlobalViewModel.RegisterState = true;

                RegisterDialog = Dialog.Show<TextDialog>();
            });

            TurnLedCommand = new RelayCommand(o =>
            {
                string state = "";

                switch(o)
                {
                    case "LED1":
                        if (_led1State == true)
                            state = "ON";
                        else
                            state = "OF";
                        serialPort.Write(string.Format("#{0}{1}\n", o, state));

                        break;
                    case "LED2":
                        if (_led2State == true)
                            state = "ON";
                        else
                            state = "OF";
                        serialPort.Write(string.Format("#{0}{1}\n", o, state));
                        break;
                    case "LED3":
                        if (_led3State == true)
                            state = "ON";
                        else
                            state = "OF";
                        serialPort.Write(string.Format("#{0}{1}\n", o, state));
                        break;
                }
            });

            OpenPortCommand = new RelayCommand(o =>
            {
                if (GlobalViewModel.IsPortOpen == true)
                {
                    serialPort.Close();
                    GlobalViewModel.IsPortOpen = false;
                    return;
                }

                if (SelectedPort != null)
                {
                    serialPort.PortName = SelectedPort;
                    serialPort.BaudRate = 19200;
                    serialPort.Parity = Parity.None;
                    serialPort.DataBits = 8;
                    serialPort.StopBits = StopBits.One;
                    serialPort.DtrEnable = true;
                    serialPort.RtsEnable = true;
                    serialPort.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);
                    try
                    {
                        serialPort.Open();
                        GlobalViewModel.IsPortOpen = serialPort.IsOpen;

                        AddRegisteredToSystem();
                    }
                    catch (Exception err)
                    {
                        HandyControl.Controls.MessageBox.Show("An unexpected error has occured, see full details below\n" + err.ToString(), "Hades System");
                        GlobalViewModel.HadesLogs += "[HADES-ERROR] " + err.Message + "\n";
                    }
                }
            });

            RefreshPortCommand = new RelayCommand(o =>
            {
                try
                {
                    serialPort.Close();
                    GlobalViewModel.IsPortOpen = false;
                }
                catch (Exception err)
                {
                    HandyControl.Controls.MessageBox.Show("An unexpected error has occured, see full details below\n" + err.ToString(), "Hades System", MessageBoxButton.YesNo, MessageBoxImage.Question);
                    GlobalViewModel.HadesLogs += "[HADES-ERROR] " + err.Message + "\n";
                }
                PopulatePortCollection();
            });
        }

        void PopulatePortCollection()
        {
            SerialPortCollection?.Clear();

            foreach (var x in SerialPort.GetPortNames())
            {
                SerialPortCollection?.Add(x);
            }
        }

        async void AddRegisteredToSystem()
        {
            if (GlobalViewModel.IsPortOpen)
            {
                if (UserRegisteredCollection.Count > 0)
                {
                    foreach (UserModel user in UserRegisteredCollection)
                    {
                        await Task.Delay(2000);

                        // Get the RFID value from the UserModel
                        string rfidValue = user.RFID;

                        //new Task(async () =>
                        //{
                        //    await Task.Delay(2000);

                        //    Application.Current.Dispatcher.Invoke(() =>
                        //    {
                        // Write the RFID value to the serial port
                        serialPort.Write(string.Format("${0}\n", rfidValue));
                        //});
                        await Task.Delay(1000);
                        //await Task.Delay(1000);
                        //});
                    }
                }
            };
            
        }
        
        private void DataReceivedHandler(object sender, SerialDataReceivedEventArgs e)
        {
            SerialPort sp = (SerialPort)sender;
            String data = sp.ReadLine().ToString().Trim();

            try
            {
                String[] val = data.Split(',');

                GlobalViewModel.RoomTemperature = val[0].Split(".")[0] + "°C";
                GlobalViewModel.RoomHumidity = val[1].Split(".")[0] + "%";

                if (val.Length > 2 && GlobalViewModel.RegisterState && !string.IsNullOrEmpty(val[2]))
                {
                    Application.Current.Dispatcher.Invoke(() =>
                    {
                        GlobalViewModel.HadesLogs += "[HADES SUCCESS] RFID TAG: " + val[2] + " REGISTERED" + "\n";
                        GlobalViewModel.RegisterState = false;
                        RegisterDialog?.Close();

                        if (GlobalViewModel.FullNameRegister != "")
                        {                            
                            userService.Create(new UserModel
                            {
                                Fullname = GlobalViewModel.FullNameRegister,
                                RFID = val[2],
                                isActive = true,
                                dateCreated = DateTime.Now,
                            });
                            GlobalViewModel.CurrentRFIDRegistered = val[2];
                            GlobalViewModel.FullNameRegister = "";
                        }
                    });
                }


            }
            catch (Exception err)
            {
                //MessageBox.Show("An unexpected error has occured, see full details below\n" + err.ToString(), "Hades System", MessageBoxButton.YesNo, MessageBoxImage.Question);
                GlobalViewModel.HadesLogs += "[HADES-WARNING] " + err.Message + "\n" + err.ToString();
                //serialPort?.Close();
            }

            GlobalViewModel.HadesLogs += "[HADES LOGS] ~ " + data + "\n";
        }
        #region Led State
        private bool _led1State;

        public bool Led1State
        {
            get { return _led1State; }
            set { _led1State = value; OnPropertyChanged(); }
        }

        private bool _led2State;

        public bool Led2State
        {
            get { return _led2State; }
            set { _led2State = value; OnPropertyChanged(); }
        }

        private bool _led3State;

        public bool Led3State
        {
            get { return _led3State; }
            set { _led3State = value; OnPropertyChanged(); }
        }

        #endregion
    }
}