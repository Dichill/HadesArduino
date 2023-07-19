using HadesArduino.MVVM.ViewModel;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace HadesArduino.UserControl
{
    /// <summary>
    /// Interaction logic for TextDialog.xaml
    /// </summary>
    public partial class TextDialog
    {
        public GlobalViewModel GlobalViewModel { get; } = GlobalViewModel.Instance;
        public TextDialog()
        {
            InitializeComponent();

            // Set the DataContext of the Border to the GlobalViewModel
            MyBorder.DataContext = GlobalViewModel;
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            GlobalViewModel.FullNameRegister = NameTxtBox.Text;
        }

        private void Storyboard_Completed(object sender, EventArgs e)
        {
            // Play the fade out animation for the old Grid (MyGrid)
            FirstGrid.BeginStoryboard((Storyboard)FindResource("WidthAnimationStoryboard"));
            FirstGrid.Visibility = Visibility.Hidden;
        }

        private void Storyboard_Completed_1(object sender, EventArgs e)
        {
            SecondGrid.Visibility = Visibility.Visible;
            SecondGrid.BeginStoryboard((Storyboard)FindResource("FadeInStoryboard"));
        }
    }
}
