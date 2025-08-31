using OxyPlot;
using OxyPlot.Axes;
using OxyPlot.Legends;
using OxyPlot.Series;
using System.Diagnostics;
using System.IO;

namespace Grafici
{
    public class MainWindowViewModel : INotifyBase
    {
        private const double FREQUENZA_CAMPIONAMENTO = 48000.0;
        private const double INV_FREQUENZA_CAMPIONAMENTO = 1.0 / FREQUENZA_CAMPIONAMENTO;
        private const int DIMENSIONE_CAMPIONE = sizeof(double);

        public string[] FileRegistrazioni { get; }

        private string fileSelezionato = "rec.dat";
        public string FileSelezionato
        {
            get => fileSelezionato;
            set
            {
                if (SetProperty(ref fileSelezionato, value))
                {
                    TempoInizio = 0;
                    Intervallo = 1;

                    CaricaRegistrazione(fileSelezionato);

                    TempoFine = Math.Min(tempoInizio + intervallo, durataTotale);

                    CaricaIntervallo();
                }
            }
        }

        private DataPoint[][] campioni;
        private LineSeries[] serie;

        private PlotModel plotModel = new();
        public PlotModel PlotModel { get => plotModel; set => SetProperty(ref plotModel, value); }

        private DataPoint massimo;
        public DataPoint Massimo { get => massimo; set => SetProperty(ref massimo, value); }

        private DataPoint minimo;
        public DataPoint Minimo { get => minimo; set => SetProperty(ref minimo, value); }

        private double durataTotale = 0; // [sec]
        public double DurataTotale
        {
            get => durataTotale;
            set
            {
                if (SetProperty(ref durataTotale, value))
                {
                    TempoFine = Math.Min(tempoInizio + intervallo, value);
                    OnPropertyChanged(nameof(DurataTotaleStr));
                }
            }
        }
        public string DurataTotaleStr => $"{string.Format("{0:0.###}", durataTotale)} sec.";

        private double intervallo = 1;
        public double Intervallo
        {
            get => intervallo;
            set
            {
                if (SetProperty(ref intervallo, value))
                    OnPropertyChanged(nameof(IntervalloStr));
            }
        }
        public string IntervalloStr => $"Intervallo {string.Format("{0:0.###}", intervallo)} sec.";

        public string IntervalloVisualizzatoStr => $"[{string.Format("{0:0.###}", tempoInizio)}; {string.Format("{0:0.###}", tempoFine)}] sec.";

        private double tempoInizio = 0;
        private double TempoInizio
        {
            get => tempoInizio;
            set
            {
                if (tempoInizio != value)
                {
                    tempoInizio = Math.Max(value, 0);
                    TempoFine = tempoInizio + intervallo;
                    OnPropertyChanged(nameof(IntervalloVisualizzatoStr));
                }
            }
        }

        private double tempoFine = 1;
        private double TempoFine
        {
            get => tempoFine;
            set
            {
                if (tempoFine != value)
                {
                    tempoFine = Math.Min(value, durataTotale);
                    OnPropertyChanged(nameof(IntervalloVisualizzatoStr));
                }
            }
        }

        public MainWindowViewModel()
        {
            FileRegistrazioni = Directory.GetFiles(".", "*.dat");
            for (int i = 0; i < FileRegistrazioni.Length; ++i)
                FileRegistrazioni[i] = Path.GetFileName(FileRegistrazioni[i]);

            fileSelezionato = FileRegistrazioni[0];

            PlotModel.Axes.Add(new LinearAxis()
            {
                Position = AxisPosition.Left,
                MajorGridlineStyle = LineStyle.Solid,
                MajorGridlineColor = OxyColors.DarkGray,
                MinorGridlineStyle = LineStyle.Solid
            });
            PlotModel.Axes.Add(new LinearAxis()
            {
                Position = AxisPosition.Bottom,
                MajorGridlineStyle = LineStyle.Solid,
                MajorGridlineColor = OxyColors.DarkGray,
                MinorGridlineStyle = LineStyle.Solid
            });

            var leggenda = new Legend
            {
                LegendPosition = LegendPosition.LeftMiddle,
                LegendPlacement = LegendPlacement.Outside
            };
            PlotModel.Legends.Add(leggenda);

            CaricaRegistrazione(fileSelezionato);
            CaricaIntervallo();
        }

        public void Indietro()
        {
            if (tempoInizio == 0) return;

            TempoInizio -= intervallo;

            CaricaIntervallo();
        }

        public void Avanti()
        {
            if (durataTotale - tempoInizio <= intervallo) return;

            TempoInizio += intervallo;

            CaricaIntervallo();
        }

        public void Aggiorna()
        {
            TempoFine = tempoInizio + intervallo;
            CaricaIntervallo();
        }

        public void Ricarica()
        {
            CaricaRegistrazione(fileSelezionato);
            CaricaIntervallo();
        }

        private void CaricaIntervallo()
        {
            try
            {
                for (int s = 0; s < campioni.Length; ++s)
                {
                    serie[s].Points.Clear();

                    int inizio = (int)Math.Ceiling(tempoInizio * FREQUENZA_CAMPIONAMENTO);
                    int fine = (int)Math.Ceiling(tempoFine * FREQUENZA_CAMPIONAMENTO);

                    var mas = new DataPoint(0, double.NegativeInfinity);
                    var min = new DataPoint(0, double.PositiveInfinity);

                    for (int i = inizio; i < fine; ++i)
                    {
                        serie[s].Points.Add(campioni[s][i]);

                        if (campioni[s][i].Y > mas.Y) mas = campioni[s][i];
                        if (campioni[s][i].Y < min.Y) min = campioni[s][i];
                    }

                    Massimo = mas;
                    Minimo = min;
                }

                PlotModel.InvalidatePlot(true);
            }
            catch (Exception ex)
            {
                if (!Debugger.IsAttached) Debugger.Launch();
                Console.WriteLine(ex.ToString());
                Debugger.Break();
            }
        }

        private void CaricaRegistrazione(string percorsoFile)
        {
            byte[] dati = File.ReadAllBytes(percorsoFile);

            int numeroSerie = dati[0];
            int numCampioni = (dati.Length - 1) / DIMENSIONE_CAMPIONE / numeroSerie;

            campioni = new DataPoint[numeroSerie][];

            int byteOffset = 1;
            for (int s = 0; s < numeroSerie; ++s)
            {
                campioni[s] = new DataPoint[numCampioni];

                double tempo = 0;
                for (int i = 0;
                    i < numCampioni;
                    ++i, byteOffset += DIMENSIONE_CAMPIONE, tempo += INV_FREQUENZA_CAMPIONAMENTO)
                {
                    campioni[s][i] = new DataPoint(tempo, BitConverter.ToDouble(dati, byteOffset));
                }
            }

            DurataTotale = campioni[0].Length / FREQUENZA_CAMPIONAMENTO;

            // ----- -----

            PlotModel.Series.Clear();

            serie = new LineSeries[campioni.Length];
            for (int s = 0; s < campioni.Length; ++s)
            {
                serie[s] = new LineSeries() { Title = $"Serie {s}" };
                PlotModel.Series.Add(serie[s]);
            }
        }
    }
}
