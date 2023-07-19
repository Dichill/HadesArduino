﻿using HadesArduino.Core;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
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
        public ObservableCollection<String>? history;
    }

    public class UserService
    {
        private readonly string filePath = "users.json";

        // Use ObservableCollection instead of List
        private ObservableCollection<UserModel> users;

        public UserService()
        {
            // Initialize the ObservableCollection and load data from file
            users = ReadUsersFromFile();
        }

        public void Create(UserModel user)
        {
            users.Add(user); // Add the user to the ObservableCollection
            SaveUsersToFile(); // Save the updated data to the file
        }

        public ObservableCollection<UserModel> Read()
        {
            return users;
        }

        public void Update(UserModel updatedUser)
        {
            // Find and update the user in the ObservableCollection
            UserModel existingUser = users.FirstOrDefault(u => u.RFID == updatedUser.RFID);
            if (existingUser != null)
            {
                existingUser.Fullname = updatedUser.Fullname;
                existingUser.isActive = updatedUser.isActive;
                existingUser.dateCreated = updatedUser.dateCreated;
                existingUser.history = updatedUser.history;
                SaveUsersToFile(); // Save the updated data to the file
            }
        }

        public void Delete(string rfid)
        {
            // Find and remove the user from the ObservableCollection
            UserModel existingUser = users.FirstOrDefault(u => u.RFID == rfid);
            if (existingUser != null)
            {
                users.Remove(existingUser);
                SaveUsersToFile(); // Save the updated data to the file
            }
        }

        private ObservableCollection<UserModel> ReadUsersFromFile()
        {
            if (File.Exists(filePath))
            {
                string jsonData = File.ReadAllText(filePath);
                return JsonConvert.DeserializeObject<ObservableCollection<UserModel>>(jsonData);
            }
            else
            {
                return new ObservableCollection<UserModel>();
            }
        }

        private void SaveUsersToFile()
        {
            string jsonData = JsonConvert.SerializeObject(users, Formatting.Indented);
            File.WriteAllText(filePath, jsonData);
        }
    }
}
