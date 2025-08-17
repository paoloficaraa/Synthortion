# Synthortion Plugin UI

Questa è l'interfaccia utente per il plugin di distorsione creativa Synthortion, progettata per essere integrata con JUCE.

## Struttura del Progetto

```
UI/
├── client/
│   ├── App.tsx                    # Componente principale dell'applicazione
│   ├── global.css                 # Stili globali con Tailwind CSS
│   ├── components/
│   │   ├── VSTInterface.tsx       # Interfaccia principale del plugin
│   │   └── ui/
│   │       ├── Knob.tsx          # Componente knob per i controlli
│   │       ├── LevelMeter.tsx    # Visualizzatore di livello audio
│   │       ├── VSTDisplay.tsx    # Display per visualizzazione FFT
│   │       └── EQSection.tsx     # Sezione equalizzatore
│   ├── hooks/
│   │   └── useAudioData.ts       # Hook per la gestione dei dati audio
│   └── lib/
│       └── utils.ts              # Utilities per classi CSS
├── public/                       # File statici
├── package.json                  # Dipendenze del progetto
├── vite.config.ts               # Configurazione Vite
├── tailwind.config.ts           # Configurazione Tailwind CSS
└── tsconfig.json                # Configurazione TypeScript
```

## Componenti Principali

### VSTInterface.tsx

L'interfaccia principale del plugin che include:

- **Knob COLOR**: Controllo principale per l'effetto di distorsione
- **Level Meters**: Visualizzatori di livello input/output
- **Display centrale**: Visualizzazione FFT e informazioni
- **Controlli aggiuntivi**: Delay, Chorus, Dry/Wet

### Componenti UI Essenziali

- **Knob**: Controllo rotativo per parametri audio
- **LevelMeter**: Visualizzatore di livello audio
- **VSTDisplay**: Display per analisi spettrale
- **EQSection**: Controlli equalizzatore

## Integrazione con JUCE

### 1. Connessione Dati Audio

Il file `useAudioData.ts` contiene il placeholder per la connessione con JUCE:

```typescript
// TODO: Replace with actual JUCE integration
// This is where you'll connect to JUCE's audio data
```

### 2. Parametri da Collegare

I seguenti stati React devono essere collegati ai parametri JUCE:

- `colorValue`: Parametro principale di distorsione
- `inputLevel` / `outputLevel`: Livelli audio
- `delayValue`: Parametro delay
- `chorusValue`: Parametro chorus
- `dryWetValue`: Mix dry/wet

### 3. Build per Produzione

```bash
npm run build
```

Questo genererà i file statici nella cartella `dist/` pronti per essere integrati in JUCE.

## Sviluppo

### Installazione Dipendenze

```bash
npm install
```

### Avvio Server di Sviluppo

```bash
npm run dev
```

### Preview Build

```bash
npm run preview
```

## Note per l'Integrazione JUCE

1. **WebView Component**: Usa il componente WebBrowserComponent di JUCE per caricare l'interfaccia
2. **Communication**: Implementa la comunicazione bidirezionale tra JUCE e React tramite JavaScript injection
3. **Real-time Updates**: Connetti i parametri audio in tempo reale usando i callback JUCE
4. **Responsive Design**: L'interfaccia è progettata per adattarsi a diverse dimensioni

## Dipendenze Minime

Il progetto usa solo le dipendenze essenziali:

- React 18.3.1
- TypeScript
- Vite per il build
- Tailwind CSS per lo styling
- Lucide React per le icone

Questo mantiene il bundle leggero e ottimizzato per l'integrazione con JUCE.
