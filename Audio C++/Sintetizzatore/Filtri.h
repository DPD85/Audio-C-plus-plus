#pragma once

#include "CostantiEdAltro.h"

#include <algorithm>

namespace Sintetizzatore
{
    /// @brief Interfaccia comune dei vari filtri.
    class Filtro
    {
      public:
        virtual ~Filtro() = default;

        /// @brief Applica il filtro al segnale in ingresso.
        /// @param campione Il campione attuale del segnale in ingresso.
        /// @return Il campione successivo del segnale filtrato.
        /// @remark Consultare la documentazione dei filtri per informazioni specifiche relative ai vari filtri.
        virtual double Computa(double campione) noexcept = 0;

        virtual void Reset() = 0;
    };

    namespace Filtri
    {
        /// @brief Smussa, con andamento esponenziale, un segnale.
        class SmussamentoEsponenziale: public Filtro
        {
          public:
            /// @brief Inizializza il filtro con fattore di smussamento e valore iniziale specificati.
            /// @param fattore_ Il fattore di smussamento: più è grande, maggiore è lo smussamento. [0, +∞]
            /// @param valoreIniziale Il valore smussato iniziale.
            explicit SmussamentoEsponenziale(const double fattore_, const double valoreIniziale = 0)
                : fattore(DaSmussamentoAGuadagno(fattore_))
            {
                Reset(valoreIniziale);
            }

            /// @brief Restituisce il valore smussato corrente.
            [[nodiscard]]
            double Valore() const
            {
                return valoreSmussato;
            }

            /// @brief Smussa il segnale in ingresso.
            /// @param valore Il campione attuale del segnale in ingresso.
            /// @return Il campione successivo del segnale smussato.
            double Computa(const double valore) noexcept override
            {
                const double t = valoreSmussato.load();
                valoreSmussato.store(t + (valore - valoreSmussato) * fattore);
                return t;
            }

            /// @brief Imposta il valore smussato iniziale a zero.
            /// @remark Il valore viene impostato in modo netto ed immediato per tanto cambierà istantaneamente.
            /// @audiosafe Computa().
            void Reset() override
            {
                valoreSmussato.store(0.0);
            }

            /// @brief Imposta il valore smussato iniziale.
            /// @param valore Il valore smussato iniziale.
            /// @remark Il valore viene impostato in modo netto ed immediato per tanto cambierà istantaneamente.
            /// @audiosafe Computa().
            void Reset(const double valore)
            {
                valoreSmussato.store(valore);
            }

          private:
            const double fattore;
            datomic valoreSmussato;
        };

        /// @brief Calcola l'inviluppo del valore assoluto di un segnale.
        ///
        /// Si tratta di uno smorzatore esponenziale che utilizza due fattori di smorzamento diversi per la fase di
        /// attacco e di rilascio nel calcolo dell'inviluppo.
        class RilevatoreInviluppo: public Filtro
        {
          public:
            /// @brief Inizializza il rilevatore d'inviluppo con i parametri specificati.
            /// @param attacco_ Il fattore di smussamento della fase di attacco: più è grande, maggiore è lo
            ///                 smussamento. [0, +∞]
            /// @param rilascio_ Il fattore di smussamento della fase di rilascio: più è grande, maggiore è lo
            ///                  smussamento. [0, +∞]
            RilevatoreInviluppo(const double attacco_, const double rilascio_)
                : attacco(DaSmussamentoAGuadagno(attacco_))
                , rilascio(DaSmussamentoAGuadagno(rilascio_))
            {}

            /// @brief Restituisce il campione corrente dell'inviluppo
            [[nodiscard]]
            double Inviluppo() const
            {
                return inviluppo;
            }

            /// @brief Computa il campione successivo dell'inviluppo del segnale.
            /// @param campione Il campione attuale del segnale.
            /// @return Il campione successivo dell'inviluppo.
            double Computa(const double campione) noexcept override
            {
                const double valore   = std::fabs(campione);
                const double guadagno = valore > inviluppo ? attacco : rilascio;

                inviluppo += (valore - inviluppo) * guadagno;

                return inviluppo;
            }

            /// @notaudiosafe Computa().
            void Reset() override
            {
                inviluppo = 0;
            }

          private:
            const double attacco;
            const double rilascio;
            double inviluppo{ 0 }; // Ampiezza
        };

        /// @brief Calcola l'inviluppo del valore assoluto di un segnale seguendo i picchi di quest'ultimo.
        class InseguitorePicchi: public RilevatoreInviluppo
        {
          public:
            /// @brief Inizializza l'inseguitore di picchi con i parametri specificati.
            /// @param rilascio_ Il fattore di smussamento della fase di rilascio: più è grande, maggiore è lo
            ///                  smussamento. [0, +∞]
            explicit InseguitorePicchi(const double rilascio_): RilevatoreInviluppo(0.0, rilascio_) {}
        };

        /// @brief Applica un ritardo ad un segnale
        class Ritardo: public Filtro
        {
          public:
            /// @brief Inizializza il filtro con il ritardo specificato.
            /// @param ritardo La dimensione del ritardo applicato al segnale, espresso in secondi. Il ritardo minimo è
            ///                pari ad un campione. (0, +∞]
            explicit Ritardo(const double ritardo): buffer(std::max<size_t>(1, DaSecondiACampioni(ritardo)), 0.0) {}

            /// @brief Ritarda il segnale in ingresso.
            /// @param campione Il campione attuale del segnale d'ingresso.
            /// @return Il campione successivo del segnale ritardato.
            double Computa(const double campione) noexcept override
            {
                const double valore = buffer.back();
                buffer.push_front(campione);
                return valore;
            }

            /// @notaudiosafe Computa().
            void Reset() override
            {
                std::ranges::fill(buffer, 0.0);
            }

          private:
            boost::circular_buffer<double> buffer;
        };

        namespace Interno
        {
            /// @brief Riduce l'aumento di volume di un segnale quando questo supera una certa soglia.
            /// @remark Implementazione interna utilizzata per implementare altri filtri, non è fatta per essere usata
            ///         direttamente.
            class Compressore: public Filtro
            {
              public:
                /// @brief Inizializza il filtro con i parametri specificati.
                /// @param rilascio Il fattore di smussamento della fase di rilascio: più è grande, maggiore è lo
                ///                 smussamento. [0, +∞]
                /// @param sogliaVolume La soglia, superata la quale, il compressore entra in funzione. [0, 1]
                /// @param CS Il coefficiente di riduzione del volume, se pari ad uno diventa un limitatore. [0, 1]
                /// @param ritardo Il ritardo, sul segnale d'ingresso, col quale il compressore agisce, espresso in
                ///                secondi. Il ritardo minimo è pari ad un campione. (0, +∞]
                Compressore(const double rilascio, const double sogliaVolume, const double CS, const double ritardo)
                    : sogliaVolumeLn(std::log(sogliaVolume))
                    , CS(CS)
                    , rilevatoreInviluppo(rilascio)
                    , ritardo(ritardo)
                {}

                /// @brief Riduce l'aumento di volume del segnale d'ingresso.
                /// @param campione Il campione corrente del segnale.
                /// @return Il campione attuale del segnale compresso.
                double Computa(const double campione) noexcept override
                {
                    const double inviluppo = rilevatoreInviluppo.Computa(campione);

                    const double valore = ritardo.Computa(campione);

                    const double inviluppoLn = std::log(inviluppo); // [Np]
                    const double guadagnoLn  = CS * std::min(0.0, sogliaVolumeLn - inviluppoLn);
                    const double guadagno    = std::exp(guadagnoLn);

                    return valore * guadagno;
                }

                /// @notaudiosafe Computa().
                void Reset() override
                {
                    rilevatoreInviluppo.Reset();
                    ritardo.Reset();
                }

              private:
                const double sogliaVolumeLn; // [Np] (Neper = identico al Bel ma calcolato con il logaritmo naturale)
                const double CS;
                InseguitorePicchi rilevatoreInviluppo;
                Ritardo ritardo;
            };
        }

        /// @brief Limita il volume di un segnale ad un valore massimo (look-ahead limiter).
        /// @remark Il limitatore non riesce a mantenere il volume strettamente sotto la soglia specificata, tal volta
        ///         la supera di un pò (se adeguatamente configurato la supererà di poco o anche molto poco). Si
        ///         consiglia quindi di configurare un soglia in un poco più bassa del limite che si desidera così da
        ///         assicurarsi che resti entro il limite desiderato, nonché di verificare sempre, facendo dei test, che
        ///         l'output del limitatore non superi mai il limite desiderato.
        class Limitatore: public Interno::Compressore
        {
          public:
            /// @brief Inizializza il limitatore con i parametri specificati.
            /// @param rilascio Il fattore di smussamento della fase di rilascio del rilevatore di inviluppo: più è
            ///                 grande, più lentamente segue il segnale quando il volume di quest'ultimo diminuisce.
            ///                 [0, +∞]
            /// @param sogliaVolume La soglia, superata la quale, il limitatore entra in funzione. [0, 1]
            /// @param ritardo Il ritardo, sul segnale d'ingresso, col quale il limitatore agisce, espresso in secondi.
            ///                Il ritardo minimo è pari ad un campione. (0, +∞]
            Limitatore(const double rilascio, const double sogliaVolume, const double ritardo)
                : Compressore(rilascio, sogliaVolume, 1.0, ritardo)
            {}
        };

        /// @brief Riduce l'aumento di volume del segnale quando questo supera una certa soglia (look-ahead compressor).
        class Compressore: public Interno::Compressore
        {
          public:
            /// @param rilascio Il fattore di smussamento della fase di rilascio: più è grande, maggiore è lo
            ///                 smussamento. [0, +∞]
            /// @param sogliaVolume La soglia, superata la quale, il compressore entra in funzione. [0, 1]
            /// @param proporzione La proporzione di riduzione del volume. (0, +∞]
            /// @param ritardo Il ritardo, sul segnale d'ingresso, col quale il compressore agisce, espresso in secondi.
            ///                Il ritardo minimo è pari ad un campione. (0, +∞]
            Compressore(
                const double rilascio, const double sogliaVolume, const double proporzione, const double ritardo)
                : Interno::Compressore(rilascio, sogliaVolume, 1.0 - 1.0 / proporzione, ritardo)
            {}
        };

        /// @brief Aggiunge l'èco ad un segnale con regolazione del volume massimo dell'èco.
        class EchoWetDry: public Filtro
        {
          public:
            /// @param ritardo Il ritardo dell'èco, espresso in secondi. (0, +∞]
            /// @param velocità La velocità con cui diminuisce il volume dell'èco, numero più grande significa minore
            ///                 velocità; un valore negativo inverte la fase del segnale. (-1, 1)
            /// @param volumeMassimo Il volume massimo dell'èco; un valore negativo inverte la fase del segnale. [-1, 1]
            EchoWetDry(const double ritardo, const double velocità, const double volumeMassimo)
                : buffer(DaSecondiACampioni(ritardo), 0.0)
                , velocità(velocità)
                , volumeMassimo(volumeMassimo)
            {}

            /// @brief Applica l'èco del segnale in ingresso.
            /// @param campione Il campione attuale del segnale d'ingresso.
            /// @return Il campione attuale del segnale con èco.
            double Computa(const double campione) noexcept override
            {
                const double valore = buffer.back();
                buffer.push_front(campione + velocità * valore);
                return (1 - volumeMassimo) * campione + volumeMassimo * valore;
            }

            /// @notaudiosafe Computa().
            void Reset() override
            {
                std::ranges::fill(buffer, 0.0);
            }

          private:
            boost::circular_buffer<double> buffer;
            const double velocità;
            const double volumeMassimo;
        };

        /// @brief Aggiunge l'èco ad un segnale.
        class Echo: public EchoWetDry
        {
          public:
            /// @param ritardo Il ritardo dell'èco, espresso in secondi. (0, +∞]
            /// @param velocità La velocità con cui diminuisce il volume dell'èco, numero più grande significa minore
            ///                 velocità; un valore negativo inverte la fase del segnale. (-1, 1)
            Echo(const double ritardo, const double velocità): EchoWetDry(ritardo, velocità, 1.0) {}
        };

        namespace Interno
        {
            /// @brief Implementa lo schema corrispondente alla topologia BiQuad.
            ///
            /// Implementa l'equazione:
            ///     \f[y(n) = a_0*x(n) + a_1*x(n-1) + a_2*x(n-1) - b_1*y(n-1) - b_2*y(n-1)\f]
            /// dove
            ///     * \f$x(n)\f$ è l'input;
            ///     * \f$y(n)\f$ è l'output;
            ///     * \f$a_0\f$, \f$a_1\f$, \f$a_2\f$, \f$b_1\f$ e \f$b_2\f$ sono dei coefficienti.
            ///
            /// @remark Implementazione interna utilizzata per implementare altri filtri, non è fatta per essere usata
            /// direttamente.
            class BiQuad: public Filtro
            {
              public:
                /// @brief Inizializza il filtro con tutti i guadagni a zero. In queste condizioni il filtro produrrà
                /// solamente silenzio.
                BiQuad(): a0(0.0), a1(0.0), a2(0.0), b1(0.0), b2(0.0) {}

                /// @brief Inizializza il filtro con i guadagni specificati.
                BiQuad(const double a0, const double a1, const double a2, const double b1, const double b2)
                {
                    Coefficienti(a0, a1, a2, b1, b2);
                }

                /// @brief Applica il filtro al segnale in ingresso.
                /// @param campione Il campione attuale del segnale d'ingresso.
                /// @return Il campione successivo dell'output del filtro.
                double Computa(const double campione) noexcept override
                {
                    const double output = a0 * campione                                 //
                                        + a1 * inputPrecedente1 + a2 * inputPrecedente2 //
                                        - b1 * outputPrecedente1 - b2 * outputPrecedente2;

                    inputPrecedente2 = inputPrecedente1;
                    inputPrecedente1 = campione;

                    outputPrecedente2 = outputPrecedente1;
                    outputPrecedente1 = output;

                    return output;
                }

                /// @notaudiosafe Computa().
                void Reset() override
                {
                    inputPrecedente1 = 0.0;
                    inputPrecedente2 = 0.0;

                    outputPrecedente1 = 0.0;
                    outputPrecedente2 = 0.0;
                }

              protected:
                /// @brief Imposta nuovi valori per i coefficienti del filtro.
                /// @notaudiosafe Computa().
                void Coefficienti(
                    const double a0_, const double a1_, const double a2_, const double b1_, const double b2_)
                {
                    a0 = a0_;
                    a1 = a1_;
                    a2 = a2_;
                    b1 = b1_;
                    b2 = b2_;
                }

              private:
                double inputPrecedente1{ 0 };  // [Ampiezza]
                double inputPrecedente2{ 0 };  // [Ampiezza]
                double outputPrecedente1{ 0 }; // [Ampiezza]
                double outputPrecedente2{ 0 }; // [Ampiezza]
                double a0, a1, a2;
                double b1, b2;
            };
        }

        /// @brief %Filtro passa basso del primo ordine.
        /// @warning
        ///     Se la frequenza di taglio è uguale alla frequenza di Nyquist allora il filtro lascerà passare tutte
        ///     le frequenze dal segnale, di fatto l'output sarà identico al segnale d'input.
        class FiltroPassaBasso: public Interno::BiQuad
        {
          public:
            /// @brief Inizializza il filtro con frequenza di taglio pari a zero. In queste condizioni il filtro
            /// produrrà solamente silenzio.
            FiltroPassaBasso(): frequenzaTaglio(0.0) {}

            /// @brief Inizializza il filtro con frequenza di taglio specificata.
            /// @param frequenzaTaglio La frequenza di taglio del filtro espressa in Hz. [0, Nyquist]
            explicit FiltroPassaBasso(const double frequenzaTaglio)
            {
                FrequenzaTaglio(frequenzaTaglio);
            }

            /// @brief Restituisce la frequenza di taglio del filtro. [Hz]
            [[nodiscard]]
            double FrequenzaTaglio() const
            {
                return frequenzaTaglio;
            }

            /// @brief Imposta la frequenza di taglio del filtro.
            /// @param frequenzaTaglio_ La frequenza di taglio del filtro espressa in Hz. [0, Nyquist]
            /// @notaudiosafe Computa().
            void FrequenzaTaglio(const double frequenzaTaglio_)
            {
                // Limito la frequenza di taglio alla frequenza di Nyquist
                frequenzaTaglio = std::min(frequenzaTaglio_, Costanti::FrequenzaCampionamento / 2.0);

                const double theta  = 2.0 * std::numbers::pi * frequenzaTaglio / Costanti::FrequenzaCampionamento;
                const double gamma  = cos(theta) / (1.0 + sin(theta));
                const double a0     = (1.0 - gamma) / 2.0;
                const double a1     = (1.0 - gamma) / 2.0;
                constexpr double a2 = 0.0;
                const double b1     = -gamma;
                constexpr double b2 = 0.0;

                Coefficienti(a0, a1, a2, b1, b2);
            }

          private:
            double frequenzaTaglio; // [Hz]
        };

        /// @brief %Filtro passa alto del primo ordine.
        /// @warning
        ///     Se la frequenza di taglio è uguale alla frequenza di Nyquist allora il filtro eliminerà tutte le
        ///     frequenze dal segnale, di fatto l'output sarà silenzio.
        class FiltroPassaAlto: public Interno::BiQuad
        {
          public:
            /// @brief Inizializza il filtro con la frequenza di taglio specificata.
            /// @param frequenzaTaglio La frequenza di taglio espressa in Hz. [0, Nyquist]
            explicit FiltroPassaAlto(const double frequenzaTaglio)
            {
                FrequenzaTaglio(frequenzaTaglio);
            }

            /// @brief Restituisce la frequenza di taglio del filtro. [Hz]
            [[nodiscard]]
            double FrequenzaTaglio() const
            {
                return frequenzaTaglio;
            }

            /// @brief Imposta la frequenza di taglio del filtro.
            /// @param frequenzaTaglio_ La frequenza di taglio del filtro espressa in Hz. [0, Nyquist]
            /// @notaudiosafe Computa().
            void FrequenzaTaglio(const double frequenzaTaglio_)
            {
                // Limito la frequenza di taglio alla frequenza di Nyquist
                frequenzaTaglio = std::min(frequenzaTaglio_, Costanti::FrequenzaCampionamento / 2.0);

                const double theta  = 2.0 * std::numbers::pi * frequenzaTaglio / Costanti::FrequenzaCampionamento;
                const double gamma  = cos(theta) / (1.0 + sin(theta));
                const double a0     = (1.0 + gamma) / 2.0;
                const double a1     = -(1.0 + gamma) / 2.0;
                constexpr double a2 = 0.0;
                const double b1     = -gamma;
                constexpr double b2 = 0.0;

                Coefficienti(a0, a1, a2, b1, b2);
            }

          private:
            double frequenzaTaglio; // [Hz]
        };

        /// @brief %Filtro passa tutto ritardante (Delaying All-Pass filter).
        ///
        /// Il filtro crea una serie di èco lasciando inalterata l'ampiezza delle frequenze ma modificandone la fase.
        /// La fase delle varie frequenze varia tra 0° per la frequenza zero e 180° per la frequenza di Nyquist, in
        /// corrispondenza della frequenza di rottura la fase è sempre -90°.
        ///
        /// Il guadagno necessario per ottenere un filtro con specifica frequenza di rottura si calcola come segue:
        ///     \f[guadagno = \frac{tan(\pi * f_r / f_c) - 1}{tan(\pi * f_r / f_c) + 1}\f]
        /// dove
        ///     * \f$f_c\f$ è la frequenza di campionamento;
        ///     * \f$f_r\f$ è la frequenza di rottura.
        class FiltroPassaTutto: public Filtro
        {
          public:
            /// @brief Inizializzo il filtro con guadagno e ritardo zero. In queste condizioni il filtro produrrà
            /// solamente silenzio.
            FiltroPassaTutto(): guadagno(0.0) {}

            /// @brief Inizializzo il filtro con guadagno zero e ritardo specificato. In queste condizioni il filtro
            /// produrrà solamente silenzio.
            /// @param ritardo La durata del ritardo. [s]
            explicit FiltroPassaTutto(const double ritardo): buffer(DaSecondiACampioni(ritardo), 0.0), guadagno(0.0) {}

            /// @brief Inizializzo il filtro con guadagno e ritardo specificati.
            /// @param ritardo La durata del ritardo. [s]
            /// @param guadagno Controlla la frequenza di rottura, quella alla quale la fase è modificata di -90°.
            ///                 [0, 1]
            FiltroPassaTutto(const double ritardo, const double guadagno)
                : buffer(DaSecondiACampioni(ritardo), 0.0)
                , guadagno(guadagno)
            {}

            /// @brief Applica il filtro al segnale in ingresso.
            /// @param campione Il campione attuale del segnale d'ingresso.
            /// @return Il campione successivo dell'output del filtro.
            double Computa(const double campione) noexcept override
            {
                const double valore = buffer.back();

                buffer.push_front(valore * guadagno + campione);

                return campione * -guadagno + valore;
            }

            /// @notaudiosafe Computa().
            void Reset() override
            {
                std::ranges::fill(buffer, 0.0);
            }

            /// @brief Restituisce il guadagno attualmente usato dal filtro. [0, 1]
            /// @notaudiosafe Computa().
            [[nodiscard]]
            double Guadagno() const
            {
                return guadagno;
            }

            /// @brief Imposta il guadagno del filtro.
            /// @param guadagno_ Il nuovo guadagno. [0, 1]
            /// @notaudiosafe Computa().
            void Guadagno(const double guadagno_)
            {
                guadagno = guadagno_;
            }

          private:
            boost::circular_buffer<double> buffer;
            double guadagno; // [0, 1]
        };

        /// @brief %Filtro pettine con filtro passa basso (Low-Pass Comb Filter).
        ///
        /// Il filtro somma al segnale in ingresso se stesso ritardo di un certo tempo, il filtro va così a creare una
        /// serie di èco.<br /> La risposta all'impulso è un serie di picchi equidistanti progressivamente attenuati.
        /// <br /> Le frequenze più alte del segnale in ingresso vengono attenuate molto più delle basse riducendo così
        /// di molto il loro contributo.
        class FiltroPettine: public Filtro
        {
          public:
            /// @brief Inizializza il filtro senza ritardo, la massima attenuazione e frequenza di taglio di 0 Hz.
            /// In queste condizioni il filtro produrrà solamente silenzio.
            FiltroPettine(): attenuazione(0.0) {}

            /// @brief Inizializza il filtro con il ritardo specificato, la massima attenuazione e frequenza di taglio
            /// di 0 Hz. In queste condizioni il filtro produrrà solamente silenzio.
            /// @param ritardo Il ritardo del segnale sommato all'input rispetto all'input. [s]
            explicit FiltroPettine(const double ritardo): buffer(DaSecondiACampioni(ritardo), 0.0), attenuazione(0.0) {}

            /// @brief Inizializza il filtro con il ritardo, la massima attenuazione e frequenza di taglio specificati.
            /// @param ritardo Il ritardo del segnale sommato all'input rispetto all'input. [s]
            /// @param attenuazione L'attenuazione dei picchi introdotti dal filtro, 0 = nessun picco, 1 = senza
            ///                     attenuazione. [0, 1]
            /// @param frequenzaTaglio La frequenza di taglio del filtro passa basso. [Hz]
            FiltroPettine(const double ritardo, const double attenuazione, const double frequenzaTaglio)
                : buffer(DaSecondiACampioni(ritardo), 0.0)
                , attenuazione(attenuazione)
                , passaBasso(frequenzaTaglio)
            {}

            /// @brief Applica il filtro al segnale in ingresso.
            /// @param campione Il campione attuale del segnale d'ingresso.
            /// @return Il campione successivo dell'output del filtro.
            double Computa(const double campione) noexcept override
            {
                double valore = buffer.back();

                valore = passaBasso.Computa(valore);

                buffer.push_front(campione + valore * attenuazione);

                return valore;
            }

            /// @notaudiosafe Computa().
            void Reset() override
            {
                std::ranges::fill(buffer, 0.0);
            }

            /// @brief Restituisce il ritardo attuale del segnale sommato all'input rispetto all'input. [s]
            /// @notaudiosafe Computa().
            [[nodiscard]]
            double Ritardo() const
            {
                return DaCampioniASecondi(buffer.size());
            }

            /// @brief Imposta il ritardo del segnale sommato all'input rispetto all'input.
            /// @param ritardo Il ritardo. [s]
            /// @notaudiosafe Computa().
            void Ritardo(const double ritardo)
            {
                buffer.resize(DaSecondiACampioni(ritardo), 0.0);
            }

            /// @brief Restituisce l'attenuazione dei picchi creati dal filtro. [0, 1]
            /// @notaudiosafe Computa().
            [[nodiscard]]
            double Attenuazione() const
            {
                return attenuazione;
            }

            /// @brief Imposta l'attenuazione dei picchi creati dal filtro.
            /// @param attenuazione_ L'attenuazione dei picchi, 0 = nessun picco, 1 = nessuna attenuazione. [0, 1]
            /// @notaudiosafe Computa().
            void Attenuazione(const double attenuazione_)
            {
                attenuazione = attenuazione_;
            }

            /// @brief Restituisce la frequenza di taglio del filtro passa basso. [Hz]
            [[nodiscard]]
            double FrequenzaTaglio() const
            {
                return passaBasso.FrequenzaTaglio();
            }

            /// @brief Imposta la frequenza di taglio del filtro passa basso.
            /// @param frequenzaTaglio La frequenza di taglio. [Hz]
            /// @notaudiosafe Computa().
            void FrequenzaTaglio(const double frequenzaTaglio)
            {
                passaBasso.FrequenzaTaglio(frequenzaTaglio);
            }

          private:
            boost::circular_buffer<double> buffer;
            double attenuazione; // [0, 1]
            FiltroPassaBasso passaBasso;
        };

        /// @brief %Riverbero di Schroeder con l'aggiunta di filtri passa passo (Low-Pass Filter–Comb Reverberator)
        /// composto da quattro filtri pettine con filtro passa basso e due filtri passa tutto.
        ///
        /// I quattro filtri pettine utilizzati hanno le seguenti caratteristiche:
        ///     - il 1° a ritardo di 1116 campioni corrispondenti a 23.25 ms. con campionamento a 48 KHz;
        ///     - il 2° a ritardo di 1188 campioni corrispondenti a 24.75 ms. con campionamento a 48 KHz;
        ///     - il 3° a ritardo di 1277 campioni corrispondenti a ~26.6 ms. con campionamento a 48 KHz;
        ///     - il 4° a ritardo di 1356 campioni corrispondenti a 28.25 ms. con campionamento a 48 KHz;
        ///     - tutti hanno la stessa attenuazione e la stessa frequenza di taglio per il filtro passa basso.
        ///
        /// I due filtri passa tutto hanno le seguenti caratteristiche:
        ///     - il 1° a ritardo di 225 campioni corrispondenti a 4.6875 ms. con campionamento a 48 KHz;
        ///     - il 2° a ritardo di 556 campioni corrispondenti a ~11.586 ms. con campionamento a 48 KHz;
        ///     - entrambi hanno un guadagno di 0.5 corrispondente ad una frequenza di rottura di ~19084 Hz.
        class Riverbero: public Filtro
        {
          public:
            /// @brief Inizializza il filtro con dimensione della stanza, riverbero e frequenza di taglio specificati.
            /// @param dimensioneStanza La dimensione della stanza simulata, 0 = stanza minuscola, 1 = stanza enorme.
            ///                         [0, 1]
            /// @param riverbero La quantità di riverbero presente nell'output, 0 = nessun riverbero, 1 = solo
            ///                  riverbero. [0, 1]
            /// @param frequenzaTaglio La frequenza di taglio del filtro passa basso [Hz]; il riverbero prodotto dalle
            ///                        frequenze attenuate dal filtro svanirà molto più velocemente rispetto a quello
            ///                        prodotto dalle altre frequenze.
            Riverbero(const double dimensioneStanza, const double riverbero, const double frequenzaTaglio)
                : riverbero(riverbero)
            {
                for (FiltroPettine &filtro : filtriPettine)
                {
                    filtro.Attenuazione(dimensioneStanza);
                    filtro.FrequenzaTaglio(frequenzaTaglio);
                }
            }

            /// @brief Applica il riverbero al segnale in ingresso.
            /// @param campione Il campione attuale del segnale d'ingresso.
            /// @return Il campione successivo del segnale con riverbero.
            double Computa(const double campione) noexcept override
            {
                double output = 0.0;

                for (FiltroPettine &filtro : filtriPettine)
                    output += filtro.Computa(campione);

                for (FiltroPassaTutto &filtro : filtriPassaTutto)
                    output = filtro.Computa(output);

                return campione * (1.0 - riverbero) + output * riverbero;
            }

            /// @notaudiosafe Computa().
            void Reset() override
            {
                for (FiltroPettine &filtro : filtriPettine)
                    filtro.Reset();
                for (FiltroPassaTutto &filtro : filtriPassaTutto)
                    filtro.Reset();
            }

          private:
            // Nota: le durate dei ritardi (espresse in numero di campioni) non devono essere correlate tra loro, ovvero
            // non deve esserci nessuna correlazione matematica semplice tra loro. Per esempio:
            //  - non devono essere multipli o sottomultipli tra loro;
            //  - non devono avere fattori comuni;
            //  - ecc...
            // La proporzione tra il ritardo più piccolo ed il più grande è consigliabile che sia di circa 1:1.5
            std::array<FiltroPettine, 4> filtriPettine = {
                FiltroPettine(DaCampioniASecondi(1'116)), // 23.25 ms.
                FiltroPettine(DaCampioniASecondi(1'188)), // 24.75 ms.
                FiltroPettine(DaCampioniASecondi(1'277)), // ~26.6 ms.
                FiltroPettine(DaCampioniASecondi(1'356))  // 28.25 ms.
            };
            // Nota: durata del ritardo molto più corta di quella dei FiltriPettine, attenuazione identica per tutti e
            // compresa tra 0.5 e 0.707
            std::array<FiltroPassaTutto, 2> filtriPassaTutto = {
                FiltroPassaTutto(DaCampioniASecondi(225), 0.5), // 4.6875 ms.; 0.5 = frequenza di rottura ~19084 Hz
                FiltroPassaTutto(DaCampioniASecondi(556), 0.5)  // ~11.584 ms.
            };

            double riverbero; // [0, 1]
        };
    }
}
