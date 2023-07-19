using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HadesArduino.MVVM.Model
{
    public class UserModel
    {
        public string? Fullname { get; set; }
        public string? RFID { get; set; }
        public bool? isActive { get; set; }
        public DateTime dateCreated { get; set; }

    }
}
