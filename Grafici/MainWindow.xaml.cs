using System.Windows;

namespace Grafici
{
    public partial class MainWindow : Window
    {
        private MainWindowViewModel viewModel;

        public MainWindow()
        {
            Thread.CurrentThread.CurrentCulture = new System.Globalization.CultureInfo("en-US");

            InitializeComponent();

            viewModel = new MainWindowViewModel();
            DataContext = viewModel;
        }

        private void Indietro_Click(object sender, RoutedEventArgs e)
        {
            viewModel.Indietro();
            Plot.ResetAllAxes();
        }

        private void Avanti_Click(object sender, RoutedEventArgs e)
        {
            viewModel.Avanti();
            Plot.ResetAllAxes();
        }

        private void Aggiorna_Click(object sender, RoutedEventArgs e)
        {
            viewModel.Aggiorna();
        }

        private void Reset_Click(object sender, RoutedEventArgs e)
        {
            Plot.ResetAllAxes();
        }

        private void Ricarica_Click(object sender, RoutedEventArgs e)
        {
            viewModel.Ricarica();
        }
    }
}
