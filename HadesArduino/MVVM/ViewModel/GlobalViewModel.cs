using HadesArduino.Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HadesArduino.MVVM.ViewModel
{
    public class GlobalViewModel : ObservableObject
    {
        public static GlobalViewModel Instance { get; } = new GlobalViewModel();

		private bool _isPortOpen;

		public bool IsPortOpen
		{
			get { return _isPortOpen; }
			set { _isPortOpen = value; OnPropertyChanged(); }
		}


		private string? _hadesLogs;

		public string? HadesLogs
		{
			get { return _hadesLogs; }
			set { _hadesLogs = value; OnPropertyChanged(); }
		}

		private string? _roomTemperature;

		public string? RoomTemperature
		{
			get { return _roomTemperature; }
			set { _roomTemperature = value; OnPropertyChanged(); }
		}

		private string? _roomHumidity;

		public string? RoomHumidity
        {
			get { return _roomHumidity; }
			set { _roomHumidity = value; OnPropertyChanged(); }
		}

		private bool _registerState;

		public bool RegisterState
		{
			get { return _registerState; }
			set { _registerState = value; OnPropertyChanged(); }
		}

		private string? _fullNameRegister;

		public string? FullNameRegister
		{
			get { return _fullNameRegister; }
			set { _fullNameRegister = value; OnPropertyChanged(); }
		}

		private string? _currentRFIDRegistered;

		public string? CurrentRFIDRegistered
		{
			get { return _currentRFIDRegistered; }
			set { _currentRFIDRegistered = value; OnPropertyChanged(); }
		}

        private bool _registeringFromPhone;

        public bool RegisteringFromPhone
        {
            get { return _registeringFromPhone; }
            set { _registeringFromPhone = value; OnPropertyChanged(); }
        }
    }
}
